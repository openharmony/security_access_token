/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "permission_policy_set.h"

#include <algorithm>

#include "accesstoken_id_manager.h"
#include "accesstoken_common_log.h"
#include "access_token_db.h"
#include "access_token_error.h"
#include "constant_common.h"
#include "permission_definition_cache.h"
#include "permission_map.h"
#include "permission_validator.h"
#include "perm_setproc.h"
#include "data_translator.h"
#include "token_field_const.h"

namespace OHOS {
namespace Security {
namespace AccessToken {

PermissionPolicySet::~PermissionPolicySet()
{
    LOGD(ATM_DOMAIN, ATM_TAG,
        "%{public}s called, tokenID: 0x%{public}x destruction", __func__, tokenId_);
}


void PermissionPolicySet::GetPermissionBriefData(std::vector<BriefPermData>& list,
    const std::vector<PermissionStatus> &permStateList)
{
    for (const auto& state : permStateList) {
        BriefPermData data = {0};
        uint32_t code;
        if (TransferPermissionToOpcode(state.permissionName, code)) {
            data.status = (state.grantStatus == PERMISSION_GRANTED) ? 1 : 0;
            data.permCode = code;
            data.flag = state.grantFlag;
            list.emplace_back(data);
        }
    }
}

std::shared_ptr<PermissionPolicySet> PermissionPolicySet::BuildPermissionPolicySet(
    AccessTokenID tokenId, const std::vector<PermissionStatus>& permStateList)
{
    ATokenTypeEnum tokenType = AccessTokenIDManager::GetInstance().GetTokenIdTypeEnum(tokenId);
    std::shared_ptr<PermissionPolicySet> policySet = std::make_shared<PermissionPolicySet>();
    PermissionValidator::FilterInvalidPermissionState(tokenType, true, permStateList, policySet->permStateList_);
    policySet->tokenId_ = tokenId;

    if (tokenType == TOKEN_HAP) {
        std::vector<BriefPermData> list;
        GetPermissionBriefData(list, policySet->permStateList_);
        PermissionDataBrief::GetInstance().AddBriefPermDataByTokenId(tokenId, list);
    }

    return policySet;
}

std::shared_ptr<PermissionPolicySet> PermissionPolicySet::BuildPolicySetWithoutDefCheck(
    AccessTokenID tokenId, const std::vector<PermissionStatus>& permStateList)
{
    std::shared_ptr<PermissionPolicySet> policySet = std::make_shared<PermissionPolicySet>();
    PermissionValidator::FilterInvalidPermissionState(
        TOKEN_TYPE_BUTT, false, permStateList, policySet->permStateList_);
    policySet->tokenId_ = tokenId;
    LOGI(ATM_DOMAIN, ATM_TAG, "TokenID: %{public}d, permStateList_ size: %{public}zu",
        tokenId, policySet->permStateList_.size());
    std::vector<BriefPermData> list;
    GetPermissionBriefData(list, policySet->permStateList_);
    PermissionDataBrief::GetInstance().AddBriefPermDataByTokenId(tokenId, list);
    return policySet;
}

std::shared_ptr<PermissionPolicySet> PermissionPolicySet::BuildPermissionPolicySetFromDb(
    AccessTokenID tokenId, const std::vector<GenericValues>& permStateRes)
{
    std::shared_ptr<PermissionPolicySet> policySet = std::make_shared<PermissionPolicySet>();
    policySet->tokenId_ = tokenId;

    for (const GenericValues& stateValue : permStateRes) {
        if ((AccessTokenID)stateValue.GetInt(TokenFiledConst::FIELD_TOKEN_ID) == tokenId) {
            PermissionStatus state;
            int ret = DataTranslator::TranslationIntoPermissionStatus(stateValue, state);
            if (ret == RET_SUCCESS) {
                MergePermissionStatus(policySet->permStateList_, state);
            } else {
                LOGE(ATM_DOMAIN, ATM_TAG, "TokenId 0%{public}u permState is wrong.", tokenId);
            }
        }
    }
    return policySet;
}

void PermissionPolicySet::UpdatePermStatus(const PermissionStatus& permOld, PermissionStatus& permNew)
{
    // if user_grant permission is not operated by user, it keeps the new initalized state.
    // the new state can be pre_authorization.
    if ((permOld.grantFlag == PERMISSION_DEFAULT_FLAG) && (permOld.grantStatus == PERMISSION_DENIED)) {
        return;
    }
    // if old user_grant permission is granted by pre_authorization fixed, it keeps the new initalized state.
    // the new state can be pre_authorization or not.
    if ((permOld.grantFlag == PERMISSION_SYSTEM_FIXED) ||
        // if old user_grant permission is granted by pre_authorization unfixed
        // and the user has not operated this permission, it keeps the new initalized state.
        (permOld.grantFlag == PERMISSION_GRANTED_BY_POLICY)) {
        return;
    }

    permNew.grantStatus = permOld.grantStatus;
    permNew.grantFlag = permOld.grantFlag;
}

void PermissionPolicySet::Update(const std::vector<PermissionStatus>& permStateList)
{
    std::vector<PermissionStatus> permStateFilterList;
    PermissionValidator::FilterInvalidPermissionState(TOKEN_HAP, true, permStateList, permStateFilterList);
    LOGI(ATM_DOMAIN, ATM_TAG, "PermStateFilterList size: %{public}zu.", permStateFilterList.size());
    Utils::UniqueWriteGuard<Utils::RWLock> infoGuard(this->permPolicySetLock_);

    for (PermissionStatus& permStateNew : permStateFilterList) {
        auto iter = std::find_if(permStateList_.begin(), permStateList_.end(),
            [permStateNew](const PermissionStatus& permStateOld) {
                return permStateNew.permissionName == permStateOld.permissionName;
            });
        if (iter != permStateList_.end()) {
            UpdatePermStatus(*iter, permStateNew);
        }
    }
    permStateList_ = permStateFilterList;
    std::vector<BriefPermData> list;
    GetPermissionBriefData(list, permStateList_);
    PermissionDataBrief::GetInstance().AddBriefPermDataByTokenId(tokenId_, list);
}

uint32_t PermissionPolicySet::GetFlagWroteToDb(uint32_t grantFlag)
{
    return ConstantCommon::GetFlagWithoutSpecifiedElement(grantFlag, PERMISSION_COMPONENT_SET);
}

std::shared_ptr<PermissionPolicySet> PermissionPolicySet::RestorePermissionPolicy(AccessTokenID tokenId,
    const std::vector<GenericValues>& permStateRes)
{
    std::shared_ptr<PermissionPolicySet> policySet = std::make_shared<PermissionPolicySet>();
    policySet->tokenId_ = tokenId;

    for (const GenericValues& stateValue : permStateRes) {
        if ((AccessTokenID)stateValue.GetInt(TokenFiledConst::FIELD_TOKEN_ID) == tokenId) {
            PermissionStatus state;
            int ret = DataTranslator::TranslationIntoPermissionStatus(stateValue, state);
            if (ret == RET_SUCCESS) {
                MergePermissionStatus(policySet->permStateList_, state);
            } else {
                LOGE(ATM_DOMAIN, ATM_TAG, "TokenId 0x%{public}x permState is wrong.", tokenId);
            }
        }
    }

    ATokenTypeEnum tokenType = AccessTokenIDManager::GetInstance().GetTokenIdTypeEnum(tokenId);
    if (tokenType == TOKEN_HAP) {
        std::vector<BriefPermData> list;
        GetPermissionBriefData(list, policySet->permStateList_);
        PermissionDataBrief::GetInstance().AddBriefPermDataByTokenId(tokenId, list);
    }

    return policySet;
}

void PermissionPolicySet::MergePermissionStatus(std::vector<PermissionStatus>& permStateList,
    PermissionStatus& state)
{
    uint32_t flag = GetFlagWroteToDb(state.grantFlag);
    state.grantFlag = flag;
    for (auto iter = permStateList.begin(); iter != permStateList.end(); iter++) {
        if (state.permissionName == iter->permissionName) {
            iter->grantStatus = state.grantStatus;
            iter->grantFlag = state.grantFlag;
            LOGD(ATM_DOMAIN, ATM_TAG, "Update permission: %{public}s.", state.permissionName.c_str());
            return;
        }
    }
    LOGD(ATM_DOMAIN, ATM_TAG, "Add permission: %{public}s.", state.permissionName.c_str());
    permStateList.emplace_back(state);
}

void PermissionPolicySet::StorePermissionState(std::vector<GenericValues>& valueList) const
{
    for (const auto& permissionState : permStateList_) {
        LOGD(ATM_DOMAIN, ATM_TAG, "PermissionName: %{public}s", permissionState.permissionName.c_str());
        GenericValues genericValues;
        genericValues.Put(TokenFiledConst::FIELD_TOKEN_ID, static_cast<int32_t>(tokenId_));
        DataTranslator::TranslationIntoGenericValues(permissionState, genericValues);
        valueList.emplace_back(genericValues);
    }
}

void PermissionPolicySet::StorePermissionPolicySet(std::vector<GenericValues>& permStateValueList)
{
    Utils::UniqueReadGuard<Utils::RWLock> infoGuard(this->permPolicySetLock_);
    StorePermissionState(permStateValueList);
}

void PermissionPolicySet::GetDefPermissions(std::vector<PermissionDef>& permList)
{
    PermissionDefinitionCache::GetInstance().GetDefPermissionsByTokenId(permList, tokenId_);
}

int PermissionPolicySet::QueryPermissionFlag(const std::string& permissionName, int& flag)
{
    Utils::UniqueReadGuard<Utils::RWLock> infoGuard(this->permPolicySetLock_);
    for (const auto& perm : permStateList_) {
        if (perm.permissionName == permissionName) {
            flag = perm.grantFlag;
            return RET_SUCCESS;
        }
    }
    LOGE(ATM_DOMAIN, ATM_TAG, "Invalid params!");
    return AccessTokenError::ERR_PERMISSION_NOT_EXIST;
}

static uint32_t UpdateWithNewFlag(uint32_t oldFlag, uint32_t currFlag)
{
    uint32_t newFlag = currFlag | (oldFlag & PERMISSION_GRANTED_BY_POLICY);
    return newFlag;
}

int32_t PermissionPolicySet::UpdatePermStateList(
    const std::string& permissionName, bool isGranted, uint32_t flag)
{
    Utils::UniqueWriteGuard<Utils::RWLock> infoGuard(this->permPolicySetLock_);
    auto iter = std::find_if(permStateList_.begin(), permStateList_.end(),
        [permissionName](const PermissionStatus& permState) {
            return permissionName == permState.permissionName;
        });
    if (iter != permStateList_.end()) {
        if ((static_cast<uint32_t>(iter->grantFlag) & PERMISSION_SYSTEM_FIXED) == PERMISSION_SYSTEM_FIXED) {
            LOGE(ATM_DOMAIN, ATM_TAG, "Permission fixed by system!");
            return AccessTokenError::ERR_PARAM_INVALID;
        }
        iter->grantStatus = isGranted ? PERMISSION_GRANTED : PERMISSION_DENIED;
        iter->grantFlag = UpdateWithNewFlag(iter->grantFlag, flag);
        uint32_t opCode;
        if (!TransferPermissionToOpcode(permissionName, opCode)) {
            LOGE(ATM_DOMAIN, ATM_TAG, "permissionName is invalid %{public}s.", permissionName.c_str());
            return AccessTokenError::ERR_PARAM_INVALID;
        }
        bool status = (iter->grantStatus == PERMISSION_GRANTED) ? 1 : 0;
        return PermissionDataBrief::GetInstance().SetBriefPermData(tokenId_, opCode, status, iter->grantFlag);
    } else {
        LOGE(ATM_DOMAIN, ATM_TAG, "Permission not request!");
        return AccessTokenError::ERR_PARAM_INVALID;
    }
    return RET_SUCCESS;
}

int32_t PermissionPolicySet::UpdateSecCompGrantedPermList(
    const std::string& permissionName, bool isToGrant)
{
    int32_t flag = 0;
    int32_t ret = QueryPermissionFlag(permissionName, flag);

    LOGD(ATM_DOMAIN, ATM_TAG, "Ret is %{public}d. flag is %{public}d", ret, flag);
    // if the permission has been operated by user or the permission has been granted by system.
    if ((ConstantCommon::IsPermOperatedByUser(flag) || ConstantCommon::IsPermOperatedBySystem(flag))) {
        LOGD(ATM_DOMAIN, ATM_TAG, "The permission has been operated.");
        if (isToGrant) {
            // The data included in requested perm list.
            int32_t status = PermissionDataBrief::GetInstance().VerifyPermissionStatus(tokenId_, permissionName);
            // Permission has been granted, there is no need to add perm state in security component permList.
            if (status == PERMISSION_GRANTED) {
                return RET_SUCCESS;
            } else {
                LOGE(ATM_DOMAIN, ATM_TAG, "Permission has been revoked by user.");
                return ERR_PERMISSION_DENIED;
            }
        } else {
            /* revoke is called while the permission has been operated by user or system */
            PermissionDataBrief::GetInstance().SecCompGrantedPermListUpdated(
                tokenId_, permissionName, false);
            return RET_SUCCESS;
        }
    }
    // the permission has not been operated by user or the app has not applied for this permission in config.json
    PermissionDataBrief::GetInstance().SecCompGrantedPermListUpdated(tokenId_, permissionName, isToGrant);
    return RET_SUCCESS;
}

int32_t PermissionPolicySet::UpdatePermissionStatus(
    const std::string& permissionName, bool isGranted, uint32_t flag, bool& statusChanged)
{
    int32_t ret;
    int32_t oldStatus = PermissionDataBrief::GetInstance().VerifyPermissionStatus(tokenId_, permissionName);
    if (!ConstantCommon::IsPermGrantedBySecComp(flag)) {
        ret = UpdatePermStateList(permissionName, isGranted, flag);
    } else {
        LOGD(ATM_DOMAIN, ATM_TAG, "Permission is set by security component.");
        ret = UpdateSecCompGrantedPermList(permissionName, isGranted);
    }
    int32_t newStatus = PermissionDataBrief::GetInstance().VerifyPermissionStatus(tokenId_, permissionName);
    statusChanged = (oldStatus == newStatus) ? false : true;
    return ret;
}

void PermissionPolicySet::ResetUserGrantPermissionStatus(void)
{
    Utils::UniqueWriteGuard<Utils::RWLock> infoGuard(this->permPolicySetLock_);
    for (auto& perm : permStateList_) {
        uint32_t oldFlag = static_cast<uint32_t>(perm.grantFlag);
        if ((oldFlag & PERMISSION_SYSTEM_FIXED) != 0) {
            continue;
        }
        /* A user_grant permission has been set by system for cancellable pre-authorization. */
        /* it should keep granted when the app reset. */
        if ((oldFlag & PERMISSION_GRANTED_BY_POLICY) != 0) {
            perm.grantStatus = PERMISSION_GRANTED;
            perm.grantFlag = PERMISSION_GRANTED_BY_POLICY;
            continue;
        }
        perm.grantStatus = PERMISSION_DENIED;
        perm.grantFlag = PERMISSION_DEFAULT_FLAG;
    }
    std::vector<BriefPermData> list;
    GetPermissionBriefData(list, permStateList_);
    PermissionDataBrief::GetInstance().AddBriefPermDataByTokenId(tokenId_, list);
    PermissionDataBrief::GetInstance().ClearAllSecCompGrantedPermById(tokenId_);
}

void PermissionPolicySet::GetPermissionStateList(std::vector<PermissionStatus>& permList)
{
    Utils::UniqueReadGuard<Utils::RWLock> infoGuard(this->permPolicySetLock_);
    permList.assign(permStateList_.begin(), permStateList_.end());
}

uint32_t PermissionPolicySet::GetReqPermissionSize()
{
    return static_cast<uint32_t>(permStateList_.size());
}

void PermDefToString(const PermissionDef& def, std::string& info)
{
    info.append(R"(    {)");
    info.append("\n");
    info.append(R"(      "permissionName": ")" + def.permissionName + R"(")" + ",\n");
    info.append(R"(      "bundleName": ")" + def.bundleName + R"(")" + ",\n");
    info.append(R"(      "grantMode": )" + std::to_string(def.grantMode) + ",\n");
    info.append(R"(      "availableLevel": )" + std::to_string(def.availableLevel) + ",\n");
    info.append(R"(      "provisionEnable": )" + std::to_string(def.provisionEnable) + ",\n");
    info.append(R"(      "distributedSceneEnable": )" + std::to_string(def.distributedSceneEnable) + ",\n");
    info.append(R"(      "label": ")" + def.label + R"(")" + ",\n");
    info.append(R"(      "labelId": )" + std::to_string(def.labelId) + ",\n");
    info.append(R"(      "description": ")" + def.description + R"(")" + ",\n");
    info.append(R"(      "descriptionId": )" + std::to_string(def.descriptionId) + ",\n");
    info.append(R"(    })");
}

void PermStateFullToString(const PermissionStatus& state, std::string& info)
{
    info.append(R"(    {)");
    info.append("\n");
    info.append(R"(      "permissionName": ")" + state.permissionName + R"(")" + ",\n");
    info.append(R"(      "grantStatus": ")" + std::to_string(state.grantStatus) + R"(")" + ",\n");
    info.append(R"(      "grantFlag": ")" + std::to_string(state.grantFlag) + R"(")" + ",\n");
    info.append(R"(    })");
}

void PermissionPolicySet::ToString(std::string& info)
{
    Utils::UniqueReadGuard<Utils::RWLock> infoGuard(this->permPolicySetLock_);
    info.append(R"(  "permDefList": [)");
    info.append("\n");
    std::vector<PermissionDef> permList;
    PermissionDefinitionCache::GetInstance().GetDefPermissionsByTokenId(permList, tokenId_);
    for (auto iter = permList.begin(); iter != permList.end(); iter++) {
        PermDefToString(*iter, info);
        if (iter != (permList.end() - 1)) {
            info.append(",\n");
        }
    }
    info.append("\n  ],\n");

    info.append(R"(  "permStateList": [)");
    info.append("\n");
    for (auto iter = permStateList_.begin(); iter != permStateList_.end(); iter++) {
        PermStateFullToString(*iter, info);
        if (iter != (permStateList_.end() - 1)) {
            info.append(",\n");
        }
    }
    info.append("\n  ]\n");
}

void PermissionPolicySet::ToString(std::string& info, const std::vector<PermissionDef>& permList,
    const std::vector<PermissionStatus>& permStateList)
{
    info.append(R"(  "permDefList": [)");
    info.append("\n");
    for (auto iter = permList.begin(); iter != permList.end(); iter++) {
        PermDefToString(*iter, info);
        if (iter != (permList.end() - 1)) {
            info.append(",\n");
        }
    }
    info.append("\n  ],\n");

    info.append(R"(  "permStateList": [)");
    info.append("\n");
    for (auto iter = permStateList.begin(); iter != permStateList.end(); iter++) {
        PermStateFullToString(*iter, info);
        if (iter != (permStateList.end() - 1)) {
            info.append(",\n");
        }
    }
    info.append("\n  ]\n");
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS

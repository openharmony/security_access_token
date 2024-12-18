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
#include "accesstoken_log.h"
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
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_ACCESSTOKEN, "PermissionPolicySet"};
}

PermissionPolicySet::~PermissionPolicySet()
{
    ACCESSTOKEN_LOG_DEBUG(LABEL,
        "%{public}s called, tokenID: 0x%{public}x destruction", __func__, tokenId_);
}


void PermissionPolicySet::GetPermissionBriefData(std::vector<BriefPermData>& list,
    const std::vector<PermissionStateFull> &permStateList)
{
    for (const auto& state : permStateList) {
        BriefPermData data = {0};
        uint32_t code;
        if (TransferPermissionToOpcode(state.permissionName, code)) {
            data.status = (state.grantStatus[0] == PERMISSION_GRANTED) ? 1 : 0;
            data.permCode = code;
            data.flag = state.grantFlags[0];
            list.emplace_back(data);
        }
    }
}

std::shared_ptr<PermissionPolicySet> PermissionPolicySet::BuildPermissionPolicySet(
    AccessTokenID tokenId, const std::vector<PermissionStateFull>& permStateList)
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
    AccessTokenID tokenId, const std::vector<PermissionStateFull>& permStateList)
{
    std::shared_ptr<PermissionPolicySet> policySet = std::make_shared<PermissionPolicySet>();
    PermissionValidator::FilterInvalidPermissionState(
        TOKEN_TYPE_BUTT, false, permStateList, policySet->permStateList_);
    policySet->tokenId_ = tokenId;
    ACCESSTOKEN_LOG_INFO(LABEL, "TokenID: %{public}d, permStateList_ size: %{public}zu",
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
            PermissionStateFull state;
            int ret = DataTranslator::TranslationIntoPermissionStateFull(stateValue, state);
            if (ret == RET_SUCCESS) {
                MergePermissionStateFull(policySet->permStateList_, state);
            } else {
                ACCESSTOKEN_LOG_ERROR(LABEL, "TokenId 0%{public}u permState is wrong.", tokenId);
            }
        }
    }
    return policySet;
}

void PermissionPolicySet::UpdatePermStateFull(const PermissionStateFull& permOld, PermissionStateFull& permNew)
{
    if (permNew.isGeneral == permOld.isGeneral) {
        // if user_grant permission is not operated by user, it keeps the new initalized state.
        // the new state can be pre_authorization.
        if ((permOld.grantFlags[0] == PERMISSION_DEFAULT_FLAG) && (permOld.grantStatus[0] == PERMISSION_DENIED)) {
            return;
        }
        // if old user_grant permission is granted by pre_authorization fixed, it keeps the new initalized state.
        // the new state can be pre_authorization or not.
        if ((permOld.grantFlags[0] == PERMISSION_SYSTEM_FIXED) ||
            // if old user_grant permission is granted by pre_authorization unfixed
            // and the user has not operated this permission, it keeps the new initalized state.
            (permOld.grantFlags[0] == PERMISSION_GRANTED_BY_POLICY)) {
            return;
        }

        // if old user_grant permission has been operated by user, it keeps the old status and old flag.
        permNew.resDeviceID = permOld.resDeviceID;
        permNew.grantStatus = permOld.grantStatus;
        permNew.grantFlags = permOld.grantFlags;
    }
}

void PermissionPolicySet::Update(const std::vector<PermissionStateFull>& permStateList)
{
    std::vector<PermissionStateFull> permStateFilterList;
    PermissionValidator::FilterInvalidPermissionState(TOKEN_HAP, true, permStateList, permStateFilterList);
    ACCESSTOKEN_LOG_INFO(LABEL, "PermStateFilterList size: %{public}zu.", permStateFilterList.size());
    Utils::UniqueWriteGuard<Utils::RWLock> infoGuard(this->permPolicySetLock_);

    for (PermissionStateFull& permStateNew : permStateFilterList) {
        auto iter = std::find_if(permStateList_.begin(), permStateList_.end(),
            [permStateNew](const PermissionStateFull& permStateOld) {
                return permStateNew.permissionName == permStateOld.permissionName;
            });
        if (iter != permStateList_.end()) {
            UpdatePermStateFull(*iter, permStateNew);
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
            PermissionStateFull state;
            int ret = DataTranslator::TranslationIntoPermissionStateFull(stateValue, state);
            if (ret == RET_SUCCESS) {
                MergePermissionStateFull(policySet->permStateList_, state);
            } else {
                ACCESSTOKEN_LOG_ERROR(LABEL, "TokenId 0x%{public}x permState is wrong.", tokenId);
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

void PermissionPolicySet::MergePermissionStateFull(std::vector<PermissionStateFull>& permStateList,
    PermissionStateFull& state)
{
    uint32_t flag = GetFlagWroteToDb(state.grantFlags[0]);
    state.grantFlags[0] = flag;
    for (auto iter = permStateList.begin(); iter != permStateList.end(); iter++) {
        if (state.permissionName == iter->permissionName) {
            iter->resDeviceID.emplace_back(state.resDeviceID[0]);
            iter->grantStatus.emplace_back(state.grantStatus[0]);
            iter->grantFlags.emplace_back(state.grantFlags[0]);
            ACCESSTOKEN_LOG_DEBUG(LABEL, "Update permission: %{public}s.", state.permissionName.c_str());
            return;
        }
    }
    ACCESSTOKEN_LOG_DEBUG(LABEL, "Add permission: %{public}s.", state.permissionName.c_str());
    permStateList.emplace_back(state);
}

void PermissionPolicySet::StorePermissionState(std::vector<GenericValues>& valueList) const
{
    for (const auto& permissionState : permStateList_) {
        ACCESSTOKEN_LOG_DEBUG(LABEL, "PermissionName: %{public}s", permissionState.permissionName.c_str());
        if (permissionState.isGeneral) {
            GenericValues genericValues;
            genericValues.Put(TokenFiledConst::FIELD_TOKEN_ID, static_cast<int32_t>(tokenId_));
            DataTranslator::TranslationIntoGenericValues(permissionState, 0, genericValues);
            valueList.emplace_back(genericValues);
            continue;
        }

        unsigned int stateSize = permissionState.resDeviceID.size();
        for (unsigned int i = 0; i < stateSize; i++) {
            GenericValues genericValues;
            genericValues.Put(TokenFiledConst::FIELD_TOKEN_ID, static_cast<int32_t>(tokenId_));
            DataTranslator::TranslationIntoGenericValues(permissionState, i, genericValues);
            valueList.emplace_back(genericValues);
        }
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
            if (perm.isGeneral) {
                flag = perm.grantFlags[0];
                return RET_SUCCESS;
            } else {
                ACCESSTOKEN_LOG_ERROR(LABEL, "Permission %{public}s is invalid", permissionName.c_str());
                return AccessTokenError::ERR_PARAM_INVALID;
            }
        }
    }
    ACCESSTOKEN_LOG_ERROR(LABEL, "Invalid params!");
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
        [permissionName](const PermissionStateFull& permState) {
            return permissionName == permState.permissionName;
        });
    if (iter != permStateList_.end()) {
        if ((static_cast<uint32_t>(iter->grantFlags[0]) & PERMISSION_SYSTEM_FIXED) == PERMISSION_SYSTEM_FIXED) {
            ACCESSTOKEN_LOG_ERROR(LABEL, "Permission fixed by system!");
            return AccessTokenError::ERR_PARAM_INVALID;
        }
        iter->grantStatus[0] = isGranted ? PERMISSION_GRANTED : PERMISSION_DENIED;
        iter->grantFlags[0] = UpdateWithNewFlag(iter->grantFlags[0], flag);
        uint32_t opCode;
        if (!TransferPermissionToOpcode(permissionName, opCode)) {
            ACCESSTOKEN_LOG_ERROR(LABEL, "permissionName is invalid %{public}s.", permissionName.c_str());
            return AccessTokenError::ERR_PARAM_INVALID;
        }
        bool status = (iter->grantStatus[0] == PERMISSION_GRANTED) ? 1 : 0;
        return PermissionDataBrief::GetInstance().SetBriefPermData(tokenId_, opCode, status, iter->grantFlags[0]);
    } else {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Permission not request!");
        return AccessTokenError::ERR_PARAM_INVALID;
    }
    return RET_SUCCESS;
}

int32_t PermissionPolicySet::UpdateSecCompGrantedPermList(
    const std::string& permissionName, bool isToGrant)
{
    int32_t flag = 0;
    int32_t ret = QueryPermissionFlag(permissionName, flag);

    ACCESSTOKEN_LOG_DEBUG(LABEL, "Ret is %{public}d. flag is %{public}d", ret, flag);
    // if the permission has been operated by user or the permission has been granted by system.
    if ((ConstantCommon::IsPermOperatedByUser(flag) || ConstantCommon::IsPermOperatedBySystem(flag))) {
        ACCESSTOKEN_LOG_DEBUG(LABEL, "The permission has been operated.");
        if (isToGrant) {
            // The data included in requested perm list.
            int32_t status = PermissionDataBrief::GetInstance().VerifyPermissionStatus(tokenId_, permissionName);
            // Permission has been granted, there is no need to add perm state in security component permList.
            if (status == PERMISSION_GRANTED) {
                return RET_SUCCESS;
            } else {
                ACCESSTOKEN_LOG_ERROR(LABEL, "Permission has been revoked by user.");
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
        ACCESSTOKEN_LOG_DEBUG(LABEL, "Permission is set by security component.");
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
        if (perm.isGeneral) {
            uint32_t oldFlag = static_cast<uint32_t>(perm.grantFlags[0]);
            if ((oldFlag & PERMISSION_SYSTEM_FIXED) != 0) {
                continue;
            }
            /* A user_grant permission has been set by system for cancellable pre-authorization. */
            /* it should keep granted when the app reset. */
            if ((oldFlag & PERMISSION_GRANTED_BY_POLICY) != 0) {
                perm.grantStatus[0] = PERMISSION_GRANTED;
                perm.grantFlags[0] = PERMISSION_GRANTED_BY_POLICY;
                continue;
            }
            perm.grantStatus[0] = PERMISSION_DENIED;
            perm.grantFlags[0] = PERMISSION_DEFAULT_FLAG;
        } else {
            continue;
        }
    }
    std::vector<BriefPermData> list;
    GetPermissionBriefData(list, permStateList_);
    PermissionDataBrief::GetInstance().AddBriefPermDataByTokenId(tokenId_, list);
    PermissionDataBrief::GetInstance().ClearAllSecCompGrantedPermById(tokenId_);
}

void PermissionPolicySet::GetPermissionStateList(std::vector<PermissionStateFull>& permList)
{
    Utils::UniqueReadGuard<Utils::RWLock> infoGuard(this->permPolicySetLock_);
    permList.assign(permStateList_.begin(), permStateList_.end());
}

void PermissionPolicySet::GetPermissionStateList(std::vector<uint32_t>& opCodeList, std::vector<bool>& statusList)
{
    Utils::UniqueReadGuard<Utils::RWLock> infoGuard(this->permPolicySetLock_);
    for (const auto& state : permStateList_) {
        uint32_t code;
        if (TransferPermissionToOpcode(state.permissionName, code)) {
            opCodeList.emplace_back(code);
            statusList.emplace_back(state.grantStatus[0] == PERMISSION_GRANTED);
        }
    }
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

void PermStateFullToString(const PermissionStateFull& state, std::string& info)
{
    info.append(R"(    {)");
    info.append("\n");
    info.append(R"(      "permissionName": ")" + state.permissionName + R"(")" + ",\n");
    info.append(R"(      "isGeneral": )" + std::to_string(state.isGeneral) + ",\n");
#ifndef ATM_BUILD_VARIANT_USER_ENABLE
    info.append(R"(      "resDeviceIDList": [ )");
    for (auto iter = state.resDeviceID.begin(); iter != state.resDeviceID.end(); iter++) {
        info.append("\n");
        info.append(R"(        { "resDeviceID": ")" + *iter + R"(")" + " }");
        if (iter != (state.resDeviceID.end() - 1)) {
            info.append(",");
        }
    }
    info.append("\n      ],\n");
#endif
    info.append(R"(      "grantStatusList": [)");
    for (auto iter = state.grantStatus.begin(); iter != state.grantStatus.end(); iter++) {
        info.append("\n");
        info.append(R"(        { "grantStatus": )" + std::to_string(*iter) + " }");
        if (iter != (state.grantStatus.end() - 1)) {
            info.append(",");
        }
    }
    info.append("\n      ],\n");

    info.append(R"(      "grantFlagsList": [)");
    for (auto iter = state.grantFlags.begin(); iter != state.grantFlags.end(); iter++) {
        info.append("\n");
        info.append(R"(        { "grantFlag": )" + std::to_string(*iter) + " }");
        if (iter != (state.grantFlags.end() - 1)) {
            info.append(",");
        }
    }
    info.append("\n      ],\n");

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
    const std::vector<PermissionStateFull>& permStateList)
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

bool PermissionPolicySet::IsPermissionReqValid(int32_t tokenApl, const std::string& permissionName,
    const std::vector<std::string>& nativeAcls)
{
    PermissionDef permissionDef;
    int ret = PermissionDefinitionCache::GetInstance().FindByPermissionName(
        permissionName, permissionDef);
    if (ret != RET_SUCCESS) {
        return false;
    }
    if (tokenApl >= permissionDef.availableLevel) {
        return true;
    }

    auto iter = std::find(nativeAcls.begin(), nativeAcls.end(), permissionName);
    if (iter != nativeAcls.end()) {
        return true;
    }
    return false;
}

void PermissionPolicySet::PermStateToString(int32_t tokenApl,
    const std::vector<std::string>& nativeAcls, std::string& info)
{
    Utils::UniqueReadGuard<Utils::RWLock> infoGuard(this->permPolicySetLock_);

    std::vector<std::string> invalidPermList = {};
    info.append(R"(  "permStateList": [)");
    info.append("\n");
    for (auto iter = permStateList_.begin(); iter != permStateList_.end(); iter++) {
        if (!IsPermissionReqValid(tokenApl, iter->permissionName, nativeAcls)) {
            invalidPermList.emplace_back(iter->permissionName);
            continue;
        }
        PermStateFullToString(*iter, info);
        if (iter != (permStateList_.end() - 1)) {
            info.append(",\n");
        }
    }
    info.append("\n  ]\n");

    if (invalidPermList.empty()) {
        return;
    }

    info.append(R"(  "invalidPermList": [)");
    info.append("\n");
    for (auto iter = invalidPermList.begin(); iter != invalidPermList.end(); iter++) {
        info.append(R"(      "permissionName": ")" + *iter + R"(")" + ",\n");
    }
    info.append("\n  ]\n");
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS

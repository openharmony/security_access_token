/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "permission_manager.h"
#include "access_token.h"
#include "accesstoken_id_manager.h"
#include "accesstoken_info_manager.h"
#include "accesstoken_log.h"
#include "callback_manager.h"
#ifdef SUPPORT_SANDBOX_APP
#include "dlp_permission_set_manager.h"
#endif
#include "permission_definition_cache.h"
#include "permission_validator.h"
#ifdef TOKEN_SYNC_ENABLE
#include "token_modify_notifier.h"
#endif

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_ACCESSTOKEN, "PermissionManager"};
}

PermissionManager& PermissionManager::GetInstance()
{
    static PermissionManager instance;
    return instance;
}

PermissionManager::PermissionManager()
{
}

PermissionManager::~PermissionManager()
{
}

void PermissionManager::AddDefPermissions(const std::vector<PermissionDef>& permList, AccessTokenID tokenId,
    bool updateFlag)
{
    std::vector<PermissionDef> permFilterList;
    PermissionValidator::FilterInvalidPermissionDef(permList, permFilterList);

    for (const auto& perm : permFilterList) {
        if (!PermissionValidator::IsPermissionDefValid(perm)) {
            ACCESSTOKEN_LOG_INFO(LABEL, "invalid permission definition info: %{public}s",
                TransferPermissionDefToString(perm).c_str());
            continue;
        }

        if (updateFlag) {
            PermissionDefinitionCache::GetInstance().Update(perm);
            continue;
        }

        if (!PermissionDefinitionCache::GetInstance().HasDefinition(perm.permissionName)) {
            PermissionDefinitionCache::GetInstance().Insert(perm, tokenId);
        } else {
            ACCESSTOKEN_LOG_INFO(LABEL, "permission %{public}s has define",
                TransferPermissionDefToString(perm).c_str());
        }
    }
}

void PermissionManager::RemoveDefPermissions(AccessTokenID tokenID)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "%{public}s called, tokenID: %{public}u", __func__, tokenID);
    std::shared_ptr<HapTokenInfoInner> tokenInfo =
        AccessTokenInfoManager::GetInstance().GetHapTokenInfoInner(tokenID);
    if (tokenInfo == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params(tokenID: %{public}u)!", tokenID);
        return;
    }
    std::string bundleName = tokenInfo->GetBundleName();
    PermissionDefinitionCache::GetInstance().DeleteByBundleName(bundleName);
}

int PermissionManager::VerifyHapAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    std::shared_ptr<HapTokenInfoInner> tokenInfoPtr =
        AccessTokenInfoManager::GetInstance().GetHapTokenInfoInner(tokenID);
    if (tokenInfoPtr == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "can not find tokenInfo!");
        return PERMISSION_DENIED;
    }
    std::shared_ptr<PermissionPolicySet> permPolicySet = tokenInfoPtr->GetHapInfoPermissionPolicySet();
    if (permPolicySet == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return PERMISSION_DENIED;
    }

    return permPolicySet->VerifyPermissStatus(permissionName);
}

int PermissionManager::VerifyNativeAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    std::shared_ptr<NativeTokenInfoInner> tokenInfoPtr =
        AccessTokenInfoManager::GetInstance().GetNativeTokenInfoInner(tokenID);
    if (tokenInfoPtr == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "can not find tokenInfo!");
        return PERMISSION_DENIED;
    }

    NativeTokenInfo info;
    tokenInfoPtr->TranslateToNativeTokenInfo(info);
    if (PermissionDefinitionCache::GetInstance().IsPermissionDefEmpty()) {
        ACCESSTOKEN_LOG_INFO(LABEL, "permission definition set has not been installed!");
        if (info.apl >= APL_SYSTEM_BASIC) {
            return PERMISSION_GRANTED;
        }
        ACCESSTOKEN_LOG_INFO(LABEL, "native process apl is %{public}d!", info.apl);
        return PERMISSION_DENIED;
    }
    if (!tokenInfoPtr->IsRemote() && !PermissionDefinitionCache::GetInstance().HasDefinition(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "no definition for permission: %{public}s!", permissionName.c_str());
        return PERMISSION_DENIED;
    }
    std::shared_ptr<PermissionPolicySet> permPolicySet =
        AccessTokenInfoManager::GetInstance().GetNativePermissionPolicySet(tokenID);
    if (permPolicySet == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return PERMISSION_DENIED;
    }

    return permPolicySet->VerifyPermissStatus(permissionName);
}

int PermissionManager::VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "%{public}s called, tokenID: %{public}u, permissionName: %{public}s", __func__,
        tokenID, permissionName.c_str());
    if (!PermissionValidator::IsPermissionNameValid(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return PERMISSION_DENIED;
    }

    ATokenTypeEnum tokenType = AccessTokenIDManager::GetInstance().GetTokenIdTypeEnum(tokenID);
    if ((tokenType == TOKEN_NATIVE) || (tokenType == TOKEN_SHELL)) {
        return VerifyNativeAccessToken(tokenID, permissionName);
    }
    if (tokenType == TOKEN_HAP) {
        return VerifyHapAccessToken(tokenID, permissionName);
    }
    ACCESSTOKEN_LOG_ERROR(LABEL, "invalid tokenType!");
    return PERMISSION_DENIED;
}

int PermissionManager::VerifyNativeToken(AccessTokenID tokenID, const std::string& permissionName)
{
    ACCESSTOKEN_LOG_INFO(LABEL,
        "%{public}s called, tokenID: %{public}u, permissionName: %{public}s", __func__,
        tokenID, permissionName.c_str());

    PermissionDef permissionInfo;
    NativeTokenInfo nativeTokenInfo;
    int res = PermissionManager::GetDefPermission(permissionName, permissionInfo);
    if (res != RET_SUCCESS) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "GetDefPermission in %{public}s failed", __func__);
        return PERMISSION_DENIED;
    }
    res = AccessTokenInfoManager::GetInstance().GetNativeTokenInfo(tokenID, nativeTokenInfo);
    if (res != RET_SUCCESS) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "GetNativeTokenInfo in %{public}s failed", __func__);
        return PERMISSION_DENIED;
    }
    if (permissionInfo.availableLevel > nativeTokenInfo.apl) {
        return PERMISSION_DENIED;
    }
    return PERMISSION_GRANTED;
}

int PermissionManager::GetDefPermission(const std::string& permissionName, PermissionDef& permissionDefResult)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "%{public}s called, permissionName: %{public}s", __func__, permissionName.c_str());
    if (!PermissionValidator::IsPermissionNameValid(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return RET_FAILED;
    }
    if (!PermissionDefinitionCache::GetInstance().HasDefinition(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "no definition for permission: %{public}s!", permissionName.c_str());
        return RET_FAILED;
    }
    return PermissionDefinitionCache::GetInstance().FindByPermissionName(permissionName, permissionDefResult);
}

int PermissionManager::GetDefPermissions(AccessTokenID tokenID, std::vector<PermissionDef>& permList)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "%{public}s called, tokenID: %{public}u", __func__, tokenID);
    std::shared_ptr<PermissionPolicySet> permPolicySet =
        AccessTokenInfoManager::GetInstance().GetHapPermissionPolicySet(tokenID);
    if (permPolicySet == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return RET_FAILED;
    }

    permPolicySet->GetDefPermissions(permList);
    return RET_SUCCESS;
}

int PermissionManager::GetReqPermissions(
    AccessTokenID tokenID, std::vector<PermissionStateFull>& reqPermList, bool isSystemGrant)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "%{public}s called, tokenID: %{public}u, isSystemGrant: %{public}d",
        __func__, tokenID, isSystemGrant);
    std::shared_ptr<PermissionPolicySet> permPolicySet =
        AccessTokenInfoManager::GetInstance().GetHapPermissionPolicySet(tokenID);
    if (permPolicySet == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return RET_FAILED;
    }

    GrantMode mode = isSystemGrant ? SYSTEM_GRANT : USER_GRANT;
    std::vector<PermissionStateFull> tmpList;
    permPolicySet->GetPermissionStateFulls(tmpList);
    for (const auto& perm : tmpList) {
        PermissionDef permDef;
        GetDefPermission(perm.permissionName, permDef);
        if (permDef.grantMode == mode) {
            reqPermList.emplace_back(perm);
        }
    }
    return RET_SUCCESS;
}

void PermissionManager::GetSelfPermissionState(std::vector<PermissionStateFull> permsList,
    PermissionListState &permState, int32_t apiVersion)
{
    bool foundGoal = false;
    int32_t goalGrantStatus;
    uint32_t goalGrantFlags;

    // api8 require vague location permission refuse directlty beause there is no vague location permission in api8
    if ((permState.permissionName == VAGUE_LOCATION_PERMISSION_NAME) &&
       (apiVersion < ACCURATE_LOCATION_API_VERSION)) {
        permState.state = INVALID_OPER;
        return;
    }

    for (const auto& perm : permsList) {
        if (perm.permissionName == permState.permissionName) {
            ACCESSTOKEN_LOG_INFO(LABEL, "find goal permission: %{public}s, status: %{public}d, flag: %{public}d",
                permState.permissionName.c_str(), perm.grantStatus[0], perm.grantFlags[0]);
            foundGoal = true;
            goalGrantStatus = perm.grantStatus[0];
            goalGrantFlags = static_cast<uint32_t>(perm.grantFlags[0]);
            break;
        }
    }
    if (foundGoal == false) {
        ACCESSTOKEN_LOG_WARN(LABEL,
            "can not find permission: %{public}s define!", permState.permissionName.c_str());
        permState.state = INVALID_OPER;
        return;
    }
    if (!PermissionDefinitionCache::GetInstance().HasDefinition(permState.permissionName)) {
        ACCESSTOKEN_LOG_WARN(LABEL,
            "no definition for permission: %{public}s!", permState.permissionName.c_str());
        permState.state = INVALID_OPER;
        return;
    }

    if (goalGrantStatus == PERMISSION_DENIED) {
        if ((goalGrantFlags == PERMISSION_DEFAULT_FLAG) ||
            ((goalGrantFlags & PERMISSION_USER_SET) != 0)) {
            permState.state = DYNAMIC_OPER;
            return;
        }
        if ((goalGrantFlags & PERMISSION_USER_FIXED) != 0) {
            permState.state = SETTING_OPER;
            return;
        }
    }

    permState.state = PASS_OPER;
    return;
}

int PermissionManager::GetPermissionFlag(AccessTokenID tokenID, const std::string& permissionName)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "%{public}s called, tokenID: %{public}u, permissionName: %{public}s",
        __func__, tokenID, permissionName.c_str());
    if (!PermissionValidator::IsPermissionNameValid(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return PERMISSION_DEFAULT_FLAG;
    }
    if (!PermissionDefinitionCache::GetInstance().HasDefinition(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "no definition for permission: %{public}s!", permissionName.c_str());
        return PERMISSION_DEFAULT_FLAG;
    }
    std::shared_ptr<PermissionPolicySet> permPolicySet =
        AccessTokenInfoManager::GetInstance().GetHapPermissionPolicySet(tokenID);
    if (permPolicySet == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return PERMISSION_DEFAULT_FLAG;
    }
    return permPolicySet->QueryPermissionFlag(permissionName);
}

void PermissionManager::UpdateTokenPermissionState(
    AccessTokenID tokenID, const std::string& permissionName, bool isGranted, int flag)
{
    std::shared_ptr<HapTokenInfoInner> infoPtr = AccessTokenInfoManager::GetInstance().GetHapTokenInfoInner(tokenID);
    if (infoPtr == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return;
    }
    if (infoPtr->IsRemote()) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "remote token can not update");
        return;
    }

    std::shared_ptr<PermissionPolicySet> permPolicySet = infoPtr->GetHapInfoPermissionPolicySet();
    if (permPolicySet == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return;
    }
#ifdef SUPPORT_SANDBOX_APP
    int32_t dlpType = infoPtr->GetDlpType();
    if (isGranted && dlpType != DLP_COMMON) {
        int32_t dlpMode = DlpPermissionSetManager::GetInstance().GetPermDlpMode(permissionName);
        if (DlpPermissionSetManager::GetInstance().IsPermStateNeedUpdate(dlpType, dlpMode)) {
            ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}u is not allowed to be granted permissionName %{public}s",
                tokenID, permissionName.c_str());
            return;
        }
    }
#endif
    bool isUpdated = permPolicySet->UpdatePermissionStatus(permissionName, isGranted, static_cast<uint32_t>(flag));
    if (isUpdated) {
        ACCESSTOKEN_LOG_INFO(LABEL, "isUpdated");
        int32_t changeType = isGranted ? GRANTED : REVOKED;
        CallbackManager::GetInstance().ExecuteCallbackAsync(tokenID, permissionName, changeType);
    }

#ifdef TOKEN_SYNC_ENABLE
    TokenModifyNotifier::GetInstance().NotifyTokenModify(tokenID);
#endif
}

void PermissionManager::GrantPermission(AccessTokenID tokenID, const std::string& permissionName, int flag)
{
    ACCESSTOKEN_LOG_INFO(LABEL,
        "%{public}s called, tokenID: %{public}u, permissionName: %{public}s, flag: %{public}d",
        __func__, tokenID, permissionName.c_str(), flag);
    if (!PermissionValidator::IsPermissionNameValid(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return;
    }
    if (!PermissionDefinitionCache::GetInstance().HasDefinition(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "no definition for permission: %{public}s!", permissionName.c_str());
        return;
    }
    if (!PermissionValidator::IsPermissionFlagValid(flag)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return;
    }
    UpdateTokenPermissionState(tokenID, permissionName, true, flag);
}

void PermissionManager::RevokePermission(AccessTokenID tokenID, const std::string& permissionName, int flag)
{
    ACCESSTOKEN_LOG_INFO(LABEL,
        "%{public}s called, tokenID: %{public}u, permissionName: %{public}s, flag: %{public}d",
        __func__, tokenID, permissionName.c_str(), flag);
    if (!PermissionValidator::IsPermissionNameValid(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return;
    }
    if (!PermissionDefinitionCache::GetInstance().HasDefinition(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "no definition for permission: %{public}s!", permissionName.c_str());
        return;
    }
    if (!PermissionValidator::IsPermissionFlagValid(flag)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return;
    }
    UpdateTokenPermissionState(tokenID, permissionName, false, flag);
}

int32_t PermissionManager::AddPermStateChangeCallback(
    const PermStateChangeScope& scope, const sptr<IRemoteObject>& callback)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "called");
    auto callbackScopePtr = std::make_shared<PermStateChangeScope>(scope);
    if (callbackScopePtr == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "callbackScopePtr is nullptr");
    }

    return CallbackManager::GetInstance().AddCallback(callbackScopePtr, callback);
}

int32_t PermissionManager::RemovePermStateChangeCallback(const sptr<IRemoteObject>& callback)
{
    return CallbackManager::GetInstance().RemoveCallback(callback);
}

bool PermissionManager::GetApiVersionByTokenId(AccessTokenID tokenID, int32_t& apiVersion)
{
    // only hap can do this
    AccessTokenIDInner *idInner = reinterpret_cast<AccessTokenIDInner *>(&tokenID);
    ATokenTypeEnum tokenType = (ATokenTypeEnum)(idInner->type);
    if (tokenType != TOKEN_HAP) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid token type %{public}d", tokenType);
        return false;
    }

    HapTokenInfo hapInfo;
    int ret = AccessTokenInfoManager::GetInstance().GetHapTokenInfo(tokenID, hapInfo);
    if (ret != RET_SUCCESS) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "get hap token info error!");
        return false;
    }

    apiVersion = hapInfo.apiVersion;

    return true;
}

bool PermissionManager::GetLocationPermissionIndex(std::vector<PermissionListStateParcel>& reqPermList,
    int& vagueIndex, int& accurateIndex)
{
    int index = 0;
    bool hasFound = false;

    for (const auto& perm : reqPermList) {
        if (perm.permsState.permissionName == VAGUE_LOCATION_PERMISSION_NAME) {
            vagueIndex = index;
            hasFound = true;
        } else if (perm.permsState.permissionName == ACCURATE_LOCATION_PERMISSION_NAME) {
            accurateIndex = index;
            hasFound = true;
        }

        index++;

        if ((vagueIndex != ELEMENT_NOT_FOUND) && (accurateIndex != ELEMENT_NOT_FOUND)) {
            break;
        }
    }

    ACCESSTOKEN_LOG_INFO(LABEL,
        "vague location permission index is %{public}d, accurate location permission index is %{public}d!",
        vagueIndex, accurateIndex);

    return hasFound;
}

bool PermissionManager::IsPermissionVaild(const std::string& permissionName)
{
    if (!PermissionValidator::IsPermissionNameValid(permissionName)) {
        ACCESSTOKEN_LOG_WARN(LABEL, "invalid permissionName %{public}s", permissionName.c_str());
        return false;
    }

    if (!PermissionDefinitionCache::GetInstance().HasDefinition(permissionName)) {
        ACCESSTOKEN_LOG_WARN(LABEL, "permission %{public}s has no definition ", permissionName.c_str());
        return false;
    }
    return true;
}

bool PermissionManager::GetPermissionStatusAndFlag(const std::string& permissionName,
    const std::vector<PermissionStateFull>& permsList, int32_t& status, uint32_t& flag)
{
    if (!IsPermissionVaild(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid permission %{public}s", permissionName.c_str());
        return false;
    }

    for (const auto& perm : permsList) {
        if (perm.permissionName == permissionName) {
            status = perm.grantStatus[0];
            flag = static_cast<uint32_t>(perm.grantFlags[0]);

            ACCESSTOKEN_LOG_DEBUG(LABEL, "permission:%{public}s, status:%{public}d, flag:%{public}d!",
                permissionName.c_str(), status, flag);
            return true;
        }
    }
    return false;
}

void PermissionManager::AllLocationPermissionHandle(std::vector<PermissionListStateParcel>& reqPermList,
    std::vector<PermissionStateFull> permsList, int vagueIndex, int accurateIndex)
{
    int32_t vagueStatus = PERMISSION_DENIED;
    uint32_t vagueFlag = PERMISSION_DEFAULT_FLAG;
    int32_t vagueState = INVALID_OPER;
    int32_t accurateStatus = PERMISSION_DENIED;
    uint32_t accurateFlag = PERMISSION_DEFAULT_FLAG;
    int32_t accurateState = INVALID_OPER;

    if (!GetPermissionStatusAndFlag(VAGUE_LOCATION_PERMISSION_NAME, permsList, vagueStatus, vagueFlag) ||
        !GetPermissionStatusAndFlag(ACCURATE_LOCATION_PERMISSION_NAME, permsList, accurateStatus, accurateFlag)) {
        return;
    }

    // vague location status -1 means vague location permission has been refused
    if (vagueStatus == PERMISSION_DENIED) {
        if ((vagueFlag == PERMISSION_DEFAULT_FLAG) || ((vagueFlag & PERMISSION_USER_SET) != 0)) {
            // vague location flag 0 or 1 means permission has not been operated or valid only once
            vagueState = DYNAMIC_OPER;
            accurateState = DYNAMIC_OPER;
        } else if ((vagueFlag & PERMISSION_USER_FIXED) != 0) {
            // vague location flag 2 means vague location has been operated, only can be changed by settings
            // so that accurate location is no need to operate
            vagueState = SETTING_OPER;
            accurateState = SETTING_OPER;
        }
    } else if (vagueStatus == PERMISSION_GRANTED) {
        // vague location status 0 means vague location permission has been accepted
        // now flag 1 is not in use so return PASS_OPER, otherwise should judge by flag
        vagueState = PASS_OPER;

        if (accurateStatus == PERMISSION_DENIED) {
            if ((accurateFlag == PERMISSION_DEFAULT_FLAG) || ((accurateFlag & PERMISSION_USER_SET) != 0)) {
                accurateState = DYNAMIC_OPER;
            } else if ((accurateFlag & PERMISSION_USER_FIXED) != 0) {
                accurateState = SETTING_OPER;
            }
        } else if (accurateStatus == PERMISSION_GRANTED) {
            accurateState = PASS_OPER;
        }
    }

    ACCESSTOKEN_LOG_INFO(LABEL,
        "vague location permission state is %{public}d, accurate location permission state is %{public}d",
        vagueState, accurateState);

    reqPermList[vagueIndex].permsState.state = vagueState;
    reqPermList[accurateIndex].permsState.state = accurateState;
}

bool PermissionManager::LocationPermissionSpecialHandle(std::vector<PermissionListStateParcel>& reqPermList,
    int32_t apiVersion, std::vector<PermissionStateFull> permsList, int vagueIndex, int accurateIndex)
{
    if ((vagueIndex != ELEMENT_NOT_FOUND) && (accurateIndex == ELEMENT_NOT_FOUND)) {
        // only vague location permission
        GetSelfPermissionState(permsList, reqPermList[vagueIndex].permsState, apiVersion);
        if (reqPermList[vagueIndex].permsState.state == DYNAMIC_OPER) {
            return true;
        }
    }

    if ((vagueIndex == ELEMENT_NOT_FOUND) && (accurateIndex != ELEMENT_NOT_FOUND)) {
        // only accurate location permission refuse directly
        ACCESSTOKEN_LOG_ERROR(LABEL, "operate invaild, accurate location permission base on vague location permission");
        reqPermList[accurateIndex].permsState.state = INVALID_OPER;
        return false;
    }

    // all location permissions
    AllLocationPermissionHandle(reqPermList, permsList, vagueIndex, accurateIndex);
    return ((reqPermList[vagueIndex].permsState.state == DYNAMIC_OPER) ||
        (reqPermList[accurateIndex].permsState.state == DYNAMIC_OPER));
}

void PermissionManager::ClearUserGrantedPermissionState(AccessTokenID tokenID)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "%{public}s called, tokenID: %{public}u", __func__, tokenID);
    std::shared_ptr<HapTokenInfoInner> infoPtr = AccessTokenInfoManager::GetInstance().GetHapTokenInfoInner(tokenID);
    if (infoPtr == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return;
    }
    if (infoPtr->IsRemote()) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "remote token can not clear.");
        return;
    }

    std::shared_ptr<PermissionPolicySet> permPolicySet = infoPtr->GetHapInfoPermissionPolicySet();
    if (permPolicySet == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "invalid params!");
        return;
    }

    permPolicySet->ResetUserGrantPermissionStatus();
}

std::string PermissionManager::TransferPermissionDefToString(const PermissionDef& inPermissionDef)
{
    std::string infos;
    infos.append(R"({"permissionName": ")" + inPermissionDef.permissionName + R"(")");
    infos.append(R"(, "bundleName": ")" + inPermissionDef.bundleName + R"(")");
    infos.append(R"(, "grantMode": )" + std::to_string(inPermissionDef.grantMode));
    infos.append(R"(, "availableLevel": )" + std::to_string(inPermissionDef.availableLevel));
    infos.append(R"(, "provisionEnable": )" + std::to_string(inPermissionDef.provisionEnable));
    infos.append(R"(, "distributedSceneEnable": )" + std::to_string(inPermissionDef.distributedSceneEnable));
    infos.append(R"(, "label": ")" + inPermissionDef.label + R"(")");
    infos.append(R"(, "labelId": )" + std::to_string(inPermissionDef.labelId));
    infos.append(R"(, "description": ")" + inPermissionDef.description + R"(")");
    infos.append(R"(, "descriptionId": )" + std::to_string(inPermissionDef.descriptionId));
    infos.append("}");
    return infos;
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS

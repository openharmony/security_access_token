/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <iostream>
#include <numeric>
#include <sstream>

#include "access_token.h"
#include "access_token_error.h"
#include "accesstoken_dfx_define.h"
#include "accesstoken_id_manager.h"
#include "accesstoken_info_manager.h"
#include "accesstoken_log.h"
#include "access_token_db.h"
#include "app_manager_access_client.h"
#include "callback_manager.h"
#include "constant_common.h"
#ifdef SUPPORT_SANDBOX_APP
#include "dlp_permission_set_manager.h"
#endif
#include "ipc_skeleton.h"
#include "parameter.h"
#include "permission_definition_cache.h"
#include "short_grant_manager.h"
#include "permission_map.h"
#include "permission_validator.h"
#include "perm_setproc.h"
#include "token_field_const.h"
#ifdef TOKEN_SYNC_ENABLE
#include "token_modify_notifier.h"
#endif

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_ACCESSTOKEN, "PermissionManager"};
static const char* PERMISSION_STATUS_CHANGE_KEY = "accesstoken.permission.change";
static constexpr int32_t VALUE_MAX_LEN = 32;
static constexpr int32_t BASE_USER_RANGE = 200000;
static const std::vector<std::string> g_notDisplayedPerms = {
    "ohos.permission.ANSWER_CALL",
    "ohos.permission.MANAGE_VOICEMAIL",
    "ohos.permission.READ_CELL_MESSAGES",
    "ohos.permission.READ_MESSAGES",
    "ohos.permission.RECEIVE_MMS",
    "ohos.permission.RECEIVE_SMS",
    "ohos.permission.RECEIVE_WAP_MESSAGES",
    "ohos.permission.SEND_MESSAGES",
    "ohos.permission.READ_CALL_LOG",
    "ohos.permission.WRITE_CALL_LOG",
    "ohos.permission.SHORT_TERM_WRITE_IMAGEVIDEO"
};
constexpr const char* APP_DISTRIBUTION_TYPE_ENTERPRISE_MDM = "enterprise_mdm";
}
PermissionManager* PermissionManager::implInstance_ = nullptr;
std::recursive_mutex PermissionManager::mutex_;

PermissionManager& PermissionManager::GetInstance()
{
    if (implInstance_ == nullptr) {
        std::lock_guard<std::recursive_mutex> lock_l(mutex_);
        if (implInstance_ == nullptr) {
            implInstance_ = new PermissionManager();
        }
    }
    return *implInstance_;
}

void PermissionManager::RegisterImpl(PermissionManager* implInstance)
{
    implInstance_ = implInstance;
}

PermissionManager::PermissionManager()
{
    char value[VALUE_MAX_LEN] = {0};
    int32_t ret = GetParameter(PERMISSION_STATUS_CHANGE_KEY, "", value, VALUE_MAX_LEN - 1);
    if (ret < 0) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Return default value, ret=%{public}d", ret);
        paramValue_ = 0;
        return;
    }
    paramValue_ = static_cast<uint64_t>(std::atoll(value));
}

PermissionManager::~PermissionManager()
{}

void PermissionManager::AddDefPermissions(const std::vector<PermissionDef>& permList, AccessTokenID tokenId,
    bool updateFlag)
{
    std::vector<PermissionDef> permFilterList;
    PermissionValidator::FilterInvalidPermissionDef(permList, permFilterList);
    ACCESSTOKEN_LOG_INFO(LABEL, "PermFilterList size: %{public}zu", permFilterList.size());
    for (const auto& perm : permFilterList) {
        if (updateFlag) {
            PermissionDefinitionCache::GetInstance().Update(perm, tokenId);
            continue;
        }

        if (!PermissionDefinitionCache::GetInstance().HasDefinition(perm.permissionName)) {
            PermissionDefinitionCache::GetInstance().Insert(perm, tokenId);
        } else {
            PermissionDefinitionCache::GetInstance().Update(perm, tokenId);
            ACCESSTOKEN_LOG_INFO(LABEL, "Permission %{public}s has define", perm.permissionName.c_str());
        }
    }
}

void PermissionManager::RemoveDefPermissions(AccessTokenID tokenID)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "tokenID: %{public}u", tokenID);
    PermissionDefinitionCache::GetInstance().DeleteByToken(tokenID);
}

int PermissionManager::VerifyHapAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    return HapTokenInfoInner::VerifyPermissionStatus(tokenID, permissionName); // 从data获取
}

PermUsedTypeEnum PermissionManager::GetPermissionUsedType(
    AccessTokenID tokenID, const std::string& permissionName)
{
    if ((tokenID == INVALID_TOKENID) ||
        (TOKEN_HAP != AccessTokenIDManager::GetInstance().GetTokenIdTypeEnum(tokenID))) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "TokenID: %{public}d is invalid.", tokenID);
        return PermUsedTypeEnum::INVALID_USED_TYPE;
    }
    PermUsedTypeEnum ret = HapTokenInfoInner::GetPermissionUsedType(tokenID, permissionName);
    ACCESSTOKEN_LOG_INFO(LABEL,
        "Application %{public}u apply for %{public}s for type %{public}d.", tokenID, permissionName.c_str(), ret);
    return ret;
}

int PermissionManager::GetDefPermission(const std::string& permissionName, PermissionDef& permissionDefResult)
{
    if (!PermissionValidator::IsPermissionNameValid(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Invalid params!");
        return AccessTokenError::ERR_PARAM_INVALID;
    }
    return PermissionDefinitionCache::GetInstance().FindByPermissionName(permissionName, permissionDefResult);
}

void PermissionManager::GetDefPermissions(AccessTokenID tokenID, std::vector<PermissionDef>& permList)
{
    PermissionDefinitionCache::GetInstance().GetDefPermissionsByTokenId(permList, tokenID);
}

int PermissionManager::GetReqPermissions(
    AccessTokenID tokenID, std::vector<PermissionStateFull>& reqPermList, bool isSystemGrant)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s called, tokenID: %{public}u, isSystemGrant: %{public}d",
        __func__, tokenID, isSystemGrant);
    std::shared_ptr<HapTokenInfoInner> infoPtr = AccessTokenInfoManager::GetInstance().GetHapTokenInfoInner(tokenID);
    if (infoPtr == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Token %{public}u is invalid.", tokenID);
        return AccessTokenError::ERR_TOKENID_NOT_EXIST;
    }
    GrantMode mode = isSystemGrant ? SYSTEM_GRANT : USER_GRANT;
    std::vector<PermissionStateFull> tmpList;
    int32_t ret = infoPtr->GetPermissionStateList(tmpList);
    if (ret != RET_SUCCESS) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "GetPermissionStateList failed, token %{public}u is invalid.", tokenID);
        return ret;
    }
    for (const auto& perm : tmpList) {
        PermissionDef permDef;
        GetDefPermission(perm.permissionName, permDef);
        if (permDef.grantMode == mode) {
            reqPermList.emplace_back(perm);
        }
    }
    return RET_SUCCESS;
}

static bool IsPermissionRequestedInHap(const std::vector<PermissionStateFull>& permsList,
    const std::string &permission, int32_t& status, uint32_t& flag)
{
    if (!PermissionDefinitionCache::GetInstance().HasHapPermissionDefinitionForHap(permission)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "No definition for hap permission: %{public}s!", permission.c_str());
        return false;
    }
    auto iter = std::find_if(permsList.begin(), permsList.end(), [permission](const PermissionStateFull& perm) {
        return permission == perm.permissionName;
    });
    if (iter == permsList.end()) {
        ACCESSTOKEN_LOG_WARN(LABEL, "Can not find permission: %{public}s define!", permission.c_str());
        return false;
    }
    ACCESSTOKEN_LOG_DEBUG(LABEL, "Find goal permission: %{public}s, status: %{public}d, flag: %{public}d",
        permission.c_str(), iter->grantStatus[0], iter->grantFlags[0]);
    status = iter->grantStatus[0];
    flag = static_cast<uint32_t>(iter->grantFlags[0]);
    return true;
}

static bool IsPermissionRestrictedByRules(const std::string& permission)
{
    // Several permission is not available to common apps.
    // Specified apps can get the permission by pre-authorization instead of Pop-ups.
    auto iterator = std::find(g_notDisplayedPerms.begin(), g_notDisplayedPerms.end(), permission);
    if (iterator != g_notDisplayedPerms.end()) {
        ACCESSTOKEN_LOG_WARN(LABEL, "Permission is not available to common apps: %{public}s!", permission.c_str());
        return true;
    }

#ifdef SUPPORT_SANDBOX_APP
    // Specified dlp permissions are limited to specified dlp type hap.
    AccessTokenID callingTokenId = IPCSkeleton::GetCallingTokenID();
    int32_t dlpType = AccessTokenInfoManager::GetInstance().GetHapTokenDlpType(callingTokenId);
    if ((dlpType != DLP_COMMON) &&
        !DlpPermissionSetManager::GetInstance().IsPermissionAvailableToDlpHap(dlpType, permission)) {
        ACCESSTOKEN_LOG_WARN(LABEL,
            "callingTokenId is not allowed to grant dlp permission: %{public}s!", permission.c_str());
        return true;
    }
#endif

    return false;
}

void PermissionManager::GetSelfPermissionState(const std::vector<PermissionStateFull>& permsList,
    PermissionListState& permState, int32_t apiVersion)
{
    int32_t goalGrantStatus;
    uint32_t goalGrantFlag;

    // api8 require vague location permission refuse directly because there is no vague location permission in api8
    if ((permState.permissionName == VAGUE_LOCATION_PERMISSION_NAME) && (apiVersion < ACCURATE_LOCATION_API_VERSION)) {
        permState.state = INVALID_OPER;
        return;
    }
    if (!IsPermissionRequestedInHap(permsList, permState.permissionName, goalGrantStatus, goalGrantFlag)) {
        permState.state = INVALID_OPER;
        return;
    }
    if (IsPermissionRestrictedByRules(permState.permissionName)) {
        permState.state = INVALID_OPER;
        return;
    }
    ACCESSTOKEN_LOG_INFO(LABEL, "%{public}s: status: %{public}d, flag: %{public}d",
        permState.permissionName.c_str(), goalGrantStatus, goalGrantFlag);
    if (goalGrantStatus == PERMISSION_DENIED) {
        if ((goalGrantFlag & PERMISSION_POLICY_FIXED) != 0) {
            permState.state = SETTING_OPER;
            return;
        }

        if ((goalGrantFlag == PERMISSION_DEFAULT_FLAG) || ((goalGrantFlag & PERMISSION_USER_SET) != 0) ||
            ((goalGrantFlag & PERMISSION_COMPONENT_SET) != 0) || ((goalGrantFlag & PERMISSION_ALLOW_THIS_TIME) != 0)) {
            permState.state = DYNAMIC_OPER;
            return;
        }
        if ((goalGrantFlag & PERMISSION_USER_FIXED) != 0) {
            permState.state = SETTING_OPER;
            return;
        }
    }
    permState.state = PASS_OPER;
    return;
}

int PermissionManager::GetPermissionFlag(AccessTokenID tokenID, const std::string& permissionName, uint32_t& flag)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "%{public}s called, tokenID: %{public}u, permissionName: %{public}s",
        __func__, tokenID, permissionName.c_str());
    if (!PermissionValidator::IsPermissionNameValid(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "PermissionName:%{public}s invalid!", permissionName.c_str());
        return AccessTokenError::ERR_PARAM_INVALID;
    }
    if (!PermissionDefinitionCache::GetInstance().HasDefinition(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "No definition for permission: %{public}s!", permissionName.c_str());
        return AccessTokenError::ERR_PERMISSION_NOT_EXIST;
    }
    uint32_t fullFlag;
    int32_t ret = HapTokenInfoInner::QueryPermissionFlag(tokenID, permissionName, fullFlag);
    if (ret == RET_SUCCESS) {
        flag = ConstantCommon::GetFlagWithoutSpecifiedElement(fullFlag, PERMISSION_GRANTED_BY_POLICY);
    }
    return ret;
}

int32_t PermissionManager::FindPermRequestToggleStatusFromDb(int32_t userID, const std::string& permissionName)
{
    std::vector<GenericValues> permRequestToggleStatusRes;
    GenericValues conditionValue;
    conditionValue.Put(TokenFiledConst::FIELD_USER_ID, userID);
    conditionValue.Put(TokenFiledConst::FIELD_PERMISSION_NAME, permissionName);

    AccessTokenDb::GetInstance().Find(AtmDataType::ACCESSTOKEN_PERMISSION_REQUEST_TOGGLE_STATUS,
        conditionValue, permRequestToggleStatusRes);
    if (permRequestToggleStatusRes.empty()) {
        // never set, return default status: CLOSED if APP_TRACKING_CONSENT
        return (permissionName == "ohos.permission.APP_TRACKING_CONSENT") ?
            PermissionRequestToggleStatus::CLOSED : PermissionRequestToggleStatus::OPEN;
    }
    return permRequestToggleStatusRes[0].GetInt(TokenFiledConst::FIELD_REQUEST_TOGGLE_STATUS);
}

void PermissionManager::AddPermRequestToggleStatusToDb(
    int32_t userID, const std::string& permissionName, int32_t status)
{
    Utils::UniqueWriteGuard<Utils::RWLock> infoGuard(this->permToggleStateLock_);
    GenericValues value;
    value.Put(TokenFiledConst::FIELD_USER_ID, userID);
    value.Put(TokenFiledConst::FIELD_PERMISSION_NAME, permissionName);
    AccessTokenDb::GetInstance().Remove(AtmDataType::ACCESSTOKEN_PERMISSION_REQUEST_TOGGLE_STATUS, value);

    std::vector<GenericValues> permRequestToggleStatusValues;
    value.Put(TokenFiledConst::FIELD_REQUEST_TOGGLE_STATUS, status);
    permRequestToggleStatusValues.emplace_back(value);
    AccessTokenDb::GetInstance().Add(AtmDataType::ACCESSTOKEN_PERMISSION_REQUEST_TOGGLE_STATUS,
        permRequestToggleStatusValues);
}

int32_t PermissionManager::SetPermissionRequestToggleStatus(const std::string& permissionName, uint32_t status,
    int32_t userID)
{
    if (userID == 0) {
        userID = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    }

    ACCESSTOKEN_LOG_INFO(LABEL, "UserID=%{public}u, permissionName=%{public}s, status=%{public}d", userID,
        permissionName.c_str(), status);
    if (!PermissionValidator::IsUserIdValid(userID)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "UserID is invalid.");
        return AccessTokenError::ERR_PARAM_INVALID;
    }
    if (!PermissionValidator::IsPermissionNameValid(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Permission name is invalid.");
        return AccessTokenError::ERR_PARAM_INVALID;
    }
    if (!PermissionDefinitionCache::GetInstance().HasDefinition(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "Permission=%{public}s is not defined.", permissionName.c_str());
        return AccessTokenError::ERR_PERMISSION_NOT_EXIST;
    }
    if (PermissionDefinitionCache::GetInstance().IsSystemGrantedPermission(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Only support permissions of user_grant to set.");
        return AccessTokenError::ERR_PARAM_INVALID;
    }
    if (!PermissionValidator::IsToggleStatusValid(status)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Status is invalid.");
        return AccessTokenError::ERR_PARAM_INVALID;
    }

    AddPermRequestToggleStatusToDb(userID, permissionName, status);

    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::ACCESS_TOKEN, "PERM_DIALOG_STATUS_INFO",
        HiviewDFX::HiSysEvent::EventType::STATISTIC, "USERID", userID, "PERMISSION_NAME", permissionName,
        "TOGGLE_STATUS", status);

    return 0;
}

int32_t PermissionManager::GetPermissionRequestToggleStatus(const std::string& permissionName, uint32_t& status,
    int32_t userID)
{
    if (userID == 0) {
        userID = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    }

    ACCESSTOKEN_LOG_INFO(LABEL, "UserID=%{public}u, permissionName=%{public}s", userID, permissionName.c_str());
    if (!PermissionValidator::IsUserIdValid(userID)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "UserID is invalid.");
        return AccessTokenError::ERR_PARAM_INVALID;
    }
    if (!PermissionValidator::IsPermissionNameValid(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Permission name is invalid.");
        return AccessTokenError::ERR_PARAM_INVALID;
    }
    if (!PermissionDefinitionCache::GetInstance().HasDefinition(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "Permission=%{public}s is not defined.", permissionName.c_str());
        return AccessTokenError::ERR_PERMISSION_NOT_EXIST;
    }
    if (PermissionDefinitionCache::GetInstance().IsSystemGrantedPermission(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Only support permissions of user_grant to get.");
        return AccessTokenError::ERR_PARAM_INVALID;
    }

    status = static_cast<uint32_t>(FindPermRequestToggleStatusFromDb(userID, permissionName));

    return 0;
}

void PermissionManager::ParamUpdate(const std::string& permissionName, uint32_t flag, bool filtered)
{
    Utils::UniqueWriteGuard<Utils::RWLock> infoGuard(this->permParamSetLock_);
    if (filtered || (PermissionDefinitionCache::GetInstance().IsUserGrantedPermission(permissionName) &&
        ((flag != PERMISSION_GRANTED_BY_POLICY) && (flag != PERMISSION_SYSTEM_FIXED)))) {
        paramValue_++;
        ACCESSTOKEN_LOG_DEBUG(LABEL,
            "paramValue_ change %{public}llu", static_cast<unsigned long long>(paramValue_));
        int32_t res = SetParameter(PERMISSION_STATUS_CHANGE_KEY, std::to_string(paramValue_).c_str());
        if (res != 0) {
            ACCESSTOKEN_LOG_ERROR(LABEL, "SetParameter failed %{public}d", res);
        }
    }
}

void PermissionManager::NotifyWhenPermissionStateUpdated(AccessTokenID tokenID, const std::string& permissionName,
    bool isGranted, uint32_t flag, const std::shared_ptr<HapTokenInfoInner>& infoPtr)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "IsUpdated");
    int32_t changeType = isGranted ? STATE_CHANGE_GRANTED : STATE_CHANGE_REVOKED;

    // set to kernel(grant/revoke)
    SetPermToKernel(tokenID, permissionName, isGranted);

    // To notify the listener register.
    CallbackManager::GetInstance().ExecuteCallbackAsync(tokenID, permissionName, changeType);

    // To notify the client cache to update by resetting paramValue_.
    ParamUpdate(permissionName, flag, false);

    // DFX.
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::ACCESS_TOKEN, "PERMISSION_CHECK_EVENT",
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "CODE", USER_GRANT_PERMISSION_EVENT,
        "CALLER_TOKENID", tokenID, "PERMISSION_NAME", permissionName, "FLAG", flag,
        "PERMISSION_GRANT_TYPE", changeType);
    grantEvent_.AddEvent(tokenID, permissionName, infoPtr->permUpdateTimestamp_);
}

int32_t PermissionManager::UpdateTokenPermissionState(
    AccessTokenID id, const std::string& permission, bool isGranted, uint32_t flag, bool needKill)
{
    std::shared_ptr<HapTokenInfoInner> infoPtr = AccessTokenInfoManager::GetInstance().GetHapTokenInfoInner(id);
    if (infoPtr == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "tokenInfo is null, tokenId=%{public}u", id);
        return AccessTokenError::ERR_TOKENID_NOT_EXIST;
    }
    if (infoPtr->IsRemote()) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Remote token can not update");
        return AccessTokenError::ERR_IDENTITY_CHECK_FAILED;
    }
    if ((flag == PERMISSION_ALLOW_THIS_TIME) && isGranted) {
        if (!TempPermissionObserver::GetInstance().IsAllowGrantTempPermission(id, permission)) {
            ACCESSTOKEN_LOG_ERROR(LABEL, "Id:%{public}d fail to grant permission:%{public}s", id, permission.c_str());
            return ERR_IDENTITY_CHECK_FAILED;
        }
    }

#ifdef SUPPORT_SANDBOX_APP
    int32_t hapDlpType = infoPtr->GetDlpType();
    if (hapDlpType != DLP_COMMON) {
        int32_t permDlpMode = DlpPermissionSetManager::GetInstance().GetPermDlpMode(permission);
        if (!DlpPermissionSetManager::GetInstance().IsPermDlpModeAvailableToDlpHap(hapDlpType, permDlpMode)) {
            ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s cannot to be granted to %{public}u", permission.c_str(), id);
            return AccessTokenError::ERR_IDENTITY_CHECK_FAILED;
        }
    }
#endif
    // statusBefore cannot use VerifyPermissionStatus in permPolicySet, because the function exclude secComp
    bool isSecCompGrantedBefore = HapTokenInfoInner::IsPermissionGrantedWithSecComp(id, permission);
    bool statusChanged = false;
    int32_t ret = infoPtr->UpdatePermissionStatus(permission, isGranted, flag, statusChanged);
    if (ret != RET_SUCCESS) {
        return ret;
    }
    if (statusChanged) {
        NotifyWhenPermissionStateUpdated(id, permission, isGranted, flag, infoPtr);
        // To notify kill process when perm is revoke
        if (needKill && (!isGranted && !isSecCompGrantedBefore)) {
            ACCESSTOKEN_LOG_INFO(LABEL, "(%{public}s) is revoked, kill process(%{public}u).", permission.c_str(), id);
            if ((ret = AppManagerAccessClient::GetInstance().KillProcessesByAccessTokenId(id)) != ERR_OK) {
                ACCESSTOKEN_LOG_ERROR(LABEL, "kill process failed, ret=%{public}d.", ret);
            }
        }
    }

#ifdef TOKEN_SYNC_ENABLE
    TokenModifyNotifier::GetInstance().NotifyTokenModify(id);
#endif
    return RET_SUCCESS;
}

int32_t PermissionManager::UpdatePermission(AccessTokenID tokenID, const std::string& permissionName,
    bool isGranted, uint32_t flag, bool needKill)
{
    int32_t ret = UpdateTokenPermissionState(tokenID, permissionName, isGranted, flag, needKill);
    if (ret != RET_SUCCESS) {
        return ret;
    }

#ifdef SUPPORT_SANDBOX_APP
    // The action of sharing would be taken place only if the grant operation or revoke operation equals to success.
    std::vector<AccessTokenID> tokenIdList;
    AccessTokenInfoManager::GetInstance().GetRelatedSandBoxHapList(tokenID, tokenIdList);
    for (const auto& id : tokenIdList) {
        (void)UpdateTokenPermissionState(id, permissionName, isGranted, flag, needKill);
    }
#endif

    // DFX
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::ACCESS_TOKEN, "UPDATE_PERMISSION",
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "TOKENID", tokenID, "PERMISSION_NAME",
        permissionName, "PERMISSION_FLAG", flag, "GRANTED_FLAG", isGranted);
    return RET_SUCCESS;
}

int32_t PermissionManager::CheckAndUpdatePermission(AccessTokenID tokenID, const std::string& permissionName,
    bool isGranted, uint32_t flag)
{
    if (!PermissionValidator::IsPermissionNameValid(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "permissionName: %{public}s, Invalid params!", permissionName.c_str());
        return AccessTokenError::ERR_PARAM_INVALID;
    }
    if (!PermissionDefinitionCache::GetInstance().HasDefinition(permissionName)) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "No definition for permission: %{public}s!", permissionName.c_str());
        return AccessTokenError::ERR_PERMISSION_NOT_EXIST;
    }
    if (!PermissionValidator::IsPermissionFlagValid(flag)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "flag: %{public}d, Invalid params!", flag);
        return AccessTokenError::ERR_PARAM_INVALID;
    }
    bool needKill = false;
    // To kill process when perm is revoke
    if (!isGranted && (flag != PERMISSION_ALLOW_THIS_TIME) && (flag != PERMISSION_COMPONENT_SET)) {
        ACCESSTOKEN_LOG_INFO(LABEL, "Perm(%{public}s) is revoked, kill process(%{public}u).",
            permissionName.c_str(), tokenID);
        needKill = true;
    }

    return UpdatePermission(tokenID, permissionName, isGranted, flag, needKill);
}

int32_t PermissionManager::GrantPermission(AccessTokenID tokenID, const std::string& permissionName, uint32_t flag)
{
    ACCESSTOKEN_LOG_INFO(LABEL,
        "%{public}s called, tokenID: %{public}u, permissionName: %{public}s, flag: %{public}d",
        __func__, tokenID, permissionName.c_str(), flag);
    return CheckAndUpdatePermission(tokenID, permissionName, true, flag);
}

int32_t PermissionManager::RevokePermission(AccessTokenID tokenID, const std::string& permissionName, uint32_t flag)
{
    ACCESSTOKEN_LOG_INFO(LABEL,
        "%{public}s called, tokenID: %{public}u, permissionName: %{public}s, flag: %{public}d",
        __func__, tokenID, permissionName.c_str(), flag);
    return CheckAndUpdatePermission(tokenID, permissionName, false, flag);
}

int32_t PermissionManager::GrantPermissionForSpecifiedTime(
    AccessTokenID tokenID, const std::string& permissionName, uint32_t onceTime)
{
    ACCESSTOKEN_LOG_INFO(LABEL,
        "%{public}s called, tokenID: %{public}u, permissionName: %{public}s, onceTime: %{public}d",
        __func__, tokenID, permissionName.c_str(), onceTime);
    return ShortGrantManager::GetInstance().RefreshPermission(tokenID, permissionName, onceTime);
}

void PermissionManager::ScopeToString(
    const std::vector<AccessTokenID>& tokenIDs, const std::vector<std::string>& permList)
{
    std::stringstream str;
    copy(tokenIDs.begin(), tokenIDs.end(), std::ostream_iterator<uint32_t>(str, ", "));
    std::string tokenidStr = str.str();

    std::string permStr;
    permStr = accumulate(permList.begin(), permList.end(), std::string(" "));

    ACCESSTOKEN_LOG_INFO(LABEL, "TokenidStr = %{public}s permStr =%{public}s",
        tokenidStr.c_str(), permStr.c_str());
}

int32_t PermissionManager::ScopeFilter(const PermStateChangeScope& scopeSrc, PermStateChangeScope& scopeRes)
{
    std::set<uint32_t> tokenIdSet;
    for (const auto& tokenId : scopeSrc.tokenIDs) {
        if (AccessTokenInfoManager::GetInstance().IsTokenIdExist(tokenId) &&
            (tokenIdSet.count(tokenId) == 0)) {
            scopeRes.tokenIDs.emplace_back(tokenId);
            tokenIdSet.insert(tokenId);
            continue;
        }
        ACCESSTOKEN_LOG_ERROR(LABEL, "TokenId %{public}d invalid!", tokenId);
    }
    std::set<std::string> permSet;
    for (const auto& permissionName : scopeSrc.permList) {
        if (PermissionDefinitionCache::GetInstance().HasDefinition(permissionName) &&
            permSet.count(permissionName) == 0) {
            scopeRes.permList.emplace_back(permissionName);
            permSet.insert(permissionName);
            continue;
        }
        ACCESSTOKEN_LOG_ERROR(LABEL, "Permission %{public}s invalid!", permissionName.c_str());
    }
    if ((scopeRes.tokenIDs.empty()) && (!scopeSrc.tokenIDs.empty())) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Valid tokenid size is 0!");
        return AccessTokenError::ERR_PARAM_INVALID;
    }
    if ((scopeRes.permList.empty()) && (!scopeSrc.permList.empty())) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Valid permission size is 0!");
        return AccessTokenError::ERR_PARAM_INVALID;
    }
    ScopeToString(scopeRes.tokenIDs, scopeRes.permList);
    return RET_SUCCESS;
}

int32_t PermissionManager::AddPermStateChangeCallback(
    const PermStateChangeScope& scope, const sptr<IRemoteObject>& callback)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "Called");
    PermStateChangeScope scopeRes;
    int32_t result = ScopeFilter(scope, scopeRes);
    if (result != RET_SUCCESS) {
        return result;
    }
    return CallbackManager::GetInstance().AddCallback(scope, callback);
}

int32_t PermissionManager::RemovePermStateChangeCallback(const sptr<IRemoteObject>& callback)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "Called");
    return CallbackManager::GetInstance().RemoveCallback(callback);
}

bool PermissionManager::GetApiVersionByTokenId(AccessTokenID tokenID, int32_t& apiVersion)
{
    // only hap can do this
    AccessTokenIDInner *idInner = reinterpret_cast<AccessTokenIDInner *>(&tokenID);
    ATokenTypeEnum tokenType = (ATokenTypeEnum)(idInner->type);
    if (tokenType != TOKEN_HAP) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Invalid token type %{public}d", tokenType);
        return false;
    }

    HapTokenInfo hapInfo;
    int ret = AccessTokenInfoManager::GetInstance().GetHapTokenInfo(tokenID, hapInfo);
    if (ret != RET_SUCCESS) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Get hap token info error!");
        return false;
    }

    apiVersion = hapInfo.apiVersion;

    return true;
}

bool PermissionManager::IsPermissionVaild(const std::string& permissionName)
{
    if (!PermissionValidator::IsPermissionNameValid(permissionName)) {
        ACCESSTOKEN_LOG_WARN(LABEL, "Invalid permissionName %{public}s", permissionName.c_str());
        return false;
    }

    if (!PermissionDefinitionCache::GetInstance().HasDefinition(permissionName)) {
        ACCESSTOKEN_LOG_WARN(LABEL, "Permission %{public}s has no definition ", permissionName.c_str());
        return false;
    }
    return true;
}

bool PermissionManager::GetLocationPermissionIndex(std::vector<PermissionListStateParcel>& reqPermList,
    LocationIndex& locationIndex)
{
    uint32_t index = 0;
    bool hasFound = false;

    for (const auto& perm : reqPermList) {
        if (perm.permsState.permissionName == VAGUE_LOCATION_PERMISSION_NAME) {
            locationIndex.vagueIndex = index;
            hasFound = true;
        } else if (perm.permsState.permissionName == ACCURATE_LOCATION_PERMISSION_NAME) {
            locationIndex.accurateIndex = index;
            hasFound = true;
        } else if (perm.permsState.permissionName == BACKGROUND_LOCATION_PERMISSION_NAME) {
            locationIndex.backIndex = index;
            hasFound = true;
        }

        index++;

        if ((locationIndex.vagueIndex != PERMISSION_NOT_REQUSET) &&
            (locationIndex.accurateIndex != PERMISSION_NOT_REQUSET) &&
            (locationIndex.backIndex != PERMISSION_NOT_REQUSET)) {
            break;
        }
    }

    ACCESSTOKEN_LOG_INFO(LABEL,
        "vague index is %{public}d, accurate index is %{public}d, background index is %{public}d!",
        locationIndex.vagueIndex, locationIndex.accurateIndex, locationIndex.backIndex);

    return hasFound;
}

bool PermissionManager::GetLocationPermissionState(AccessTokenID tokenID,
    std::vector<PermissionListStateParcel>& reqPermList, std::vector<PermissionStateFull>& permsList,
    int32_t apiVersion, const LocationIndex& locationIndex)
{
    bool needVagueDynamic = false;
    bool needAccurateDynamic = false; // needVagueDynamic-false, 1. not request;2. request but not equal to DYNAMIC_OPER
    if (locationIndex.vagueIndex != PERMISSION_NOT_REQUSET) {
        GetSelfPermissionState(permsList, reqPermList[locationIndex.vagueIndex].permsState, apiVersion);
        needVagueDynamic = reqPermList[locationIndex.vagueIndex].permsState.state == DYNAMIC_OPER;
    }

    if (locationIndex.accurateIndex != PERMISSION_NOT_REQUSET) {
        bool isVagueGranted = VerifyHapAccessToken(tokenID, VAGUE_LOCATION_PERMISSION_NAME) == PERMISSION_GRANTED;
        // request accurate and vague permission, if vague has been set or invalid, accurate can't be requested
        GetSelfPermissionState(permsList, reqPermList[locationIndex.accurateIndex].permsState, apiVersion);
        needAccurateDynamic = reqPermList[locationIndex.accurateIndex].permsState.state == DYNAMIC_OPER;

        // update permsState
        if (needAccurateDynamic) {
            // vague permissoion is not pop and permission status os not granted
            if (!needVagueDynamic && !isVagueGranted) {
                reqPermList[locationIndex.accurateIndex].permsState.state = INVALID_OPER;
                needAccurateDynamic = false;
            }
        }
    }

    if (locationIndex.backIndex != PERMISSION_NOT_REQUSET) {
        if (apiVersion >= BACKGROUND_LOCATION_API_VERSION) {
            // background permission
            // with back and vague permission, request back can not pop dynamic dialog
            if (locationIndex.vagueIndex != PERMISSION_NOT_REQUSET) {
                reqPermList[locationIndex.vagueIndex].permsState.state = INVALID_OPER;
            }
            if (locationIndex.accurateIndex != PERMISSION_NOT_REQUSET) {
                reqPermList[locationIndex.accurateIndex].permsState.state = INVALID_OPER;
            }
            reqPermList[locationIndex.backIndex].permsState.state = INVALID_OPER;
            return false;
        }
        // with back and vague permission
        // back state is SETTING_OPER when dynamic pop-up dialog appears and INVALID_OPER when it doesn't
        GetSelfPermissionState(permsList, reqPermList[locationIndex.backIndex].permsState, apiVersion);
        if (reqPermList[locationIndex.backIndex].permsState.state == DYNAMIC_OPER) {
            if (needAccurateDynamic || needVagueDynamic) {
                reqPermList[locationIndex.backIndex].permsState.state = SETTING_OPER;
            } else {
                reqPermList[locationIndex.backIndex].permsState.state = INVALID_OPER;
            }
        }
    }
    return needVagueDynamic || needAccurateDynamic;
}

bool PermissionManager::LocationPermissionSpecialHandle(
    AccessTokenID tokenID,
    std::vector<PermissionListStateParcel>& reqPermList,
    std::vector<PermissionStateFull>& permsList, int32_t apiVersion)
{
    struct LocationIndex locationIndex;
    if (!GetLocationPermissionIndex(reqPermList, locationIndex)) {
        return false;
    }
    return GetLocationPermissionState(tokenID, reqPermList, permsList, apiVersion, locationIndex);
}

void PermissionManager::NotifyUpdatedPermList(const std::vector<std::string>& grantedPermListBefore,
    const std::vector<std::string>& grantedPermListAfter, AccessTokenID tokenID)
{
    for (uint32_t i = 0; i < grantedPermListBefore.size(); i++) {
        ACCESSTOKEN_LOG_INFO(LABEL, "grantedPermListBefore[i] %{public}s.", grantedPermListBefore[i].c_str());
        auto it = find(grantedPermListAfter.begin(), grantedPermListAfter.end(), grantedPermListBefore[i]);
        if (it == grantedPermListAfter.end()) {
            CallbackManager::GetInstance().ExecuteCallbackAsync(
                tokenID, grantedPermListBefore[i], STATE_CHANGE_REVOKED);
            ParamUpdate(grantedPermListBefore[i], 0, true);
        }
    }
    for (uint32_t i = 0; i < grantedPermListAfter.size(); i++) {
        ACCESSTOKEN_LOG_INFO(LABEL, "grantedPermListAfter[i] %{public}s.", grantedPermListAfter[i].c_str());
        auto it = find(grantedPermListBefore.begin(), grantedPermListBefore.end(), grantedPermListAfter[i]);
        if (it == grantedPermListBefore.end()) {
            CallbackManager::GetInstance().ExecuteCallbackAsync(
                tokenID, grantedPermListAfter[i], STATE_CHANGE_GRANTED);
            ParamUpdate(grantedPermListAfter[i], 0, false);
        }
    }
}

bool PermissionManager::IsPermissionStateOrFlagMatched(const PermissionStateFull& state1,
    const PermissionStateFull& state2)
{
    return ((state1.grantStatus[0] == state2.grantStatus[0]) && (state1.grantFlags[0] == state2.grantFlags[0]));
}

void PermissionManager::GetStateOrFlagChangedList(std::vector<PermissionStateFull>& stateListBefore,
    std::vector<PermissionStateFull>& stateListAfter, std::vector<PermissionStateFull>& stateChangeList)
{
    uint32_t size = stateListBefore.size();

    for (uint32_t i = 0; i < size; ++i) {
        PermissionStateFull state1 = stateListBefore[i];
        PermissionStateFull state2 = stateListAfter[i];

        if (!IsPermissionStateOrFlagMatched(state1, state2)) {
            stateChangeList.emplace_back(state2);
        }
    }
}

void PermissionManager::NotifyPermGrantStoreResult(bool result, uint64_t timestamp)
{
    grantEvent_.NotifyPermGrantStoreResult(result, timestamp);
}

void PermissionManager::AddPermToKernel(AccessTokenID tokenID, const std::shared_ptr<PermissionPolicySet>& policy)
{
    if (policy == nullptr) {
        return;
    }
    std::vector<uint32_t> opCodeList;
    std::vector<bool> statusList;
    policy->GetPermissionStateList(opCodeList, statusList);
    int32_t ret = AddPermissionToKernel(tokenID, opCodeList, statusList);
    if (ret != ACCESS_TOKEN_OK) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "AddPermissionToKernel(token=%{public}d), size=%{public}zu, err=%{public}d",
            tokenID, opCodeList.size(), ret);
    }
}

void PermissionManager::AddPermToKernel(AccessTokenID tokenID)
{
    std::vector<uint32_t> opCodeList;
    std::vector<bool> statusList;
    std::vector<uint32_t> EmptyList;
    HapTokenInfoInner::GetPermStatusListByTokenId(tokenID, EmptyList, opCodeList, statusList);
    int32_t ret = AddPermissionToKernel(tokenID, opCodeList, statusList);
    if (ret != ACCESS_TOKEN_OK) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "AddPermissionToKernel(token=%{public}d), size=%{public}zu, err=%{public}d",
            tokenID, opCodeList.size(), ret);
    }
}

void PermissionManager::AddPermToKernel(AccessTokenID tokenID, const std::vector<std::string>& permList)
{
    std::vector<uint32_t> permCodeList;
    for (const auto &permission : permList) {
        uint32_t code;
        if (!TransferPermissionToOpcode(permission, code)) {
            continue;
        }
        permCodeList.emplace_back(code);
    }

    std::vector<uint32_t> opCodeList;
    std::vector<bool> statusList;
    HapTokenInfoInner::GetPermStatusListByTokenId(tokenID, permCodeList, opCodeList, statusList);
    int32_t ret = AddPermissionToKernel(tokenID, opCodeList, statusList);
    if (ret != ACCESS_TOKEN_OK) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "AddPermissionToKernel(token=%{public}d), size=%{public}zu, err=%{public}d",
            tokenID, opCodeList.size(), ret);
    }
}

void PermissionManager::RemovePermFromKernel(AccessTokenID tokenID)
{
    int32_t ret = RemovePermissionFromKernel(tokenID);
    ACCESSTOKEN_LOG_INFO(LABEL,
        "RemovePermissionFromKernel(token=%{public}d), err=%{public}d", tokenID, ret);
}

void PermissionManager::SetPermToKernel(
    AccessTokenID tokenID, const std::string& permissionName, bool isGranted)
{
    uint32_t code;
    if (!TransferPermissionToOpcode(permissionName, code)) {
        return;
    }
    int32_t ret = SetPermissionToKernel(tokenID, code, isGranted);
    ACCESSTOKEN_LOG_INFO(LABEL,
        "SetPermissionToKernel(token=%{public}d, permission=(%{public}s), err=%{public}d",
        tokenID, permissionName.c_str(), ret);
}

bool IsAclSatisfied(const PermissionDef& permDef, const HapPolicyParams& policy)
{
    if (policy.apl < permDef.availableLevel) {
        if (!permDef.provisionEnable) {
            ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s provisionEnable is false.", permDef.permissionName.c_str());
            return false;
        }
        auto isAclExist = std::any_of(
            policy.aclRequestedList.begin(), policy.aclRequestedList.end(), [permDef](const auto &perm) {
            return permDef.permissionName == perm;
        });
        if (!isAclExist) {
            ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s need acl.", permDef.permissionName.c_str());
            return false;
        }
    }
    return true;
}

bool IsPermAvailableRangeSatisfied(const PermissionDef& permDef, const std::string& appDistributionType)
{
    if (permDef.availableType == ATokenAvailableTypeEnum::MDM) {
        if (appDistributionType == "none") {
            ACCESSTOKEN_LOG_INFO(LABEL, "Debug app use permission: %{public}s.",
                permDef.permissionName.c_str());
            return true;
        }
        if (appDistributionType != APP_DISTRIBUTION_TYPE_ENTERPRISE_MDM) {
            ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s is a mdm permission, the hap is not a mdm application.",
                permDef.permissionName.c_str());
            return false;
        }
    }
    return true;
}

bool IsUserGrantPermPreAuthorized(const std::vector<PreAuthorizationInfo> &list,
    const std::string &permissionName, bool &userCancelable)
{
    auto iter = std::find_if(list.begin(), list.end(), [&permissionName](const auto &info) {
            return info.permissionName == permissionName;
        });
    if (iter == list.end()) {
        ACCESSTOKEN_LOG_INFO(LABEL, "Permission(%{public}s) is not in the list", permissionName.c_str());
        return false;
    }

    userCancelable = iter->userCancelable;
    return true;
}

bool PermissionManager::InitDlpPermissionList(const std::string& bundleName, int32_t userId,
    std::vector<PermissionStateFull>& initializedList)
{
    // get dlp original app
    AccessTokenIDEx tokenId = AccessTokenInfoManager::GetInstance().GetHapTokenID(userId, bundleName, 0);
    std::shared_ptr<HapTokenInfoInner> infoPtr =
        AccessTokenInfoManager::GetInstance().GetHapTokenInfoInner(tokenId.tokenIdExStruct.tokenID);
    if (infoPtr == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Token %{public}u is invalid.", tokenId.tokenIdExStruct.tokenID);
        return false;
    }
    (void)infoPtr->GetPermissionStateList(initializedList);
    return true;
}

bool PermissionManager::InitPermissionList(const std::string& appDistributionType,
    const HapPolicyParams& policy, std::vector<PermissionStateFull>& initializedList)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "Before, request perm list size: %{public}zu, preAuthorizationInfo size %{public}zu, "
        "ACLRequestedList size %{public}zu.",
        policy.permStateList.size(), policy.preAuthorizationInfo.size(), policy.aclRequestedList.size());

    for (auto state : policy.permStateList) {
        PermissionDef permDef;
        int32_t ret = PermissionManager::GetInstance().GetDefPermission(
            state.permissionName, permDef);
        if (ret != AccessToken::AccessTokenKitRet::RET_SUCCESS) {
            ACCESSTOKEN_LOG_ERROR(LABEL, "Get definition of %{public}s failed, ret = %{public}d.",
                state.permissionName.c_str(), ret);
            continue;
        }
        if (!IsAclSatisfied(permDef, policy)) {
            ACCESSTOKEN_LOG_ERROR(LABEL, "Acl of %{public}s is invalid.", permDef.permissionName.c_str());
            return false;
        }

        // edm check
        if (!IsPermAvailableRangeSatisfied(permDef, appDistributionType)) {
            ACCESSTOKEN_LOG_ERROR(LABEL, "Available range of %{public}s is invalid.", permDef.permissionName.c_str());
            return false;
        }
        state.grantFlags[0] = PERMISSION_DEFAULT_FLAG;
        state.grantStatus[0] = PERMISSION_DENIED;

        if (permDef.grantMode == AccessToken::GrantMode::SYSTEM_GRANT) {
            state.grantFlags[0] = PERMISSION_SYSTEM_FIXED;
            state.grantStatus[0] = PERMISSION_GRANTED;
            initializedList.emplace_back(state);
            continue;
        }
        if (policy.preAuthorizationInfo.size() == 0) {
            initializedList.emplace_back(state);
            continue;
        }
        bool userCancelable = true;
        if (IsUserGrantPermPreAuthorized(policy.preAuthorizationInfo, state.permissionName, userCancelable)) {
            state.grantFlags[0] = userCancelable ? PERMISSION_GRANTED_BY_POLICY : PERMISSION_SYSTEM_FIXED;
            state.grantStatus[0] = PERMISSION_GRANTED;
        }
        initializedList.emplace_back(state);
    }
    ACCESSTOKEN_LOG_INFO(LABEL, "After, request perm list size: %{public}zu.", initializedList.size());
    return true;
}

} // namespace AccessToken
} // namespace Security
} // namespace OHOS

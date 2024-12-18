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

#include "accesstoken_manager_service.h"

#include <unistd.h>

#include "access_token.h"
#include "access_token_error.h"
#include "accesstoken_dfx_define.h"
#include "accesstoken_id_manager.h"
#include "accesstoken_info_manager.h"
#include "accesstoken_log.h"
#include "config_policy_loader.h"
#include "constant_common.h"
#ifdef SUPPORT_SANDBOX_APP
#include "dlp_permission_set_parser.h"
#endif
#include "hap_token_info.h"
#include "hap_token_info_inner.h"
#include "hisysevent_adapter.h"
#ifdef HITRACE_NATIVE_ENABLE
#include "hitrace_meter.h"
#endif
#include "ipc_skeleton.h"
#include "libraryloader.h"
#include "native_token_info_inner.h"
#include "native_token_receptor.h"
#include "parameter.h"
#include "permission_list_state.h"
#include "permission_manager.h"
#include "short_grant_manager.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "permission_definition_parser.h"
#ifdef TOKEN_SYNC_ENABLE
#include "token_modify_notifier.h"
#endif // TOKEN_SYNC_ENABLE

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_ACCESSTOKEN, "ATMServ"
};
static const char* ACCESS_TOKEN_SERVICE_INIT_KEY = "accesstoken.permission.init";
constexpr int32_t ERROR = -1;
constexpr int TWO_ARGS = 2;
const char* GRANT_ABILITY_BUNDLE_NAME = "com.ohos.permissionmanager";
const char* GRANT_ABILITY_ABILITY_NAME = "com.ohos.permissionmanager.GrantAbility";
const char* PERMISSION_STATE_SHEET_ABILITY_NAME = "com.ohos.permissionmanager.PermissionStateSheetAbility";
const char* GLOBAL_SWITCH_SHEET_ABILITY_NAME = "com.ohos.permissionmanager.GlobalSwitchSheetAbility";
}

const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<AccessTokenManagerService>::GetInstance().get());

AccessTokenManagerService::AccessTokenManagerService()
    : SystemAbility(SA_ID_ACCESSTOKEN_MANAGER_SERVICE, true), state_(ServiceRunningState::STATE_NOT_START)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "AccessTokenManagerService()");
}

AccessTokenManagerService::~AccessTokenManagerService()
{
    ACCESSTOKEN_LOG_INFO(LABEL, "~AccessTokenManagerService()");
}

void AccessTokenManagerService::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        ACCESSTOKEN_LOG_INFO(LABEL, "AccessTokenManagerService has already started!");
        return;
    }
    ACCESSTOKEN_LOG_INFO(LABEL, "AccessTokenManagerService is starting.");
    if (!Initialize()) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Failed to initialize.");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    bool ret = Publish(DelayedSingleton<AccessTokenManagerService>::GetInstance().get());
    if (!ret) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Failed to publish service!");
        ReportSysEventServiceStartError(SA_PUBLISH_FAILED, "Publish accesstoken_service fail.", ERROR);
        return;
    }
    AccessTokenServiceParamSet();
    (void)AddSystemAbilityListener(SECURITY_COMPONENT_SERVICE_ID);
#ifdef TOKEN_SYNC_ENABLE
    (void)AddSystemAbilityListener(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
#endif
    ACCESSTOKEN_LOG_INFO(LABEL, "Congratulations, AccessTokenManagerService start successfully!");
}

void AccessTokenManagerService::OnStop()
{
    ACCESSTOKEN_LOG_INFO(LABEL, "Stop service.");
    state_ = ServiceRunningState::STATE_NOT_START;
}

void AccessTokenManagerService::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
#ifdef TOKEN_SYNC_ENABLE
    if (systemAbilityId == DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID) {
        AccessTokenInfoManager::GetInstance().InitDmCallback();
    }
#endif
}

void AccessTokenManagerService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    if (systemAbilityId == SECURITY_COMPONENT_SERVICE_ID) {
        HapTokenInfoInner::ClearAllSecCompGrantedPerm();
        return;
    }
}

PermUsedTypeEnum AccessTokenManagerService::GetPermissionUsedType(
    AccessTokenID tokenID, const std::string& permissionName)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "TokenID=%{public}d, permission=%{public}s", tokenID, permissionName.c_str());
    return PermissionManager::GetInstance().GetPermissionUsedType(tokenID, permissionName);
}

int AccessTokenManagerService::VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
#ifdef HITRACE_NATIVE_ENABLE
    StartTrace(HITRACE_TAG_ACCESS_CONTROL, "AccessTokenVerifyPermission");
#endif
    int32_t res = AccessTokenInfoManager::GetInstance().VerifyAccessToken(tokenID, permissionName);
    ACCESSTOKEN_LOG_DEBUG(LABEL, "TokenID: %{public}d, permission: %{public}s, res %{public}d",
        tokenID, permissionName.c_str(), res);
    if ((res == PERMISSION_GRANTED) &&
        (AccessTokenIDManager::GetInstance().GetTokenIdTypeEnum(tokenID) == TOKEN_HAP)) {
        res = AccessTokenInfoManager::GetInstance().IsPermissionRestrictedByUserPolicy(tokenID, permissionName) ?
            PERMISSION_DENIED : PERMISSION_GRANTED;
    }
#ifdef HITRACE_NATIVE_ENABLE
    FinishTrace(HITRACE_TAG_ACCESS_CONTROL);
#endif
    return res;
}

int AccessTokenManagerService::GetDefPermission(
    const std::string& permissionName, PermissionDefParcel& permissionDefResult)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "Permission: %{public}s", permissionName.c_str());
    return PermissionManager::GetInstance().GetDefPermission(permissionName, permissionDefResult.permissionDef);
}

int AccessTokenManagerService::GetDefPermissions(AccessTokenID tokenID, std::vector<PermissionDefParcel>& permList)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "TokenID: %{public}d", tokenID);
    std::vector<PermissionDef> permVec;
    PermissionManager::GetInstance().GetDefPermissions(tokenID, permVec);
    for (const auto& perm : permVec) {
        PermissionDefParcel permParcel;
        permParcel.permissionDef = perm;
        permList.emplace_back(permParcel);
    }
    return RET_SUCCESS;
}

int AccessTokenManagerService::GetReqPermissions(
    AccessTokenID tokenID, std::vector<PermissionStateFullParcel>& reqPermList, bool isSystemGrant)
{
    std::vector<PermissionStateFull> permList;
    int ret = PermissionManager::GetInstance().GetReqPermissions(tokenID, permList, isSystemGrant);

    for (const auto& perm : permList) {
        PermissionStateFullParcel permParcel;
        permParcel.permStatFull = perm;
        reqPermList.emplace_back(permParcel);
    }
    return ret;
}

PermissionOper AccessTokenManagerService::GetSelfPermissionsState(std::vector<PermissionListStateParcel>& reqPermList,
    PermissionGrantInfoParcel& infoParcel)
{
    infoParcel.info.grantBundleName = grantBundleName_;
    infoParcel.info.grantAbilityName = grantAbilityName_;
    infoParcel.info.grantServiceAbilityName = grantServiceAbilityName_;
    AccessTokenID callingTokenID = IPCSkeleton::GetCallingTokenID();
    return GetPermissionsState(callingTokenID, reqPermList);
}

int32_t AccessTokenManagerService::GetPermissionsStatus(AccessTokenID tokenID,
    std::vector<PermissionListStateParcel>& reqPermList)
{
    if (!AccessTokenInfoManager::GetInstance().IsTokenIdExist(tokenID)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "TokenID=%{public}d does not exist", tokenID);
        return ERR_TOKENID_NOT_EXIST;
    }
    PermissionOper ret = GetPermissionsState(tokenID, reqPermList);
    return ret == INVALID_OPER ? RET_FAILED : RET_SUCCESS;
}

PermissionOper AccessTokenManagerService::GetPermissionsState(AccessTokenID tokenID,
    std::vector<PermissionListStateParcel>& reqPermList)
{
    int32_t apiVersion = 0;
    if (!PermissionManager::GetInstance().GetApiVersionByTokenId(tokenID, apiVersion)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Get api version error");
        return INVALID_OPER;
    }
    ACCESSTOKEN_LOG_INFO(LABEL, "TokenID: %{public}d, apiVersion: %{public}d", tokenID, apiVersion);

    bool needRes = false;
    std::vector<PermissionStateFull> permsList;
    int retUserGrant = PermissionManager::GetInstance().GetReqPermissions(tokenID, permsList, false);
    int retSysGrant = PermissionManager::GetInstance().GetReqPermissions(tokenID, permsList, true);
    if ((retSysGrant != RET_SUCCESS) || (retUserGrant != RET_SUCCESS)) {
        ACCESSTOKEN_LOG_ERROR(LABEL,
            "GetReqPermissions failed, retUserGrant:%{public}d, retSysGrant:%{public}d",
            retUserGrant, retSysGrant);
        return INVALID_OPER;
    }

    // api9 location permission handle here
    if (apiVersion >= ACCURATE_LOCATION_API_VERSION) {
        needRes = PermissionManager::GetInstance().LocationPermissionSpecialHandle(
            tokenID, reqPermList, permsList, apiVersion);
    }

    uint32_t size = reqPermList.size();
    for (uint32_t i = 0; i < size; i++) {
        // api9 location permission special handle above
        if (((reqPermList[i].permsState.permissionName == VAGUE_LOCATION_PERMISSION_NAME) ||
            (reqPermList[i].permsState.permissionName == ACCURATE_LOCATION_PERMISSION_NAME) ||
            (reqPermList[i].permsState.permissionName == BACKGROUND_LOCATION_PERMISSION_NAME)) &&
            (apiVersion >= ACCURATE_LOCATION_API_VERSION)) {
            continue;
        }

        PermissionManager::GetInstance().GetSelfPermissionState(permsList, reqPermList[i].permsState, apiVersion);
        if (static_cast<PermissionOper>(reqPermList[i].permsState.state) == DYNAMIC_OPER) {
            needRes = true;
        }
        ACCESSTOKEN_LOG_DEBUG(LABEL, "Perm: %{public}s, state: %{public}d",
            reqPermList[i].permsState.permissionName.c_str(), reqPermList[i].permsState.state);
    }
    if (GetTokenType(tokenID) == TOKEN_HAP && AccessTokenInfoManager::GetInstance().GetPermDialogCap(tokenID)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "TokenID=%{public}d is under control", tokenID);
        uint32_t size = reqPermList.size();
        for (uint32_t i = 0; i < size; i++) {
            if (reqPermList[i].permsState.state != INVALID_OPER) {
                reqPermList[i].permsState.state = FORBIDDEN_OPER;
            }
        }
        return FORBIDDEN_OPER;
    }
    if (needRes) {
        return DYNAMIC_OPER;
    }
    return PASS_OPER;
}

int AccessTokenManagerService::GetPermissionFlag(
    AccessTokenID tokenID, const std::string& permissionName, uint32_t& flag)
{
    return PermissionManager::GetInstance().GetPermissionFlag(tokenID, permissionName, flag);
}

int32_t AccessTokenManagerService::SetPermissionRequestToggleStatus(
    const std::string& permissionName, uint32_t status, int32_t userID = 0)
{
    return PermissionManager::GetInstance().SetPermissionRequestToggleStatus(permissionName, status, userID);
}

int32_t AccessTokenManagerService::GetPermissionRequestToggleStatus(
    const std::string& permissionName, uint32_t& status, int32_t userID = 0)
{
    return PermissionManager::GetInstance().GetPermissionRequestToggleStatus(permissionName, status, userID);
}

int AccessTokenManagerService::GrantPermission(AccessTokenID tokenID, const std::string& permissionName, uint32_t flag)
{
    int32_t ret = PermissionManager::GetInstance().GrantPermission(tokenID, permissionName, flag);
    if (flag == PermissionFlag::PERMISSION_USER_SET) {
        AccessTokenInfoManager::GetInstance().ClearHapPolicy();
    }
    return ret;
}

int AccessTokenManagerService::RevokePermission(AccessTokenID tokenID, const std::string& permissionName, uint32_t flag)
{
    return PermissionManager::GetInstance().RevokePermission(tokenID, permissionName, flag);
}

int AccessTokenManagerService::GrantPermissionForSpecifiedTime(
    AccessTokenID tokenID, const std::string& permissionName, uint32_t onceTime)
{
    int32_t ret = PermissionManager::GetInstance().GrantPermissionForSpecifiedTime(tokenID, permissionName, onceTime);
    return ret;
}

int AccessTokenManagerService::ClearUserGrantedPermissionState(AccessTokenID tokenID)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "TokenID: %{public}d", tokenID);
    AccessTokenInfoManager::GetInstance().ClearUserGrantedPermissionState(tokenID);
    AccessTokenInfoManager::GetInstance().SetPermDialogCap(tokenID, false);
    return RET_SUCCESS;
}

int32_t AccessTokenManagerService::RegisterPermStateChangeCallback(
    const PermStateChangeScopeParcel& scope, const sptr<IRemoteObject>& callback)
{
    return PermissionManager::GetInstance().AddPermStateChangeCallback(scope.scope, callback);
}

int32_t AccessTokenManagerService::UnRegisterPermStateChangeCallback(const sptr<IRemoteObject>& callback)
{
    return PermissionManager::GetInstance().RemovePermStateChangeCallback(callback);
}

AccessTokenIDEx AccessTokenManagerService::AllocHapToken(const HapInfoParcel& info, const HapPolicyParcel& policy)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "BundleName: %{public}s", info.hapInfoParameter.bundleName.c_str());
    AccessTokenIDEx tokenIdEx;
    tokenIdEx.tokenIDEx = 0LL;

    int ret = AccessTokenInfoManager::GetInstance().CreateHapTokenInfo(
        info.hapInfoParameter, policy.hapPolicyParameter, tokenIdEx);
    if (ret != RET_SUCCESS) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Hap token info create failed");
    }
    return tokenIdEx;
}

int32_t AccessTokenManagerService::InitHapToken(
    const HapInfoParcel& info, HapPolicyParcel& policy, AccessTokenIDEx& fullTokenId)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "Init hap %{public}s.", info.hapInfoParameter.bundleName.c_str());
    std::vector<PermissionStateFull> initializedList;
    if (info.hapInfoParameter.dlpType == DLP_COMMON) {
        if (!PermissionManager::GetInstance().InitPermissionList(info.hapInfoParameter.appDistributionType,
            policy.hapPolicyParameter, initializedList)) {
            return ERR_PERM_REQUEST_CFG_FAILED;
        }
    } else {
        if (!PermissionManager::GetInstance().InitDlpPermissionList(
            info.hapInfoParameter.bundleName, info.hapInfoParameter.userID, initializedList)) {
            return ERR_PERM_REQUEST_CFG_FAILED;
        }
    }
    policy.hapPolicyParameter.permStateList = initializedList;

    int32_t ret = AccessTokenInfoManager::GetInstance().CreateHapTokenInfo(
        info.hapInfoParameter, policy.hapPolicyParameter, fullTokenId);
    if (ret != RET_SUCCESS) {
        return ret;
    }

    return ret;
}

int AccessTokenManagerService::DeleteToken(AccessTokenID tokenID)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "TokenID: %{public}d", tokenID);
    // only support hap token deletion
    return AccessTokenInfoManager::GetInstance().RemoveHapTokenInfo(tokenID);
}

int AccessTokenManagerService::GetTokenType(AccessTokenID tokenID)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "TokenID: %{public}d", tokenID);
    return AccessTokenIDManager::GetInstance().GetTokenIdType(tokenID);
}

AccessTokenIDEx AccessTokenManagerService::GetHapTokenID(
    int32_t userID, const std::string& bundleName, int32_t instIndex)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "UserID: %{public}d, bundle: %{public}s, instIndex: %{public}d",
        userID, bundleName.c_str(), instIndex);
    return AccessTokenInfoManager::GetInstance().GetHapTokenID(userID, bundleName, instIndex);
}

AccessTokenID AccessTokenManagerService::AllocLocalTokenID(
    const std::string& remoteDeviceID, AccessTokenID remoteTokenID)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "RemoteDeviceID: %{public}s, remoteTokenID: %{public}d",
        ConstantCommon::EncryptDevId(remoteDeviceID).c_str(), remoteTokenID);
    AccessTokenID tokenID = AccessTokenInfoManager::GetInstance().AllocLocalTokenID(remoteDeviceID, remoteTokenID);
    return tokenID;
}

int32_t AccessTokenManagerService::UpdateHapToken(AccessTokenIDEx& tokenIdEx,
    const UpdateHapInfoParams& info, const HapPolicyParcel& policyParcel)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "TokenID: %{public}d", tokenIdEx.tokenIdExStruct.tokenID);
    std::vector<PermissionStateFull> InitializedList;
    if (!PermissionManager::GetInstance().InitPermissionList(
        info.appDistributionType, policyParcel.hapPolicyParameter, InitializedList)) {
        return ERR_PERM_REQUEST_CFG_FAILED;
    }
    int32_t ret = AccessTokenInfoManager::GetInstance().UpdateHapToken(tokenIdEx, info,
        InitializedList, policyParcel.hapPolicyParameter.apl, policyParcel.hapPolicyParameter.permList);
    return ret;
}

int AccessTokenManagerService::GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfoParcel& infoParcel)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "TokenID: %{public}d", tokenID);

    return AccessTokenInfoManager::GetInstance().GetHapTokenInfo(tokenID, infoParcel.hapTokenInfoParams);
}

int AccessTokenManagerService::GetHapTokenInfoExtension(AccessTokenID tokenID,
    HapTokenInfoParcel& hapTokenInfoRes, std::string& appID)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "TokenID: %{public}d.", tokenID);
    int ret = AccessTokenInfoManager::GetInstance().GetHapTokenInfo(tokenID, hapTokenInfoRes.hapTokenInfoParams);
    if (ret != RET_SUCCESS) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Get hap token info extenstion failed, ret is %{public}d.", ret);
        return ret;
    }

    return AccessTokenInfoManager::GetInstance().GetHapAppIdByTokenId(tokenID, appID);
}

int AccessTokenManagerService::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfoParcel& infoParcel)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "TokenID: %{public}d", tokenID);
    NativeTokenInfoBase baseInfo;
    int32_t ret = AccessTokenInfoManager::GetInstance().GetNativeTokenInfo(tokenID, baseInfo);
    infoParcel.nativeTokenInfoParams.apl = baseInfo.apl;
    infoParcel.nativeTokenInfoParams.processName = baseInfo.processName;
    return ret;
}

#ifndef ATM_BUILD_VARIANT_USER_ENABLE
int32_t AccessTokenManagerService::ReloadNativeTokenInfo()
{
    return NativeTokenReceptor::GetInstance().Init();
}

#endif

AccessTokenID AccessTokenManagerService::GetNativeTokenId(const std::string& processName)
{
    return AccessTokenInfoManager::GetInstance().GetNativeTokenId(processName);
}

#ifdef TOKEN_SYNC_ENABLE
int AccessTokenManagerService::GetHapTokenInfoFromRemote(AccessTokenID tokenID,
    HapTokenInfoForSyncParcel& hapSyncParcel)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "TokenID: %{public}d", tokenID);

    return AccessTokenInfoManager::GetInstance().GetHapTokenInfoFromRemote(tokenID,
        hapSyncParcel.hapTokenInfoForSyncParams);
}

int AccessTokenManagerService::SetRemoteHapTokenInfo(const std::string& deviceID,
    HapTokenInfoForSyncParcel& hapSyncParcel)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "DeviceID: %{public}s", ConstantCommon::EncryptDevId(deviceID).c_str());
    int ret = AccessTokenInfoManager::GetInstance().SetRemoteHapTokenInfo(deviceID,
        hapSyncParcel.hapTokenInfoForSyncParams);
    return ret;
}

int AccessTokenManagerService::DeleteRemoteToken(const std::string& deviceID, AccessTokenID tokenID)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "DeviceID: %{public}s, token id %{public}d",
        ConstantCommon::EncryptDevId(deviceID).c_str(), tokenID);
    return AccessTokenInfoManager::GetInstance().DeleteRemoteToken(deviceID, tokenID);
}

AccessTokenID AccessTokenManagerService::GetRemoteNativeTokenID(const std::string& deviceID,
    AccessTokenID tokenID)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "DeviceID: %{public}s, token id %{public}d",
        ConstantCommon::EncryptDevId(deviceID).c_str(), tokenID);

    return AccessTokenInfoManager::GetInstance().GetRemoteNativeTokenID(deviceID, tokenID);
}

int AccessTokenManagerService::DeleteRemoteDeviceTokens(const std::string& deviceID)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "DeviceID: %{public}s", ConstantCommon::EncryptDevId(deviceID).c_str());
    return AccessTokenInfoManager::GetInstance().DeleteRemoteDeviceTokens(deviceID);
}

int32_t AccessTokenManagerService::RegisterTokenSyncCallback(const sptr<IRemoteObject>& callback)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "Call token sync callback registed.");
    return TokenModifyNotifier::GetInstance().RegisterTokenSyncCallback(callback);
}

int32_t AccessTokenManagerService::UnRegisterTokenSyncCallback()
{
    ACCESSTOKEN_LOG_INFO(LABEL, "Call token sync callback unregisted.");
    return TokenModifyNotifier::GetInstance().UnRegisterTokenSyncCallback();
}
#endif

void AccessTokenManagerService::DumpTokenInfo(const AtmToolsParamInfoParcel& infoParcel, std::string& dumpInfo)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "Called");

    AccessTokenInfoManager::GetInstance().DumpTokenInfo(infoParcel.info, dumpInfo);
}

int32_t AccessTokenManagerService::GetVersion(uint32_t& version)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "Called");
    version = DEFAULT_TOKEN_VERSION;
    return RET_SUCCESS;
}

int32_t AccessTokenManagerService::SetPermDialogCap(const HapBaseInfoParcel& hapBaseInfoParcel, bool enable)
{
    AccessTokenIDEx tokenIdEx = AccessTokenInfoManager::GetInstance().GetHapTokenID(
        hapBaseInfoParcel.hapBaseInfo.userID,
        hapBaseInfoParcel.hapBaseInfo.bundleName,
        hapBaseInfoParcel.hapBaseInfo.instIndex);
    int32_t ret = AccessTokenInfoManager::GetInstance().SetPermDialogCap(tokenIdEx.tokenIdExStruct.tokenID, enable);
    // DFX
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::ACCESS_TOKEN, "SET_PERMISSION_DIALOG_CAP",
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "TOKENID", tokenIdEx.tokenIdExStruct.tokenID,
        "USERID", hapBaseInfoParcel.hapBaseInfo.userID, "BUNDLENAME", hapBaseInfoParcel.hapBaseInfo.bundleName,
        "INSTINDEX", hapBaseInfoParcel.hapBaseInfo.instIndex, "ENABLE", enable);
    return ret;
}

void AccessTokenManagerService::GetPermissionManagerInfo(PermissionGrantInfoParcel& infoParcel)
{
    infoParcel.info.grantBundleName = grantBundleName_;
    infoParcel.info.grantAbilityName = grantAbilityName_;
    infoParcel.info.grantServiceAbilityName = grantServiceAbilityName_;
    infoParcel.info.permStateAbilityName = permStateAbilityName_;
    infoParcel.info.globalSwitchAbilityName = globalSwitchAbilityName_;
}

int32_t AccessTokenManagerService::InitUserPolicy(
    const std::vector<UserState>& userList, const std::vector<std::string>& permList)
{
    return AccessTokenInfoManager::GetInstance().InitUserPolicy(userList, permList);
}

int32_t AccessTokenManagerService::UpdateUserPolicy(const std::vector<UserState>& userList)
{
    return AccessTokenInfoManager::GetInstance().UpdateUserPolicy(userList);
}

int32_t AccessTokenManagerService::ClearUserPolicy()
{
    return AccessTokenInfoManager::GetInstance().ClearUserPolicy();
}

int AccessTokenManagerService::Dump(int fd, const std::vector<std::u16string>& args)
{
    if (fd < 0) {
        return ERR_INVALID_VALUE;
    }

    dprintf(fd, "AccessToken Dump:\n");
    std::string arg0 = ((args.size() == 0)? "" : Str16ToStr8(args.at(0)));
    if (arg0.compare("-h") == 0) {
        dprintf(fd, "Usage:\n");
        dprintf(fd, "       -h: command help\n");
        dprintf(fd, "       -a: dump all tokens\n");
        dprintf(fd, "       -t <TOKEN_ID>: dump special token id\n");
    } else if (arg0.compare("-t") == 0) {
        if (args.size() < TWO_ARGS) {
            return ERR_INVALID_VALUE;
        }
        long long tokenID = atoll(static_cast<const char *>(Str16ToStr8(args.at(1)).c_str()));
        if (tokenID <= 0) {
            return ERR_INVALID_VALUE;
        }
        AtmToolsParamInfoParcel infoParcel;
        infoParcel.info.tokenId = static_cast<AccessTokenID>(tokenID);
        std::string dumpStr;
        DumpTokenInfo(infoParcel, dumpStr);
        dprintf(fd, "%s\n", dumpStr.c_str());
    }  else if (arg0.compare("-a") == 0 || arg0 == "") {
        std::string dumpStr;
        AtmToolsParamInfoParcel infoParcel;
        DumpTokenInfo(infoParcel, dumpStr);
        dprintf(fd, "%s\n", dumpStr.c_str());
    }
    return ERR_OK;
}

void AccessTokenManagerService::AccessTokenServiceParamSet() const
{
    int32_t res = SetParameter(ACCESS_TOKEN_SERVICE_INIT_KEY, std::to_string(1).c_str());
    if (res != 0) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "SetParameter ACCESS_TOKEN_SERVICE_INIT_KEY 1 failed %{public}d", res);
        return;
    }
    // 2 is to tell others sa that at service is loaded.
    res = SetParameter(ACCESS_TOKEN_SERVICE_INIT_KEY, std::to_string(2).c_str());
    if (res != 0) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "SetParameter ACCESS_TOKEN_SERVICE_INIT_KEY 2 failed %{public}d", res);
        return;
    }
}

void AccessTokenManagerService::GetConfigValue()
{
    LibraryLoader loader(CONFIG_POLICY_LIBPATH);
    ConfigPolicyLoaderInterface* policy = loader.GetObject<ConfigPolicyLoaderInterface>();
    if (policy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Dlopen libaccesstoken_config_policy failed.");
        return;
    }
    AccessTokenConfigValue value;
    if (policy->GetConfigValue(ServiceType::ACCESSTOKEN_SERVICE, value)) {
        // set value from config
        grantBundleName_ = value.atConfig.grantBundleName.empty() ?
            GRANT_ABILITY_BUNDLE_NAME : value.atConfig.grantBundleName;
        grantAbilityName_ = value.atConfig.grantAbilityName.empty() ?
            GRANT_ABILITY_ABILITY_NAME : value.atConfig.grantAbilityName;
        grantServiceAbilityName_ = value.atConfig.grantServiceAbilityName.empty() ?
            GRANT_ABILITY_ABILITY_NAME : value.atConfig.grantServiceAbilityName;
        permStateAbilityName_ = value.atConfig.permStateAbilityName.empty() ?
            PERMISSION_STATE_SHEET_ABILITY_NAME : value.atConfig.permStateAbilityName;
        globalSwitchAbilityName_ = value.atConfig.globalSwitchAbilityName.empty() ?
            GLOBAL_SWITCH_SHEET_ABILITY_NAME : value.atConfig.globalSwitchAbilityName;
    } else {
        ACCESSTOKEN_LOG_INFO(LABEL, "No config file or config file is not valid, use default values");
        grantBundleName_ = GRANT_ABILITY_BUNDLE_NAME;
        grantAbilityName_ = GRANT_ABILITY_ABILITY_NAME;
        grantServiceAbilityName_ = GRANT_ABILITY_ABILITY_NAME;
        permStateAbilityName_ = PERMISSION_STATE_SHEET_ABILITY_NAME;
        globalSwitchAbilityName_ = GLOBAL_SWITCH_SHEET_ABILITY_NAME;
    }

    ACCESSTOKEN_LOG_INFO(LABEL, "GrantBundleName_ is %{public}s, grantAbilityName_ is %{public}s, \
        permStateAbilityName_ is %{public}s, permStateAbilityName_ is %{public}s",
        grantBundleName_.c_str(), grantAbilityName_.c_str(),
        permStateAbilityName_.c_str(), permStateAbilityName_.c_str());
}

bool AccessTokenManagerService::Initialize()
{
    ReportSysEventPerformance();
    AccessTokenInfoManager::GetInstance().Init();
    AccessTokenInfoManager::GetInstance().ClearHapPolicy();
    NativeTokenReceptor::GetInstance().Init();

#ifdef EVENTHANDLER_ENABLE
    TempPermissionObserver::GetInstance().InitEventHandler();
    ShortGrantManager::GetInstance().InitEventHandler();
#endif

#ifdef SUPPORT_SANDBOX_APP
    DlpPermissionSetParser::GetInstance().Init();
#endif
    PermissionDefinitionParser::GetInstance().Init();
    GetConfigValue();
    TempPermissionObserver::GetInstance().GetConfigValue();
    ACCESSTOKEN_LOG_INFO(LABEL, "Initialize success");
    return true;
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS

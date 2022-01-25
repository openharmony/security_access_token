/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "accesstoken_manager_client.h"

#include "accesstoken_log.h"
#include "accesstoken_manager_proxy.h"
#include "hap_token_info.h"
#include "iservice_registry.h"
#include "native_token_info.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_ACCESSTOKEN, "AccessTokenManagerClient"
};
} // namespace

AccessTokenManagerClient& AccessTokenManagerClient::GetInstance()
{
    static AccessTokenManagerClient instance;
    return instance;
}

AccessTokenManagerClient::AccessTokenManagerClient()
{}

AccessTokenManagerClient::~AccessTokenManagerClient()
{}

int AccessTokenManagerClient::VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return PERMISSION_DENIED;
    }
    return proxy->VerifyAccessToken(tokenID, permissionName);
}

int AccessTokenManagerClient::GetDefPermission(
    const std::string& permissionName, PermissionDef& permissionDefResult)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    PermissionDefParcel permissionDefParcel;
    int result = proxy->GetDefPermission(permissionName, permissionDefParcel);
    permissionDefResult = permissionDefParcel.permissionDef;
    return result;
}

int AccessTokenManagerClient::GetDefPermissions(AccessTokenID tokenID, std::vector<PermissionDef>& permList)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    std::vector<PermissionDefParcel> parcelList;
    int result = proxy->GetDefPermissions(tokenID, parcelList);
    for (auto permParcel : parcelList) {
        PermissionDef perm = permParcel.permissionDef;
        permList.emplace_back(perm);
    }
    return result;
}

int AccessTokenManagerClient::GetReqPermissions(
    AccessTokenID tokenID, std::vector<PermissionStateFull>& reqPermList, bool isSystemGrant)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    std::vector<PermissionStateFullParcel> parcelList;
    int result = proxy->GetReqPermissions(tokenID, parcelList, isSystemGrant);
    for (auto permParcel : parcelList) {
        PermissionStateFull perm = permParcel.permStatFull;
        reqPermList.emplace_back(perm);
    }
    return result;
}

int AccessTokenManagerClient::GetPermissionFlag(AccessTokenID tokenID, const std::string& permissionName)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return DEFAULT_PERMISSION_FLAGS;
    }
    return proxy->GetPermissionFlag(tokenID, permissionName);
}

int AccessTokenManagerClient::GrantPermission(AccessTokenID tokenID, const std::string& permissionName, int flag)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->GrantPermission(tokenID, permissionName, flag);
}

int AccessTokenManagerClient::RevokePermission(AccessTokenID tokenID, const std::string& permissionName, int flag)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->RevokePermission(tokenID, permissionName, flag);
}

int AccessTokenManagerClient::ClearUserGrantedPermissionState(AccessTokenID tokenID)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->ClearUserGrantedPermissionState(tokenID);
}

AccessTokenIDEx AccessTokenManagerClient::AllocHapToken(const HapInfoParams& info, const HapPolicyParams& policy)
{
    AccessTokenIDEx res = { 0 };
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return res;
    }
    HapInfoParcel hapInfoParcel;
    HapPolicyParcel  hapPolicyParcel;
    hapInfoParcel.hapInfoParameter = info;
    hapPolicyParcel.hapPolicyParameter = policy;

    return proxy->AllocHapToken(hapInfoParcel, hapPolicyParcel);
}

int AccessTokenManagerClient::DeleteToken(AccessTokenID tokenID)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->DeleteToken(tokenID);
}

ATokenTypeEnum AccessTokenManagerClient::GetTokenType(AccessTokenID tokenID)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return TOKEN_INVALID;
    }
    return (ATokenTypeEnum)(proxy->GetTokenType(tokenID));
}

int AccessTokenManagerClient::CheckNativeDCap(AccessTokenID tokenID, const std::string& dcap)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->CheckNativeDCap(tokenID, dcap);
}

AccessTokenID AccessTokenManagerClient::GetHapTokenID(int userID, const std::string& bundleName, int instIndex)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->GetHapTokenID(userID, bundleName, instIndex);
}

AccessTokenID AccessTokenManagerClient::AllocLocalTokenID(
    const std::string& remoteDeviceID, AccessTokenID remoteTokenID)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->AllocLocalTokenID(remoteDeviceID, remoteTokenID);
}

int AccessTokenManagerClient::UpdateHapToken(
    AccessTokenID tokenID, const std::string& appIDDesc, const HapPolicyParams& policy)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    HapPolicyParcel hapPolicyParcel;
    hapPolicyParcel.hapPolicyParameter = policy;
    return proxy->UpdateHapToken(tokenID, appIDDesc, hapPolicyParcel);
}

int AccessTokenManagerClient::GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo& hapTokenInfoRes)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    HapTokenInfoParcel hapTokenInfoParcel;
    int res = proxy->GetHapTokenInfo(tokenID, hapTokenInfoParcel);

    hapTokenInfoRes = hapTokenInfoParcel.hapTokenInfoParams;
    return res;
}

int AccessTokenManagerClient::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo& nativeTokenInfoRes)
{
    ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    NativeTokenInfoParcel nativeTokenInfoParcel;
    int res = proxy->GetNativeTokenInfo(tokenID, nativeTokenInfoParcel);
    nativeTokenInfoRes = nativeTokenInfoParcel.nativeTokenInfoParams;
    return res;
}

sptr<IAccessTokenManager> AccessTokenManagerClient::GetProxy()
{
    if (proxy_ == nullptr) {
        std::lock_guard<std::mutex> lock(proxyMutex_);
        if (proxy_ == nullptr) {
            auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (sam == nullptr) {
                ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: GetSystemAbilityManager is null", __func__);
                return nullptr;
            }
            auto accesstokenSa = sam->GetSystemAbility(IAccessTokenManager::SA_ID_ACCESSTOKEN_MANAGER_SERVICE);
            if (accesstokenSa == nullptr) {
                ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: GetSystemAbility %{public}d is null", __func__,
                    IAccessTokenManager::SA_ID_ACCESSTOKEN_MANAGER_SERVICE);
                return nullptr;
            }

            auto proxy = iface_cast<IAccessTokenManager>(accesstokenSa);
            if (proxy == nullptr) {
                ACCESSTOKEN_LOG_DEBUG(LABEL, "%{public}s: iface_cast get null", __func__);
                return nullptr;
            }
            proxy_ = proxy;
        }
    }

    return proxy_;
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
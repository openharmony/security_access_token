/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "camera_manager_privacy_client.h"

#include "accesstoken_log.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PRIVACY, "CameraManagerPrivacyClient"
};
std::recursive_mutex g_instanceMutex;
} // namespace

CameraManagerPrivacyClient& CameraManagerPrivacyClient::GetInstance()
{
    static CameraManagerPrivacyClient* instance = nullptr;
    if (instance == nullptr) {
        std::lock_guard<std::recursive_mutex> lock(g_instanceMutex);
        if (instance == nullptr) {
            instance = new CameraManagerPrivacyClient();
        }
    }
    return *instance;
}

CameraManagerPrivacyClient::CameraManagerPrivacyClient()
{}

CameraManagerPrivacyClient::~CameraManagerPrivacyClient()
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    ReleaseProxy();
}

int32_t CameraManagerPrivacyClient::MuteCameraPersist(PolicyType policyType, bool muteMode)
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Proxy is null");
        return -1;
    }
    return proxy->MuteCameraPersist(policyType, muteMode);
}

bool CameraManagerPrivacyClient::IsCameraMuted()
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Proxy is null");
        return false;
    }
    bool muteMode = false;
    proxy->IsCameraMuted(muteMode);
    return muteMode;
}

void CameraManagerPrivacyClient::InitProxy()
{
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "GetSystemAbilityManager is null");
        return;
    }
    auto cameraManagerSa = sam->GetSystemAbility(CAMERA_SERVICE_ID);
    if (cameraManagerSa == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "GetSystemAbility %{public}d is null",
            CAMERA_SERVICE_ID);
        return;
    }

    serviceDeathObserver_ = sptr<CameraManagerPrivacyDeathRecipient>::MakeSptr();
    if (serviceDeathObserver_ != nullptr) {
        cameraManagerSa->AddDeathRecipient(serviceDeathObserver_);
    }

    proxy_ = new CameraManagerPrivacyProxy(cameraManagerSa);
    if (proxy_ == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Iface_cast get null");
    }
}

void CameraManagerPrivacyClient::OnRemoteDiedHandle()
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    ReleaseProxy();
}

sptr<ICameraService> CameraManagerPrivacyClient::GetProxy()
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    if (proxy_ == nullptr) {
        InitProxy();
    }
    return proxy_;
}

void CameraManagerPrivacyClient::ReleaseProxy()
{
    if (proxy_ != nullptr && serviceDeathObserver_ != nullptr) {
        proxy_->AsObject()->RemoveDeathRecipient(serviceDeathObserver_);
    }
    proxy_ = nullptr;
    serviceDeathObserver_ = nullptr;
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS


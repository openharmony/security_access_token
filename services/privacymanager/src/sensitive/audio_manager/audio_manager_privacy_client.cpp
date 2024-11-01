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
#include "audio_manager_privacy_client.h"
#include <unistd.h>

#include "accesstoken_log.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PRIVACY, "AudioManagerPrivacyClient"
};
std::recursive_mutex g_instanceMutex;
} // namespace

AudioManagerPrivacyClient& AudioManagerPrivacyClient::GetInstance()
{
    static AudioManagerPrivacyClient* instance = nullptr;
    if (instance == nullptr) {
        std::lock_guard<std::recursive_mutex> lock(g_instanceMutex);
        if (instance == nullptr) {
            instance = new AudioManagerPrivacyClient();
        }
    }
    return *instance;
}

AudioManagerPrivacyClient::AudioManagerPrivacyClient()
{}

AudioManagerPrivacyClient::~AudioManagerPrivacyClient()
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    ReleaseProxy();
}

int32_t AudioManagerPrivacyClient::SetMicrophoneMutePersistent(const bool isMute, const PolicyType type)
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Proxy is null");
        return -1;
    }
    return proxy->SetMicrophoneMutePersistent(isMute, type);
}

bool AudioManagerPrivacyClient::GetPersistentMicMuteState()
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Proxy is null");
        return false;
    }
    return proxy->GetPersistentMicMuteState();
}

void AudioManagerPrivacyClient::InitProxy()
{
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "GetSystemAbilityManager is null");
        return;
    }
    auto audioManagerSa = sam->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);
    if (audioManagerSa == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "GetSystemAbility %{public}d is null",
            AUDIO_POLICY_SERVICE_ID);
        return;
    }

    serviceDeathObserver_ = sptr<AudioMgrDeathRecipient>::MakeSptr();
    if (serviceDeathObserver_ != nullptr) {
        audioManagerSa->AddDeathRecipient(serviceDeathObserver_);
    }

    proxy_ = new AudioManagerPrivacyProxy(audioManagerSa);
    if (proxy_ == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Iface_cast get null");
    }
}

void AudioManagerPrivacyClient::OnRemoteDiedHandle()
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    ReleaseProxy();
}

sptr<IAudioPolicy> AudioManagerPrivacyClient::GetProxy()
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    if (proxy_ == nullptr) {
        InitProxy();
    }
    return proxy_;
}

void AudioManagerPrivacyClient::ReleaseProxy()
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


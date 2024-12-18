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
#include "app_manager_access_client.h"
#include <unistd.h>

#include "accesstoken_log.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_ACCESSTOKEN, "AppManagerAccessClient"
};
std::recursive_mutex g_instanceMutex;
} // namespace

AppManagerAccessClient& AppManagerAccessClient::GetInstance()
{
    static AppManagerAccessClient* instance = nullptr;
    if (instance == nullptr) {
        std::lock_guard<std::recursive_mutex> lock(g_instanceMutex);
        if (instance == nullptr) {
            AppManagerAccessClient* tmp = new AppManagerAccessClient();
            instance = std::move(tmp);
        }
    }
    return *instance;
}

AppManagerAccessClient::AppManagerAccessClient()
{}

AppManagerAccessClient::~AppManagerAccessClient()
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    ReleaseProxy();
}

int32_t AppManagerAccessClient::KillProcessesByAccessTokenId(const uint32_t accessTokenId)
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Proxy is null.");
        return -1;
    }
    sptr<IAmsMgr> amsService = proxy->GetAmsMgr();
    if (amsService == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "AmsService is null.");
    }
    return amsService->KillProcessesByAccessTokenId(accessTokenId);
}

int32_t AppManagerAccessClient::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver>& observer)
{
    ACCESSTOKEN_LOG_INFO(LABEL, "Entry");
    if (observer == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Callback is nullptr.");
        return -1;
    }
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Proxy is null.");
        return -1;
    }
    std::vector<std::string> bundleNameList;
    return proxy->RegisterApplicationStateObserver(observer, bundleNameList);
}

int32_t AppManagerAccessClient::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    if (observer == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Callback is nullptr.");
        return -1;
    }
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Proxy is null");
        return -1;
    }
    return proxy->UnregisterApplicationStateObserver(observer);
}

int32_t AppManagerAccessClient::GetForegroundApplications(std::vector<AppStateData>& list)
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Proxy is null");
        return -1;
    }
    return proxy->GetForegroundApplications(list);
}

void AppManagerAccessClient::InitProxy()
{
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "GetSystemAbilityManager is null");
        return;
    }
    auto appManagerSa = sam->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (appManagerSa == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "GetSystemAbility %{public}d is null",
            APP_MGR_SERVICE_ID);
        return;
    }

    serviceDeathObserver_ = sptr<AppMgrDeathRecipient>::MakeSptr();
    if (serviceDeathObserver_ != nullptr) {
        appManagerSa->AddDeathRecipient(serviceDeathObserver_);
    }

    proxy_ = new AppManagerAccessProxy(appManagerSa);
    if (proxy_ == nullptr || proxy_->AsObject() == nullptr || proxy_->AsObject()->IsObjectDead()) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "Iface_cast get null");
    }
}

void AppManagerAccessClient::RegisterDeathCallback(const std::shared_ptr<AppManagerDeathCallback>& callback)
{
    std::lock_guard<std::mutex> lock(deathCallbackMutex_);
    if (callback == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "AppManagerAccessClient: Callback is nullptr.");
        return;
    }
    appManagerDeathCallbackList_.emplace_back(callback);
}

void AppManagerAccessClient::OnRemoteDiedHandle()
{
    std::vector<std::shared_ptr<AppManagerDeathCallback>> tmpCallbackList;
    {
        std::lock_guard<std::mutex> lock(deathCallbackMutex_);
        tmpCallbackList.assign(appManagerDeathCallbackList_.begin(), appManagerDeathCallbackList_.end());
    }

    for (size_t i = 0; i < tmpCallbackList.size(); i++) {
        tmpCallbackList[i]->NotifyAppManagerDeath();
    }
    {
        std::lock_guard<std::mutex> lock(proxyMutex_);
        ReleaseProxy();
    }
}

sptr<IAppMgr> AppManagerAccessClient::GetProxy()
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    if (proxy_ == nullptr || proxy_->AsObject() == nullptr || proxy_->AsObject()->IsObjectDead()) {
        InitProxy();
    }
    return proxy_;
}

void AppManagerAccessClient::ReleaseProxy()
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


/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef ACCESS_APP_MANAGER_ACCESS_CLIENT_H
#define ACCESS_APP_MANAGER_ACCESS_CLIENT_H

#include <mutex>
#include <string>

#include "app_status_change_callback.h"
#include "app_manager_death_callback.h"
#include "app_manager_death_recipient.h"
#include "app_manager_access_proxy.h"
#include "nocopyable.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
class AppManagerAccessClient final {
public:
    static AppManagerAccessClient& GetInstance();
    virtual ~AppManagerAccessClient();

    int32_t KillProcessesByAccessTokenId(const uint32_t accessTokenId);
    int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver>& observer);
    int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver>& observer);
    int32_t GetForegroundApplications(std::vector<AppStateData>& list);
    void RegisterDeathCallback(const std::shared_ptr<AppManagerDeathCallback>& callback);
    void OnRemoteDiedHandle();

private:
    AppManagerAccessClient();
    DISALLOW_COPY_AND_MOVE(AppManagerAccessClient);

    void InitProxy();
    sptr<IAppMgr> GetProxy();
    void ReleaseProxy();

    sptr<AppMgrDeathRecipient> serviceDeathObserver_ = nullptr;
    std::mutex proxyMutex_;
    std::mutex deathCallbackMutex_;
    sptr<IAppMgr> proxy_ = nullptr;
    std::vector<std::shared_ptr<AppManagerDeathCallback>> appManagerDeathCallbackList_;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // ACCESS_APP_MANAGER_ACCESS_CLIENT_H


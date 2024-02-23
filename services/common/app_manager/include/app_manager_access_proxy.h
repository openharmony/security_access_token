/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef ACCESS_APP_MANAGER_ACCESS_PROXY_H
#define ACCESS_APP_MANAGER_ACCESS_PROXY_H

#include <iremote_proxy.h>

#include "app_state_data.h"
#include "process_data.h"
#include "service_ipc_interface_code.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
class IApplicationStateObserver : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.IApplicationStateObserver");

    virtual void OnForegroundApplicationChanged(const AppStateData &appStateData) = 0;
    virtual void OnProcessDied(const ProcessData &processData) = 0;
    virtual void OnApplicationStateChanged(const AppStateData &appStateData) = 0;
    enum class Message {
        TRANSACT_ON_FOREGROUND_APPLICATION_CHANGED = 0,
        TRANSACT_ON_PROCESS_DIED = 5,
        TRANSACT_ON_APPLICATION_STATE_CHANGED = 6,
    };
};

class IAppMgr : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.AppMgr");

    virtual int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver>& observer,
        const std::vector<std::string>& bundleNameList = {}) = 0;
    virtual int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver>& observer) = 0;
    virtual int32_t GetForegroundApplications(std::vector<AppStateData>& list) = 0;

    enum class Message {
        REGISTER_APPLICATION_STATE_OBSERVER = 12,
        UNREGISTER_APPLICATION_STATE_OBSERVER = 13,
        GET_FOREGROUND_APPLICATIONS = 14,
    };
};

class AppManagerAccessProxy : public IRemoteProxy<IAppMgr> {
public:
    explicit AppManagerAccessProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IAppMgr>(impl) {};

    virtual ~AppManagerAccessProxy() = default;

    int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver>& observer,
        const std::vector<std::string> &bundleNameList = {}) override;
    int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver>& observer) override;
    int32_t GetForegroundApplications(std::vector<AppStateData>& list) override;
private:
    static inline BrokerDelegator<AppManagerAccessProxy> delegator_;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // ACCESS_APP_MANAGER_ACCESS_PROXY_H

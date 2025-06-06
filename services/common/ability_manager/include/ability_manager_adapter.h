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

#ifndef ACCESS_TOKEN_ABILITY_MANAGER_ADAPTER_H
#define ACCESS_TOKEN_ABILITY_MANAGER_ADAPTER_H

#include <mutex>
#include "ability_manager_access_loader.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
/**
 * @class AbilityManagerAdapter
 * AbilityManagerAdapter is used to access ability manager services.
 */
class AbilityManagerAdapter {
private:
    AbilityManagerAdapter();
    virtual ~AbilityManagerAdapter();
    DISALLOW_COPY_AND_MOVE(AbilityManagerAdapter);

public:
    static AbilityManagerAdapter& GetInstance();

    int32_t StartAbility(const InnerWant &innerWant, const sptr<IRemoteObject> &callerToken);
    int32_t KillProcessForPermissionUpdate(uint32_t accessTokenId);

    enum class Message {
        START_ABILITY = 1001,
        KILL_PROCESS_FOR_PERMISSION_UPDATE = 5300,
    };

private:
    void InitProxy();

    class AbilityMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        AbilityMgrDeathRecipient() = default;
        ~AbilityMgrDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    private:
        DISALLOW_COPY_AND_MOVE(AbilityMgrDeathRecipient);
    };

    sptr<IRemoteObject> GetProxy();
    void ReleaseProxy(const wptr<IRemoteObject>& remote);

    std::mutex proxyMutex_;
    sptr<IRemoteObject> proxy_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif  // ACCESS_TOKEN_ABILITY_MANAGER_ADAPTER_H

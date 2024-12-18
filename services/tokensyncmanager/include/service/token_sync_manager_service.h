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

#ifndef TOKEN_SYNC_MANAGER_SERVICE_H
#define TOKEN_SYNC_MANAGER_SERVICE_H

#include <string>

#ifdef EVENTHANDLER_ENABLE
#include "event_handler.h"
#include "access_event_handler.h"
#endif
#include "hap_token_info_for_sync_parcel.h"
#include "iremote_object.h"
#include "nocopyable.h"
#include "singleton.h"
#include "system_ability.h"
#include "token_sync_manager_stub.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };
class TokenSyncManagerService final : public SystemAbility, public TokenSyncManagerStub {
    DECLARE_DELAYED_SINGLETON(TokenSyncManagerService);
    DECLEAR_SYSTEM_ABILITY(TokenSyncManagerService);

public:
    void OnStart() override;
    void OnStop() override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

#ifdef EVENTHANDLER_ENABLE
    std::shared_ptr<AccessEventHandler> GetSendEventHandler() const;
    std::shared_ptr<AccessEventHandler> GetRecvEventHandler() const;
#endif
    int GetRemoteHapTokenInfo(const std::string& deviceID, AccessTokenID tokenID) override;
    int DeleteRemoteHapTokenInfo(AccessTokenID tokenID) override;
    int UpdateRemoteHapTokenInfo(const HapTokenInfoForSync& tokenInfo) override;

private:
    bool Initialize();

#ifdef EVENTHANDLER_ENABLE
    std::shared_ptr<AppExecFwk::EventRunner> sendRunner_;
    std::shared_ptr<AppExecFwk::EventRunner> recvRunner_;
    std::shared_ptr<AccessEventHandler> sendHandler_;
    std::shared_ptr<AccessEventHandler> recvHandler_;
#endif
    ServiceRunningState state_;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // TOKEN_SYNC_MANAGER_SERVICE_H

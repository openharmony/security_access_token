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

#ifndef ACTIVE_STATUS_CHANGE_CALLBACK_MANAGER_H
#define ACTIVE_STATUS_CHANGE_CALLBACK_MANAGER_H

#include <mutex>
#include <vector>

#ifdef EVENTHANDLER_ENABLE
#include "access_event_handler.h"
#endif
#include "access_token.h"
#include "accesstoken_common_log.h"
#include "active_change_response_info.h"
#include "perm_active_status_callback_death_recipient.h"
#include "perm_active_status_change_callback_proxy.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
struct CallbackData {
    CallbackData() : permList_(), callbackObject_(nullptr)
    {}
    CallbackData(const std::vector<std::string>& permList, sptr<IRemoteObject> callback)
        : permList_(permList), callbackObject_(callback)
    {}

    AccessTokenID registerTokenId {0};
    std::vector<std::string> permList_;
    sptr<IRemoteObject> callbackObject_;
};

class ActiveStatusCallbackManager {
public:
    virtual ~ActiveStatusCallbackManager();
    ActiveStatusCallbackManager();
    static ActiveStatusCallbackManager& GetInstance();

    int32_t AddCallback(
        AccessTokenID regiterTokenId, const std::vector<std::string>& permList, const sptr<IRemoteObject>& callback);
    int32_t RemoveCallback(const sptr<IRemoteObject>& callback);
    bool NeedCalled(const std::vector<std::string>& permList, const std::string& permName);
    void ExecuteCallbackAsync(ActiveChangeResponse& info);
#ifdef EVENTHANDLER_ENABLE
    void InitEventHandler(const std::shared_ptr<AccessEventHandler>& eventHandler);
#endif
    void ActiveStatusChange(ActiveChangeResponse& info);
private:
    std::mutex mutex_;
    std::vector<CallbackData> callbackDataList_;
    sptr<IRemoteObject::DeathRecipient> callbackDeathRecipient_;
#ifdef EVENTHANDLER_ENABLE
    std::shared_ptr<AccessEventHandler> eventHandler_;
#endif
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // ACTIVE_STATUS_CHANGE_CALLBACK_MANAGER_H

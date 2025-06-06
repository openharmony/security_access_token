/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "callback_death_recipients.h"

#include "access_token.h"
#include "callback_manager.h"
#ifdef TOKEN_SYNC_ENABLE
#include "token_modify_notifier.h"
#endif // TOKEN_SYNC_ENABLE

namespace OHOS {
namespace Security {
namespace AccessToken {

void PermStateCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    LOGI(ATM_DOMAIN, ATM_TAG, "Enter");
    if (remote == nullptr) {
        LOGE(ATM_DOMAIN, ATM_TAG, "Remote object is nullptr");
        return;
    }

    sptr<IRemoteObject> object = remote.promote();
    if (object == nullptr) {
        LOGE(ATM_DOMAIN, ATM_TAG, "Object is nullptr");
        return;
    }
    CallbackManager::GetInstance().RemoveCallback(object);
    LOGI(ATM_DOMAIN, ATM_TAG, "End");
}

#ifdef TOKEN_SYNC_ENABLE
void TokenSyncCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    LOGI(ATM_DOMAIN, ATM_TAG, "Call OnRemoteDied.");
    if (remote == nullptr) {
        LOGE(ATM_DOMAIN, ATM_TAG, "Remote object is nullptr.");
        return;
    }
    TokenModifyNotifier::GetInstance().UnRegisterTokenSyncCallback();
    LOGI(ATM_DOMAIN, ATM_TAG, "Call UnRegisterTokenSyncCallback end.");
}
#endif // TOKEN_SYNC_ENABLE
} // namespace AccessToken
} // namespace Security
} // namespace OHOS

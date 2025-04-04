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

#include "access_event_handler.h"

#include "accesstoken_common_log.h"

namespace OHOS {
namespace Security {
namespace AccessToken {

AccessEventHandler::AccessEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner>& runner)
    : AppExecFwk::EventHandler(runner)
{
    LOGI(ATM_DOMAIN, ATM_TAG, "Enter");
}
AccessEventHandler::~AccessEventHandler() = default;

bool AccessEventHandler::ProxyPostTask(const Callback &callback, int64_t delayTime)
{
    LOGD(ATM_DOMAIN, ATM_TAG, "PostTask without name");
    return AppExecFwk::EventHandler::PostTask(callback, delayTime);
}

bool AccessEventHandler::ProxyPostTask(
    const Callback &callback, const std::string &name, int64_t delayTime)
{
    LOGD(ATM_DOMAIN, ATM_TAG, "PostTask with name");
    return AppExecFwk::EventHandler::PostTask(callback, name, delayTime);
}

void AccessEventHandler::ProxyRemoveTask(const std::string &name)
{
    AppExecFwk::EventHandler::RemoveTask(name);
}
}  // namespace AccessToken
}  // namespace Security
}  // namespace OHOS
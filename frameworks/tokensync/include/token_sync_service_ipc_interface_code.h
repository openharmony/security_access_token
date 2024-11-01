/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef TOKEN_SYNC_SERVICE_IPC_INTERFACE_CODE_H
#define TOKEN_SYNC_SERVICE_IPC_INTERFACE_CODE_H

namespace OHOS {
namespace Security {
namespace AccessToken {
/* SAID:3504 */
enum class TokenSyncInterfaceCode {
    GET_REMOTE_HAP_TOKEN_INFO = 0x0000,
    DELETE_REMOTE_HAP_TOKEN_INFO,
    UPDATE_REMOTE_HAP_TOKEN_INFO,
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS

#endif // TOKEN_SYNC_SERVICE_IPC_INTERFACE_CODE_H

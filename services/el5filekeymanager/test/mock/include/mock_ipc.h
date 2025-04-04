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

#ifndef EL5_FILEKEY_MANAGER_SERVICE_MOCK_IPC_H
#define EL5_FILEKEY_MANAGER_SERVICE_MOCK_IPC_H

#include <cstdint>

namespace OHOS {
namespace Security {
namespace AccessToken {
class MockIpc {
public:
    MockIpc();
    ~MockIpc();
    static void SetCallingUid(uint32_t uid);
    static void SetCallingTokenID(uint32_t tokenId);
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // EL5_FILEKEY_MANAGER_SERVICE_MOCK_IPC_H

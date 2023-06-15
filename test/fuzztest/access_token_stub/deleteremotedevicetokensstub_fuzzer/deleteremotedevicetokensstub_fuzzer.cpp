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

#include "deleteremotedevicetokensstub_fuzzer.h"

#include <string>
#include <thread>
#include <vector>
#undef private
#include "accesstoken_manager_service.h"
#include "accesstoken_service_ipc_interface_code.h"

using namespace std;
using namespace OHOS::Security::AccessToken;

namespace OHOS {
    bool DeleteRemoteDeviceTokensStubFuzzTest(const uint8_t* data, size_t size)
    {
    #ifdef TOKEN_SYNC_ENABLE
        if ((data == nullptr) || (size == 0)) {
            return false;
        }

        std::string testName(reinterpret_cast<const char*>(data), size);

        MessageParcel datas;
        datas.WriteInterfaceToken(IAccessTokenManager::GetDescriptor());
        if (!datas.WriteString(testName)) {
            return false;
        }

        uint32_t code = static_cast<uint32_t>(
            IAccessTokenManager::AccessTokenInterfaceCode::DELETE_REMOTE_DEVICE_TOKEN);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<AccessTokenManagerService>::GetInstance()->OnRemoteRequest(code, datas, reply, option);

        return true;
    #else
        return true;
    #endif
    }
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DeleteRemoteDeviceTokensStubFuzzTest(data, size);
    return 0;
}

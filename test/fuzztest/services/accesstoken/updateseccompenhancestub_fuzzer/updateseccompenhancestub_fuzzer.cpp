/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "updateseccompenhancestub_fuzzer.h"

#include <string>
#include <thread>
#include <vector>

#include "accesstoken_manager_service.h"
#undef private
#include "errors.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "iaccess_token_manager.h"
#include "on_permission_used_record_callback_stub.h"
#include "permission_used_request.h"
#include "permission_used_request_parcel.h"

using namespace std;
using namespace OHOS::Security::AccessToken;

namespace OHOS {
    bool UpdateSecCompEnhanceStubFuzzTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size == 0)) {
            return false;
        }

        FuzzedDataProvider provider(data, size);

        MessageParcel datas;
        datas.WriteInterfaceToken(IAccessTokenManager::GetDescriptor());
        if (!datas.WriteInt32(provider.ConsumeIntegral<int32_t>())) {
            return false;
        }

        if (!datas.WriteUint32(provider.ConsumeIntegral<uint32_t>())) {
            return false;
        }

        uint32_t code = static_cast<uint32_t>(IAccessTokenManagerIpcCode::COMMAND_UPDATE_SEC_COMP_ENHANCE);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<AccessTokenManagerService>::GetInstance()->OnRemoteRequest(code, datas, reply, option);

        return true;
    }
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::UpdateSecCompEnhanceStubFuzzTest(data, size);
    return 0;
}

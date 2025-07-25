/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "istoastshownneededstub_fuzzer.h"

#include "accesstoken_manager_service.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "iaccess_token_manager.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
bool IsToastShownNeededStubFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }

    FuzzedDataProvider provider(data, size);

    MessageParcel datas;
    if (!datas.WriteInterfaceToken(IAccessTokenManager::GetDescriptor())) {
        return false;
    }

    int32_t pid = provider.ConsumeIntegral<int32_t>();
    if (!datas.WriteInt32(pid)) {
        return false;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    DelayedSingleton<AccessTokenManagerService>::GetInstance()->OnRemoteRequest(
        static_cast<uint32_t>(IAccessTokenManagerIpcCode::COMMAND_IS_TOAST_SHOWN_NEEDED), datas, reply, option);

    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::IsToastShownNeededStubFuzzTest(data, size);
    return 0;
}

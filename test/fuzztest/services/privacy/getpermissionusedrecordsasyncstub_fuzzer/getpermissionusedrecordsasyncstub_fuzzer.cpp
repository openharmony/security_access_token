/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "getpermissionusedrecordsasyncstub_fuzzer.h"

#include <string>
#include <thread>
#include <vector>

#undef private
#include "errors.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "iprivacy_manager.h"
#include "on_permission_used_record_callback_stub.h"
#include "permission_used_request.h"
#include "permission_used_request_parcel.h"
#include "privacy_manager_service.h"

using namespace std;
using namespace OHOS::Security::AccessToken;

class TestCallBack : public OnPermissionUsedRecordCallbackStub {
public:
    TestCallBack() = default;
    virtual ~TestCallBack() = default;

    void OnQueried(OHOS::ErrCode code, PermissionUsedResult& result)
    {}
};
namespace OHOS {
    bool GetPermissionUsedRecordsAsyncStubFuzzTest(const uint8_t* data, size_t size)
    {
        if ((data == nullptr) || (size == 0)) {
            return false;
        }

        FuzzedDataProvider provider(data, size);
        PermissionUsedRequest request = {
            .tokenId = provider.ConsumeIntegral<AccessTokenID>(),
            .isRemote = provider.ConsumeBool(),
            .deviceId = provider.ConsumeRandomLengthString(),
            .bundleName = provider.ConsumeRandomLengthString(),
            .permissionList = {provider.ConsumeRandomLengthString()},
            .beginTimeMillis = provider.ConsumeIntegral<int64_t>(),
            .endTimeMillis = provider.ConsumeIntegral<int64_t>(),
            .flag = static_cast<PermissionUsageFlag>(provider.ConsumeIntegralInRange<uint32_t>(
                0, static_cast<uint32_t>(PermissionUsageFlag::FLAG_PERMISSION_USAGE_SUMMARY_IN_APP_FOREGROUND))),
        };
        PermissionUsedRequestParcel requestParcel;
        requestParcel.request = request;

        MessageParcel datas;
        datas.WriteInterfaceToken(IPrivacyManager::GetDescriptor());
        if (!datas.WriteParcelable(&requestParcel)) {
            return false;
        }

        sptr<TestCallBack> callback(new (std::nothrow) TestCallBack());
        if (!datas.WriteRemoteObject(callback->AsObject())) {
            return false;
        }

        uint32_t code = static_cast<uint32_t>(IPrivacyManagerIpcCode::COMMAND_GET_PERMISSION_USED_RECORDS_ASYNC);

        MessageParcel reply;
        MessageOption option;
        DelayedSingleton<PrivacyManagerService>::GetInstance()->OnRemoteRequest(code, datas, reply, option);

        return true;
    }
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::GetPermissionUsedRecordsAsyncStubFuzzTest(data, size);
    return 0;
}

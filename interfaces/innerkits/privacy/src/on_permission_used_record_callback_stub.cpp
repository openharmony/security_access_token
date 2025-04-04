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

#include "on_permission_used_record_callback_stub.h"

#include "accesstoken_common_log.h"
#include "privacy_error.h"
#include "permission_used_result_parcel.h"
#include "string_ex.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static constexpr int32_t RET_NOK = -1;
}

int32_t OnPermissionUsedRecordCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    LOGD(PRI_DOMAIN, PRI_TAG, "Entry, code: 0x%{public}x", code);
    std::u16string descriptor = data.ReadInterfaceToken();
    if (descriptor != OnPermissionUsedRecordCallback::GetDescriptor()) {
        LOGE(PRI_DOMAIN, PRI_TAG, "Get unexpect descriptor: %{public}s", Str16ToStr8(descriptor).c_str());
        return ERROR_IPC_REQUEST_FAIL;
    }

    int32_t msgCode =  static_cast<int32_t>(code);
    if (msgCode == static_cast<int32_t>(PrivacyPermissionRecordInterfaceCode::ON_QUERIED)) {
        ErrCode errCode = data.ReadInt32();
        PermissionUsedResult result;
        if (errCode != NO_ERROR) {
            OnQueried(errCode, result);
            return RET_NOK;
        }
        sptr<PermissionUsedResultParcel> resultSptr = data.ReadParcelable<PermissionUsedResultParcel>();
        if (resultSptr == nullptr) {
            LOGE(PRI_DOMAIN, PRI_TAG, "ReadParcelable fail");
            return RET_NOK;
        }
        LOGI(PRI_DOMAIN, PRI_TAG, "ErrCode: %{public}d", errCode);
        OnQueried(errCode, resultSptr->result);
    } else {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return NO_ERROR;
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS

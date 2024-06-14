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

#ifndef EL5_FILEKEY_SERVICE_EXT_INTERFACE_H
#define EL5_FILEKEY_SERVICE_EXT_INTERFACE_H

#include <vector>

#include "data_lock_type.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
class El5FilekeyServiceExtInterface {
public:
    virtual int32_t AcquireAccess(DataLockType type, bool isApp) = 0;
    virtual int32_t ReleaseAccess(DataLockType type, bool isApp) = 0;
    virtual int32_t GenerateAppKey(uint32_t uid, const std::string& bundleName, std::string& keyId) = 0;
    virtual int32_t DeleteAppKey(const std::string& keyId) = 0;
    virtual int32_t GetUserAppKey(int32_t userId, bool getAllFlag,
        std::vector<std::pair<int32_t, std::string>> &keyInfos) = 0;
    virtual int32_t ChangeUserAppkeysLoadInfo(int32_t userId, std::vector<std::pair<std::string, bool>> &loadInfos) = 0;
    virtual int32_t SetFilePathPolicy(int32_t userId) = 0;
    virtual int32_t SetPolicyScreenLocked() = 0;
    virtual int32_t DumpData(int fd, const std::vector<std::u16string>& args) = 0;
    virtual int32_t RegisterCallback(const sptr<El5FilekeyCallbackInterface> &callback) = 0;
};
}  // namespace AccessToken
}  // namespace Security
}  // namespace OHOS
#endif // EL5_FILEKEY_SERVICE_EXT_INTERFACE_H

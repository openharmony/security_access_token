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

#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include <string>

namespace OHOS {
namespace Security {
namespace AccessToken {
enum DeviceIdType {
    NETWORK_ID,
    UNIVERSALLY_UNIQUE_ID,
    UNIQUE_DISABILITY_ID,
    UNKNOWN,
};

struct DeviceId {
    std::string networkId;
    std::string universallyUniqueId; // uuid
    std::string uniqueDeviceId; // udid
};

struct DeviceInfo {
    DeviceId deviceId;
    std::string deviceName;
    std::string deviceType;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS

#endif // DEVICE_INFO_H

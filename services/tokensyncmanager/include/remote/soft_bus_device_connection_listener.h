/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef SOFT_BUS_DEVICE_CONNECTION_LISTENER_H
#define SOFT_BUS_DEVICE_CONNECTION_LISTENER_H

#include <atomic>
#include <memory>
#include <string>

#include "accesstoken_common_log.h"
#include "device_manager_callback.h"
#include "dm_device_info.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
class TokenSyncDmInitCallback final : public DistributedHardware::DmInitCallback {
    void OnRemoteDied() override
    {}
};

class SoftBusDeviceConnectionListener final : public DistributedHardware::DeviceStateCallback {
public:
    SoftBusDeviceConnectionListener();
    ~SoftBusDeviceConnectionListener() override;

    /**
     * @brief node online callback
     *
     * @param deviceInfo node info
     */
    void OnDeviceOnline(const DistributedHardware::DmDeviceInfo &info) override;

    /**
     * @brief node offline callback
     *
     * @param deviceInfo node info
     */
    void OnDeviceOffline(const DistributedHardware::DmDeviceInfo &info) override;

    /**
     * @brief node ready callback
     *
     * @param deviceInfo node info
     */
    void OnDeviceReady(const DistributedHardware::DmDeviceInfo &info) override;

    /**
     * @brief node changed callback
     *
     * @param deviceInfo node info
     */
    void OnDeviceChanged(const DistributedHardware::DmDeviceInfo &info) override;

private:
    void UnloadTokensyncService();
};
}  // namespace AccessToken
}  // namespace Security
}  // namespace OHOS
#endif

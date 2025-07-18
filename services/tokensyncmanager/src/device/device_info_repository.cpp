/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "device_info_repository.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
std::recursive_mutex g_instanceMutex;
}
DeviceInfoRepository &DeviceInfoRepository::GetInstance()
{
    static DeviceInfoRepository* instance = nullptr;
    if (instance == nullptr) {
        std::lock_guard<std::recursive_mutex> lock(g_instanceMutex);
        if (instance == nullptr) {
            DeviceInfoRepository* tmp = new (std::nothrow) DeviceInfoRepository();
            instance = std::move(tmp);
        }
    }
    return *instance;
}

std::vector<DeviceInfo> DeviceInfoRepository::ListDeviceInfo()
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    std::vector<DeviceInfo> deviceInfo;

    std::map<std::string, DeviceInfo>::iterator it;
    std::map<std::string, DeviceInfo>::iterator itEnd;
    it = deviceInfoMap_.begin();
    itEnd = deviceInfoMap_.end();
    while (it != itEnd) {
        deviceInfo.push_back(it->second);
        ++it;
    }
    return deviceInfo;
}

bool DeviceInfoRepository::FindDeviceInfo(const std::string &nodeId, DeviceIdType type, DeviceInfo &deviceInfo)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    DeviceId deviceId;
    if (FindDeviceIdByNodeIdLocked(nodeId, type, deviceId)) {
        return FindDeviceInfoByDeviceIdLocked(deviceId, deviceInfo);
    }
    return false;
}

bool DeviceInfoRepository::FindDeviceIdByNodeIdLocked(
    const std::string &nodeId, const DeviceIdType type, DeviceId &deviceId) const
{
    if (type == DeviceIdType::NETWORK_ID) {
        return FindDeviceIdByNetworkIdLocked(nodeId, deviceId);
    } else if (type == DeviceIdType::UNIVERSALLY_UNIQUE_ID) {
        return FindDeviceIdByUniversallyUniqueIdLocked(nodeId, deviceId);
    } else if (type == DeviceIdType::UNIQUE_DISABILITY_ID) {
        return FindDeviceIdByUniqueDeviceIdLocked(nodeId, deviceId);
    } else if (type == DeviceIdType::UNKNOWN) {
        if (FindDeviceIdByNetworkIdLocked(nodeId, deviceId)) {
            return true;
        } else if (FindDeviceIdByUniversallyUniqueIdLocked(nodeId, deviceId)) {
            return true;
        } else if (FindDeviceIdByUniqueDeviceIdLocked(nodeId, deviceId)) {
            return true;
        }
        return false;
    } else {
        return false;
    }
}

bool DeviceInfoRepository::FindDeviceInfoByDeviceIdLocked(const DeviceId deviceId, DeviceInfo &deviceInfo) const
{
    std::string deviceInfoKey = deviceId.networkId + deviceId.universallyUniqueId + deviceId.uniqueDeviceId;
    if (deviceInfoMap_.count(deviceInfoKey) > 0) {
        deviceInfo = deviceInfoMap_.at(deviceInfoKey);
        return true;
    }
    return false;
}

bool DeviceInfoRepository::FindDeviceIdByNetworkIdLocked(const std::string &networkId, DeviceId &deviceId) const
{
    if (deviceIdMapByNetworkId_.count(networkId) > 0) {
        deviceId = deviceIdMapByNetworkId_.at(networkId);
        return true;
    }
    return false;
}

bool DeviceInfoRepository::FindDeviceIdByUniversallyUniqueIdLocked(
    const std::string &universallyUniqueId, DeviceId &deviceId) const
{
    if (deviceIdMapByUniversallyUniqueId_.count(universallyUniqueId) > 0) {
        deviceId = deviceIdMapByUniversallyUniqueId_.at(universallyUniqueId);
        return true;
    }
    return false;
}

bool DeviceInfoRepository::FindDeviceIdByUniqueDeviceIdLocked(
    const std::string &uniqueDeviceId, DeviceId &deviceId) const
{
    if (deviceIdMapByUniqueDeviceId_.count(uniqueDeviceId) > 0) {
        deviceId = deviceIdMapByUniqueDeviceId_.at(uniqueDeviceId);
        return true;
    }
    return false;
}

void DeviceInfoRepository::DeleteAllDeviceInfoExceptOne(const DeviceInfo deviceInfo)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    deviceIdMapByNetworkId_.clear();
    deviceIdMapByUniversallyUniqueId_.clear();
    deviceIdMapByUniqueDeviceId_.clear();
    deviceInfoMap_.clear();
    SaveDeviceInfo(deviceInfo);
}

void DeviceInfoRepository::SaveDeviceInfo(const DeviceInfo deviceInfo)
{
    SaveDeviceInfo(deviceInfo.deviceId, deviceInfo.deviceName, deviceInfo.deviceType);
}

void DeviceInfoRepository::SaveDeviceInfo(
    const DeviceId deviceId, const std::string &deviceName, const std::string &deviceType)
{
    SaveDeviceInfo(
        deviceId.networkId, deviceId.universallyUniqueId, deviceId.uniqueDeviceId, deviceName, deviceType);
}

void DeviceInfoRepository::SaveDeviceInfo(const std::string &networkId, const std::string &universallyUniqueId,
    const std::string &uniqueDeviceId, const std::string &deviceName, const std::string &deviceType)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);

    DeleteDeviceInfo(networkId, DeviceIdType::NETWORK_ID);
    DeleteDeviceInfo(universallyUniqueId, DeviceIdType::UNIVERSALLY_UNIQUE_ID);
    DeleteDeviceInfo(uniqueDeviceId, DeviceIdType::UNIQUE_DISABILITY_ID);

    DeviceId deviceId;
    deviceId.networkId = networkId;
    deviceId.universallyUniqueId = universallyUniqueId;
    deviceId.uniqueDeviceId = uniqueDeviceId;

    DeviceInfo deviceInfo;
    deviceInfo.deviceId = deviceId;
    deviceInfo.deviceName = deviceName;
    deviceInfo.deviceType = deviceType;

    const std::string deviceInfoKey = networkId + universallyUniqueId + uniqueDeviceId;
    deviceIdMapByNetworkId_.insert(std::pair<std::string, DeviceId>(networkId, deviceId));
    deviceIdMapByUniversallyUniqueId_.insert(std::pair<std::string, DeviceId>(universallyUniqueId, deviceId));
    deviceIdMapByUniqueDeviceId_.insert(std::pair<std::string, DeviceId>(uniqueDeviceId, deviceId));
    deviceInfoMap_.insert(std::pair<std::string, DeviceInfo>(deviceInfoKey, deviceInfo));
}

void DeviceInfoRepository::DeleteDeviceInfo(const std::string &nodeId, const DeviceIdType type)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    DeviceId deviceId;
    if (FindDeviceIdByNodeIdLocked(nodeId, type, deviceId)) {
        DeleteDeviceInfoByDeviceIdLocked(deviceId);
    }
}

void DeviceInfoRepository::DeleteDeviceInfoByDeviceIdLocked(const DeviceId deviceId)
{
    deviceIdMapByNetworkId_.erase(deviceId.networkId);
    deviceIdMapByUniversallyUniqueId_.erase(deviceId.universallyUniqueId);
    deviceIdMapByUniqueDeviceId_.erase(deviceId.uniqueDeviceId);
    const std::string deviceInfoKey = deviceId.networkId + deviceId.universallyUniqueId + deviceId.uniqueDeviceId;
    deviceInfoMap_.erase(deviceInfoKey);
}

void DeviceInfoRepository::Clear()
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    deviceIdMapByNetworkId_.clear();
    deviceIdMapByUniversallyUniqueId_.clear();
    deviceIdMapByUniqueDeviceId_.clear();
    deviceInfoMap_.clear();
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS

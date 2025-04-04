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

#ifndef SOFT_BUS_MANAGER_H
#define SOFT_BUS_MANAGER_H

#include <functional>
#include <cinttypes>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "device_manager.h"
#include "remote_command_executor.h"
#include "socket.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
class SoftBusManager final {
public:
    virtual ~SoftBusManager();

    /**
     * @brief Get instance of SoftBusManager
     *
     * @return SoftBusManager's instance.
     * @since 1.0
     * @version 1.0
     */
    static SoftBusManager &GetInstance();

    /**
     * @brief Bind soft bus service.
     *
     * @since 1.0
     * @version 1.0
     */
    void Initialize();

    /**
     * @brief Unbind soft bus service when DPMS has been destroyed.
     *
     * @since 1.0
     * @version 1.0
     */
    void Destroy();

    /**
     * @brief Open session with the peer device sychronized.
     *
     * @param deviceUdid The udid of peer device.
     * @return Session id if open successfully, otherwise return -1(Constant::FAILURE).
     * @since 1.0
     * @version 1.0
     */
    int BindService(const std::string &deviceUdid);

    /**
     * @brief Close socket with the peer device.
     *
     * @param socketFd The socket id need to close.
     * @return 0 if close successfully, otherwise return -1(Constant::FAILURE).
     * @since 1.0
     * @version 1.0
     */
    int CloseSocket(int socketFd);

    /**
     * @brief Get UUID(networkId) by deviceNodeId.
     *
     * @param networkId The valid networkId.
     * @return uuid if deviceManager is ready, empty string otherwise.
     * @since 1.0
     * @version 1.0
     */
    std::string GetUniversallyUniqueIdByNodeId(const std::string &networkId);

    /**
     *  @brief Get deviceId(UDID) by deviceNodeId.
     *
     * @param networkId The valid networkId.
     * @return udid if deviceManager work correctly, empty string otherwise.
     * @since 1.0
     * @version 1.0
     */
    std::string GetUniqueDeviceIdByNodeId(const std::string &networkId);

    bool GetNetworkIdBySocket(const int32_t socket, std::string& networkId);

    int32_t GetRepeatTimes();

public:
    static const std::string SESSION_NAME;

private:
    SoftBusManager();
    int DeviceInit();
    bool CheckAndCopyStr(char* dest, uint32_t destLen, const std::string& src);
    int32_t InitSocketAndListener(const std::string& networkId, ISocketListener& listener);
    int32_t ServiceSocketInit();

    /**
     * @brief Fulfill local device info
     *
     * @return 0 if operate successfully, otherwise return -1(Constant::FAILURE).
     * @since 1.0
     * @version 1.0
     */
    int FulfillLocalDeviceInfo();

    /**
     * @brief add all trusted device info.
     *
     * @since 1.0
     * @version 1.0
     */
    int AddTrustedDeviceInfo();

    void SetDefaultConfigValue();
    void GetConfigValue();

    // soft bus session server opened flag
    bool isSoftBusServiceBindSuccess_;
    std::atomic_bool inited_;

    // init mutex
    std::mutex mutex_;

    // fulfill thread mutex
    std::mutex fulfillMutex_;

    // soft bus service socket fd
    int32_t socketFd_ = -1;

    // soft bus client socket with networkId map
    std::mutex clientSocketMutex_;
    std::map<int32_t, std::string> clientSocketMap_;

    // remote request overtime repeat times
    int32_t sendRequestRepeatTimes_ = 0;
};
}  // namespace AccessToken
}  // namespace Security
}  // namespace OHOS
#endif  // SOFT_BUS_MANAGER_H

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

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <thread>

#define private public
#include "remote_command_executor.h"
#include "token_sync_manager_service.h"
#include "soft_bus_manager.h"
#include "soft_bus_channel.h"
#undef private

#include "gtest/gtest.h"
#include "accesstoken_kit.h"
#include "accesstoken_common_log.h"
#include "access_token_error.h"
#include "base_remote_command.h"
#include "constant_common.h"
#include "delete_remote_token_command.h"
#include "device_info_manager.h"
#include "device_info_repository.h"
#include "device_info.h"
#include "device_manager_callback.h"
#include "dm_device_info.h"
#include "i_token_sync_manager.h"
#include "remote_command_manager.h"
#include "socket.h"
#include "soft_bus_device_connection_listener.h"
#include "soft_bus_socket_listener.h"
#include "test_common.h"
#include "token_setproc.h"
#include "token_sync_manager_stub.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static DistributedHardware::DmDeviceInfo g_devInfo = {
    // udid = deviceid-1:udid-001  uuid = deviceid-1:uuid-001
    .deviceId = "deviceid-1",
    .deviceName = "remote_mock",
    .deviceTypeId = 1,
    .networkId = "deviceid-1"
};

static std::vector<std::thread> threads_;
static std::shared_ptr<SoftBusDeviceConnectionListener> g_ptrDeviceStateCallback =
    std::make_shared<SoftBusDeviceConnectionListener>();
static int32_t g_selfUid;
static AccessTokenID g_selfTokenId = 0;
static const int32_t OUT_OF_MAP_SOCKET = 2;
}

class TokenSyncServiceTest : public testing::Test {
public:
    TokenSyncServiceTest();
    ~TokenSyncServiceTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void OnDeviceOffline(const DistributedHardware::DmDeviceInfo &info);
    void SetUp();
    void TearDown();
    std::shared_ptr<TokenSyncManagerService> tokenSyncManagerService_;
};

TokenSyncServiceTest::TokenSyncServiceTest()
{
    DelayedSingleton<TokenSyncManagerService>::GetInstance()->Initialize();
}
TokenSyncServiceTest::~TokenSyncServiceTest()
{}

void NativeTokenGet()
{
    uint64_t tokenId = 0;
    tokenId = TestCommon::GetNativeTokenIdFromProcess("token_sync_service");
    ASSERT_NE(tokenId, static_cast<AccessTokenID>(0));
    EXPECT_EQ(0, SetSelfTokenID(tokenId));
}

void TokenSyncServiceTest::SetUpTestCase()
{
    g_selfUid = getuid();
    g_selfTokenId = GetSelfTokenID();
    TestCommon::SetTestEvironment(g_selfTokenId);
    NativeTokenGet();
}
void TokenSyncServiceTest::TearDownTestCase()
{
    SetSelfTokenID(g_selfTokenId);
    TestCommon::ResetTestEvironment();
}
void TokenSyncServiceTest::SetUp()
{
    tokenSyncManagerService_ = DelayedSingleton<TokenSyncManagerService>::GetInstance();
    EXPECT_NE(nullptr, tokenSyncManagerService_);
}
void TokenSyncServiceTest::TearDown()
{
    LOGI(ATM_DOMAIN, ATM_TAG, "TearDown start.");
    tokenSyncManagerService_ = nullptr;
    for (auto it = threads_.begin(); it != threads_.end(); it++) {
        it->join();
    }
    threads_.clear();

    if (g_ptrDeviceStateCallback != nullptr) {
        OnDeviceOffline(g_devInfo);
        sleep(1);
    }
}

void TokenSyncServiceTest::OnDeviceOffline(const DistributedHardware::DmDeviceInfo &info)
{
    std::string networkId = info.networkId;
    std::string uuid = DeviceInfoManager::GetInstance().ConvertToUniversallyUniqueIdOrFetch(networkId);
    std::string udid = DeviceInfoManager::GetInstance().ConvertToUniqueDeviceIdOrFetch(networkId);

    LOGI(ATM_DOMAIN, ATM_TAG,
        "networkId: %{public}s,  uuid: %{public}s, udid: %{public}s",
        networkId.c_str(),
        uuid.c_str(),
        ConstantCommon::EncryptDevId(udid).c_str());

    if (uuid != "" && udid != "") {
        RemoteCommandManager::GetInstance().NotifyDeviceOffline(uuid);
        RemoteCommandManager::GetInstance().NotifyDeviceOffline(udid);
        DeviceInfoManager::GetInstance().RemoveRemoteDeviceInfo(networkId, DeviceIdType::NETWORK_ID);
    } else {
        LOGE(ATM_DOMAIN, ATM_TAG, "uuid or udid is empty, offline failed.");
    }
}

/**
 * @tc.name: CheckAndCopyStr001
 * @tc.desc: destlen not equal to src
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TokenSyncServiceTest, CheckAndCopyStr001, TestSize.Level4)
{
    std::string test_src = "testSrc";
    ASSERT_FALSE(SoftBusManager::GetInstance().CheckAndCopyStr(nullptr, test_src.length(), test_src));
}

/**
 * @tc.name: CloseSocket001
 * @tc.desc: invalid socketFd
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TokenSyncServiceTest, CloseSocket001, TestSize.Level4)
{
    ASSERT_EQ(Constant::FAILURE, SoftBusManager::GetInstance().CloseSocket(-1));
    ASSERT_EQ(Constant::SUCCESS, SoftBusManager::GetInstance().CloseSocket(OUT_OF_MAP_SOCKET));
    std::string networkId;
    ASSERT_FALSE(SoftBusManager::GetInstance().GetNetworkIdBySocket(OUT_OF_MAP_SOCKET, networkId));
}

/**
 * @tc.name: GetUniversallyUniqueIdByNodeId001
 * @tc.desc: invalid nodeId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TokenSyncServiceTest, GetUniversallyUniqueIdByNodeId001, TestSize.Level4)
{
    SoftBusManager::GetInstance().Initialize();
    SoftBusManager::GetInstance().SetDefaultConfigValue();
    ASSERT_EQ("", SoftBusManager::GetInstance().GetUniversallyUniqueIdByNodeId(""));
    ASSERT_EQ("", SoftBusManager::GetInstance().GetUniqueDeviceIdByNodeId(""));
}

/**
 * @tc.name: InsertCallbackAndExcute001
 * @tc.desc: Ond
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TokenSyncServiceTest, InsertCallbackAndExcute001, TestSize.Level4)
{
    SoftBusDeviceConnectionListener listener;
    listener.OnDeviceOffline(g_devInfo);
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(g_devInfo.networkId, "123", g_devInfo.deviceId,
        g_devInfo.deviceName, std::to_string(g_devInfo.deviceTypeId));
    listener.OnDeviceOffline(g_devInfo);
    SoftBusChannel channel("test");
    std::string test("test");
    channel.InsertCallback(0, test);
    ASSERT_EQ(true, channel.isSocketUsing_);
    ASSERT_EQ("", channel.ExecuteCommand("test", "test"));
}
}  // namespace AccessToken
}  // namespace Security
}  // namespace OHOS

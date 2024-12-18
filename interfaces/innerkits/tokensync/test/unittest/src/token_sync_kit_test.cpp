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

#include "token_sync_kit_test.h"

#include "i_token_sync_manager.h"
#include "token_sync_manager_client.h"

using namespace testing::ext;

namespace OHOS {
namespace Security {
namespace AccessToken {
void TokenSyncKitTest::SetUpTestCase()
{}

void TokenSyncKitTest::TearDownTestCase()
{
}

void TokenSyncKitTest::SetUp()
{
}

void TokenSyncKitTest::TearDown()
{}

static const int TIME_500_MS = 1000 * 500; // 0.5 second

static void StartOrStopTokenSyncService(bool start)
{
    pid_t pid = fork();
    int ret = 0;
    if (pid == 0) {
        if (start) {
            ret = execlp("service_control", "service_control", "start", "token_sync_service", nullptr);
        } else {
            ret = execlp("service_control", "service_control", "stop", "token_sync_service", nullptr);
        }
        if (ret == -1) {
            std::cout << "execlp failed" << std::endl;
        }
        exit(0);
    }
    usleep(TIME_500_MS);
}

/**
 * @tc.name: UpdateRemoteHapTokenInfo001
 * @tc.desc: TokenSyncManagerProxy::UpdateRemoteHapTokenInfo function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TokenSyncKitTest, UpdateRemoteHapTokenInfo001, TestSize.Level1)
{
    HapTokenInfoForSync tokenInfo;

    ASSERT_EQ(TokenSyncError::TOKEN_SYNC_IPC_ERROR,
        TokenSyncManagerClient::GetInstance().UpdateRemoteHapTokenInfo(tokenInfo));

    StartOrStopTokenSyncService(true);
    ASSERT_EQ(0, TokenSyncManagerClient::GetInstance().UpdateRemoteHapTokenInfo(tokenInfo));
    StartOrStopTokenSyncService(false);
}

/**
 * @tc.name: GetRemoteHapTokenInfo001
 * @tc.desc: TokenSyncManagerProxy::GetRemoteHapTokenInfo function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TokenSyncKitTest, GetRemoteHapTokenInfo001, TestSize.Level1)
{
    ASSERT_EQ(TokenSyncError::TOKEN_SYNC_IPC_ERROR,
        TokenSyncManagerClient::GetInstance().GetRemoteHapTokenInfo("", 0));

    StartOrStopTokenSyncService(true);
    ASSERT_EQ(TokenSyncError::TOKEN_SYNC_PARAMS_INVALID,
        TokenSyncManagerClient::GetInstance().GetRemoteHapTokenInfo("", 0));
    StartOrStopTokenSyncService(false);
}

/**
 * @tc.name: DeleteRemoteHapTokenInfo001
 * @tc.desc: TokenSyncManagerProxy::DeleteRemoteHapTokenInfo function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TokenSyncKitTest, DeleteRemoteHapTokenInfo001, TestSize.Level1)
{
    ASSERT_EQ(TokenSyncError::TOKEN_SYNC_IPC_ERROR,
        TokenSyncManagerClient::GetInstance().DeleteRemoteHapTokenInfo(0));

    StartOrStopTokenSyncService(true);
    ASSERT_EQ(TokenSyncError::TOKEN_SYNC_PARAMS_INVALID,
        TokenSyncManagerClient::GetInstance().DeleteRemoteHapTokenInfo(0));
    StartOrStopTokenSyncService(false);
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS

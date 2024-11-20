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

#include "remote_command_factory.h"

#include "nlohmann/json.hpp"

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
std::recursive_mutex g_instanceMutex;
}

RemoteCommandFactory &RemoteCommandFactory::GetInstance()
{
    static RemoteCommandFactory* instance = nullptr;
    if (instance == nullptr) {
        std::lock_guard<std::recursive_mutex> lock(g_instanceMutex);
        if (instance == nullptr) {
            instance = new RemoteCommandFactory();
        }
    }
    return *instance;
}

std::shared_ptr<SyncRemoteHapTokenCommand> RemoteCommandFactory::NewSyncRemoteHapTokenCommand(
    const std::string &srcDeviceId, const std::string &dstDeviceId, AccessTokenID tokenID)
{
    return std::make_shared<SyncRemoteHapTokenCommand>(srcDeviceId, dstDeviceId, tokenID);
}

std::shared_ptr<DeleteRemoteTokenCommand> RemoteCommandFactory::NewDeleteRemoteTokenCommand(
    const std::string &srcDeviceId, const std::string &dstDeviceId, AccessTokenID tokenID)
{
    return std::make_shared<DeleteRemoteTokenCommand>(srcDeviceId, dstDeviceId, tokenID);
}

std::shared_ptr<UpdateRemoteHapTokenCommand> RemoteCommandFactory::NewUpdateRemoteHapTokenCommand(
    const std::string &srcDeviceId, const std::string &dstDeviceId, const HapTokenInfoForSync& tokenInfo)
{
    return std::make_shared<UpdateRemoteHapTokenCommand>(srcDeviceId, dstDeviceId, tokenInfo);
}

std::shared_ptr<BaseRemoteCommand> RemoteCommandFactory::NewRemoteCommandFromJson(
    const std::string &commandName, const std::string &commandJsonString)
{
    if (commandName == "SyncRemoteHapTokenCommand") {
        return std::make_shared<SyncRemoteHapTokenCommand>(commandJsonString);
    }
    if (commandName == "DeleteRemoteTokenCommand") {
        return std::make_shared<DeleteRemoteTokenCommand>(commandJsonString);
    }
    if (commandName == "UpdateRemoteHapTokenCommand") {
        return std::make_shared<UpdateRemoteHapTokenCommand>(commandJsonString);
    }
    return nullptr;
}
}  // namespace AccessToken
}  // namespace Security
}  // namespace OHOS

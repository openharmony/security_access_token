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

#include "libraryloader.h"
#include <string>
#include "access_token_error.h"
#include "ability_manager_access_loader.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static constexpr uint32_t INVALID_INDEX = 0;
}

class AbilityManagerAccessLoaderMock final: public AbilityManagerAccessLoaderInterface {
    int32_t StartAbility(const AAFwk::Want &want, const sptr<IRemoteObject> &callerToken) override;
};

int32_t AbilityManagerAccessLoaderMock::StartAbility(const AAFwk::Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    if (want.GetIntParam("appIndex", 0) == INVALID_INDEX) {
        return AccessTokenError::ERR_SERVICE_ABNORMAL;
    }
    return ERR_OK;
}

LibraryLoader::LibraryLoader(const std::string& path)
{
    instance_ = new AbilityManagerAccessLoaderMock();
    handle_ = nullptr;
}

LibraryLoader::~LibraryLoader()
{}

bool LibraryLoader::PrintErrorLog(const std::string& targetName)
{
    return true;
}

void LibraryLoader::Create()
{}

void LibraryLoader::Destroy()
{}
} // AccessToken
} // Security
} // OHOS
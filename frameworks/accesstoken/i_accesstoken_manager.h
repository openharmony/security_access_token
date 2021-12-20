/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef I_ACCESSTOKEN_MANAGER_H
#define I_ACCESSTOKEN_MANAGER_H

#include <string>
#include "iremote_broker.h"
#include "errors.h"

#include "accesstoken.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
class IAccessTokenManager : public IRemoteBroker {
public:
    static const int SA_ID_ACCESSTOKEN_MANAGER_SERVICE = 3503;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.security.accesstoken.IAccessTokennManager");

    virtual int VerifyAccesstoken(AccessTokenID tokenID, const std::string &permissionName) = 0;

    enum class InterfaceCode {
        VERIFY_ACCESSTOKEN = 0xff01,
    };
};
}  // namespace AccessToken
}  // namespace Security
}  // namespace OHOS

#endif  // I_ACCESSTOKEN_MANAGER_H

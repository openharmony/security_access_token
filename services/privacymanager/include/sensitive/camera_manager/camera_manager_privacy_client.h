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

#ifndef CAMERA_MANAGER_PRIVACY_CLIENT_H
#define CAMERA_MANAGER_PRIVACY_CLIENT_H

#include <mutex>
#include <stdint.h>
#include <string>

#include "camera_manager_privacy_death_recipient.h"
#include "camera_manager_privacy_proxy.h"
#include "nocopyable.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
class CameraManagerPrivacyClient final {
public:
    static CameraManagerPrivacyClient& GetInstance();
    virtual ~CameraManagerPrivacyClient();

    int32_t MuteCameraPersist(PolicyType policyType, bool muteMode);
    bool IsCameraMuted();
    void OnRemoteDiedHandle();

private:
    CameraManagerPrivacyClient();
    DISALLOW_COPY_AND_MOVE(CameraManagerPrivacyClient);

    void InitProxy();
    sptr<ICameraService> GetProxy();
    void ReleaseProxy();

    sptr<CameraManagerPrivacyDeathRecipient> serviceDeathObserver_ = nullptr;
    std::mutex proxyMutex_;
    sptr<ICameraService> proxy_ = nullptr;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // CAMERA_MANAGER_PRIVACY_CLIENT_H


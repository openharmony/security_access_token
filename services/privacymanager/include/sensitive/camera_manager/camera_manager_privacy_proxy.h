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

#ifndef OHOS_CAMERA_MANAGER_PRIVACY_PROXY_H
#define OHOS_CAMERA_MANAGER_PRIVACY_PROXY_H

#include <iremote_proxy.h>

#include "privacy_camera_service_ipc_interface_code.h"
#include "privacy_param.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
class ICameraService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ICameraService");

    virtual int32_t MuteCameraPersist(PolicyType policyType, bool muteMode) = 0;
    virtual int32_t IsCameraMuted(bool &muteMode) = 0;
};

class CameraManagerPrivacyProxy : public IRemoteProxy<ICameraService> {
public:
    explicit CameraManagerPrivacyProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<ICameraService>(impl) {}

    virtual ~CameraManagerPrivacyProxy() = default;

    int32_t MuteCameraPersist(PolicyType policyType, bool muteMode) override;
    int32_t IsCameraMuted(bool &muteMode) override;
private:
    static inline BrokerDelegator<CameraManagerPrivacyProxy> delegator_;
};
}
}
}
#endif // OHOS_CAMERA_MANAGER_PRIVACY_PROXY_H

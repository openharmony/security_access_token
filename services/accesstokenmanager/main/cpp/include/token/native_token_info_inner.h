/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef ACCESSTOKEN_NATIVE_TOKEN_INFO_INNER_H
#define ACCESSTOKEN_NATIVE_TOKEN_INFO_INNER_H

#include <string>
#include <vector>
#include "access_token.h"
#include "generic_values.h"
#include "native_token_info_base.h"
#include "permission_policy_set.h"
#include "permission_state_full.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
static const int MAX_DCAPS_NUM = 10 * 1024;
static const int MAX_REQ_PERM_NUM = 10 * 1024;

class NativeTokenInfoInner final {
public:
    NativeTokenInfoInner();
    NativeTokenInfoInner(NativeTokenInfoBase& info,
        const std::vector<PermissionStateFull>& permStateList);
    virtual ~NativeTokenInfoInner();

    void TransferNativeInfo(std::vector<GenericValues>& valueList) const;
    void TransferPermissionPolicy(std::vector<GenericValues>& permStateValues) const;
    void SetDcaps(const std::string& dcapStr);
    void SetNativeAcls(const std::string& aclsStr);
    void ToString(std::string& info) const;
    int RestoreNativeTokenInfo(AccessTokenID tokenId, const GenericValues& inGenericValues,
        const std::vector<GenericValues>& permStateRes);

    AccessTokenID GetTokenID() const;
    AccessTokenID GetApl() const;
    std::string GetProcessName() const;
    std::shared_ptr<PermissionPolicySet> GetNativeInfoPermissionPolicySet() const;
    uint32_t GetReqPermissionSize() const;

private:
    std::string DcapToString(const std::vector<std::string>& dcap) const;
    std::string NativeAclsToString(const std::vector<std::string>& nativeAcls) const;

    NativeTokenInfoBase tokenInfoBasic_;
    std::shared_ptr<PermissionPolicySet> permPolicySet_;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // ACCESSTOKEN_NATIVE_TOKEN_INFO_INNER_H

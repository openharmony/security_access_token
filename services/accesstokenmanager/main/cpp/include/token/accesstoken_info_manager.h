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

#ifndef ACCESSTOKEN_TOKEN_INFO_MANAGER_H
#define ACCESSTOKEN_TOKEN_INFO_MANAGER_H

#include <algorithm>
#include <atomic>
#include <map>
#include <memory>
#include <vector>

#include "access_token.h"
#include "atm_tools_param_info.h"
#ifdef TOKEN_SYNC_ENABLE
#include "device_manager.h"
#endif
#include "hap_token_info.h"
#include "hap_token_info_inner.h"
#include "native_token_info_inner.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
static const int UDID_MAX_LENGTH = 128; // udid/uuid max length

#ifdef TOKEN_SYNC_ENABLE
class AccessTokenDmInitCallback final : public DistributedHardware::DmInitCallback {
    void OnRemoteDied() override
    {}
};
#endif

class AccessTokenInfoManager final {
public:
    static AccessTokenInfoManager& GetInstance();
    ~AccessTokenInfoManager();
    void Init();
    std::shared_ptr<HapTokenInfoInner> GetHapTokenInfoInner(AccessTokenID id);
    int GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo& infoParcel);
    std::shared_ptr<NativeTokenInfoInner> GetNativeTokenInfoInner(AccessTokenID id);
    int GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfoBase& info);
    int AllocAccessTokenIDEx(const HapInfoParams& info, AccessTokenID tokenId, AccessTokenIDEx& tokenIdEx);
    int RemoveHapTokenInfo(AccessTokenID id);
    int RemoveNativeTokenInfo(AccessTokenID id);
    int32_t GetHapAppIdByTokenId(AccessTokenID tokenID, std::string& appId);
    int CreateHapTokenInfo(const HapInfoParams& info, const HapPolicyParams& policy, AccessTokenIDEx& tokenIdEx);
    AccessTokenIDEx GetHapTokenID(int32_t userID, const std::string& bundleName, int32_t instIndex);
    AccessTokenID AllocLocalTokenID(const std::string& remoteDeviceID, AccessTokenID remoteTokenID);
    void ProcessNativeTokenInfos(const std::vector<std::shared_ptr<NativeTokenInfoInner>>& tokenInfos);
    int32_t UpdateHapToken(AccessTokenIDEx& tokenIdEx, const UpdateHapInfoParams& info,
        const std::vector<PermissionStateFull>& permStateList, ATokenAplEnum apl,
        const std::vector<PermissionDef>& permList);
    void DumpTokenInfo(const AtmToolsParamInfo& info, std::string& dumpInfo);
    bool IsTokenIdExist(AccessTokenID id);
    AccessTokenID GetNativeTokenId(const std::string& processName);
    void GetRelatedSandBoxHapList(AccessTokenID tokenId, std::vector<AccessTokenID>& tokenIdList);
    int32_t GetHapTokenDlpType(AccessTokenID id);
    int32_t SetPermDialogCap(AccessTokenID tokenID, bool enable);
    int32_t InitUserPolicy(const std::vector<UserState>& userList, const std::vector<std::string>& permList);
    int32_t UpdateUserPolicy(const std::vector<UserState>& userList);
    int32_t ClearUserPolicy();
    bool GetPermDialogCap(AccessTokenID tokenID);
    void DumpToken();
    int32_t GetCurDumpTaskNum();
    void AddDumpTaskNum();
    void ReduceDumpTaskNum();
    void ClearUserGrantedPermissionState(AccessTokenID tokenID);
    int32_t ClearUserGrantedPermission(AccessTokenID tokenID);
    bool IsPermissionRestrictedByUserPolicy(AccessTokenID id, const std::string& permissionName);
    int32_t VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName);
    int32_t VerifyNativeAccessToken(AccessTokenID tokenID, const std::string& permissionName);
    void ClearHapPolicy();

#ifdef TOKEN_SYNC_ENABLE
    /* tokensync needed */
    void InitDmCallback(void);
    int GetHapTokenSync(AccessTokenID tokenID, HapTokenInfoForSync& hapSync);
    int GetHapTokenInfoFromRemote(AccessTokenID tokenID,
        HapTokenInfoForSync& hapSync);
    int SetRemoteHapTokenInfo(const std::string& deviceID, HapTokenInfoForSync& hapSync);
    bool IsRemoteHapTokenValid(const std::string& deviceID, const HapTokenInfoForSync& hapSync);
    int DeleteRemoteToken(const std::string& deviceID, AccessTokenID tokenID);
    AccessTokenID GetRemoteNativeTokenID(const std::string& deviceID, AccessTokenID tokenID);
    int DeleteRemoteDeviceTokens(const std::string& deviceID);
#endif

    bool UpdateCapStateToDatabase(AccessTokenID tokenID, bool enable);

private:
    AccessTokenInfoManager();
    DISALLOW_COPY_AND_MOVE(AccessTokenInfoManager);

    void InitHapTokenInfos(uint32_t& hapSize);
    void InitNativeTokenInfos(uint32_t& nativeSize);
    int AddHapTokenInfo(const std::shared_ptr<HapTokenInfoInner>& info);
    int AddNativeTokenInfo(const std::shared_ptr<NativeTokenInfoInner>& info);
    std::string GetHapUniqueStr(const std::shared_ptr<HapTokenInfoInner>& info) const;
    std::string GetHapUniqueStr(const int& userID, const std::string& bundleName, const int& instIndex) const;
    bool TryUpdateExistNativeToken(const std::shared_ptr<NativeTokenInfoInner>& infoPtr);
    int AllocNativeToken(const std::shared_ptr<NativeTokenInfoInner>& infoPtr);
    int AddHapTokenInfoToDb(AccessTokenID tokenID, const std::shared_ptr<HapTokenInfoInner>& hapInfo);
    int AddNativeTokenInfoToDb(
        const std::vector<GenericValues>& nativeInfoValues, const std::vector<GenericValues>& permStateValues);
    int RemoveTokenInfoFromDb(AccessTokenID tokenID, bool isHap = true);
    int CreateRemoteHapTokenInfo(AccessTokenID mapID, HapTokenInfoForSync& hapSync);
    int UpdateRemoteHapTokenInfo(AccessTokenID mapID, HapTokenInfoForSync& hapSync);
    void PermissionStateNotify(const std::shared_ptr<HapTokenInfoInner>& info, AccessTokenID id);
    void DumpHapTokenInfoByTokenId(const AccessTokenID tokenId, std::string& dumpInfo);
    void DumpHapTokenInfoByBundleName(const std::string& bundleName, std::string& dumpInfo);
    void DumpAllHapTokenname(std::string& dumpInfo);
    void DumpNativeTokenInfoByProcessName(const std::string& processName, std::string& dumpInfo);
    void DumpAllNativeTokenName(std::string& dumpInfo);
    int32_t ParseUserPolicyInfo(const std::vector<UserState>& userList,
        const std::vector<std::string>& permList, std::map<int32_t, bool>& changedUserList);
    int32_t ParseUserPolicyInfo(const std::vector<UserState>& userList,
        std::map<int32_t, bool>& changedUserList);
    int32_t UpdatePermissionStateToKernel(const std::vector<std::string>& permCodeList,
        const std::map<AccessTokenID, bool>& tokenIdList);
    int32_t UpdatePermissionStateToKernel(const std::map<AccessTokenID, bool>& tokenIdList);
    void GetGoalHapList(std::map<AccessTokenID, bool>& tokenIdList,
        std::map<int32_t, bool>& changedUserList);
    bool hasInited_;
    std::atomic_int32_t dumpTaskNum_;

    OHOS::Utils::RWLock hapTokenInfoLock_;
    OHOS::Utils::RWLock nativeTokenInfoLock_;
    OHOS::Utils::RWLock managerLock_;
    OHOS::Utils::RWLock modifyLock_;

    std::map<int, std::shared_ptr<HapTokenInfoInner>> hapTokenInfoMap_;
    std::map<std::string, AccessTokenID> hapTokenIdMap_;
    std::map<uint32_t, NativeTokenInfoCache> nativeTokenInfoMap_;

    OHOS::Utils::RWLock userPolicyLock_;
    std::vector<int32_t> inactiveUserList_;
    std::vector<std::string> permPolicyList_;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif // ACCESSTOKEN_TOKEN_INFO_MANAGER_H

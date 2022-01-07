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

#include "accesstoken_info_manager.h"

#include "accesstoken_id_manager.h"
#include "accesstoken_log.h"
#include "data_storage.h"
#include "data_translator.h"
#include "data_validator.h"
#include "field_const.h"
#include "generic_values.h"
#include "hap_token_info_inner.h"
#include "permission_manager.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_ACCESSTOKEN, "AccessTokenInfoManager"};
}

AccessTokenInfoManager::AccessTokenInfoManager() : hasInited_(false)
{}

AccessTokenInfoManager::~AccessTokenInfoManager()
{
    if (!hasInited_) {
        return;
    }
    this->tokenDataWorker_.Stop();
    this->hasInited_ = false;
}

void AccessTokenInfoManager::Init()
{
    OHOS::Utils::UniqueWriteGuard<OHOS::Utils::RWLock> lk(this->managerLock_);
    if (hasInited_) {
        return;
    }

    ACCESSTOKEN_LOG_INFO(LABEL, "init begin!");
    InitHapTokenInfos();
    InitNativeTokenInfos();
    this->tokenDataWorker_.Start(1);
    hasInited_ = true;
    ACCESSTOKEN_LOG_INFO(LABEL, "Init success");
}

void AccessTokenInfoManager::InitHapTokenInfos()
{
    std::vector<GenericValues> hapTokenRes;
    std::vector<GenericValues> permDefRes;
    std::vector<GenericValues> permStateRes;

    DataStorage::GetRealDataStorage().Find(DataStorage::ACCESSTOKEN_HAP_INFO, hapTokenRes);
    DataStorage::GetRealDataStorage().Find(DataStorage::ACCESSTOKEN_PERMISSION_DEF, permDefRes);
    DataStorage::GetRealDataStorage().Find(DataStorage::ACCESSTOKEN_PERMISSION_STATE, permStateRes);

    for (GenericValues& tokenValue : hapTokenRes) {
        AccessTokenID tokenId = (AccessTokenID)tokenValue.GetInt(FIELD_TOKEN_ID);
        int ret = AccessTokenIDManager::GetInstance().RegisterTokenId(tokenId, TOKEN_HAP);
        if (ret != RET_SUCCESS) {
            ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: tokenId 0x%{public}x add id failed.",
                __func__, tokenId);
            continue;
        }
        std::shared_ptr<HapTokenInfoInner> hap = std::make_shared<HapTokenInfoInner>();
        if (hap == nullptr) {
            AccessTokenIDManager::GetInstance().ReleaseTokenId(tokenId);
            ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: tokenId 0x%{public}x alloc failed.", __func__, tokenId);
            continue;
        }
        ret = hap->RestoreHapTokenInfo(tokenId, tokenValue, permDefRes, permStateRes);
        if (ret != RET_SUCCESS) {
            AccessTokenIDManager::GetInstance().ReleaseTokenId(tokenId);
            ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: tokenId 0x%{public}x restore failed.", __func__, tokenId);
            continue;
        }

        ret = AddHapTokenInfo(hap);
        if (ret != RET_SUCCESS) {
            AccessTokenIDManager::GetInstance().ReleaseTokenId(tokenId);
            ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: tokenId 0x%{public}x add failed.", __func__, tokenId);
            continue;
        }
        ACCESSTOKEN_LOG_INFO(LABEL,
            "%{public}s:restore hap token 0x%{public}x bundle name %{public}s user %{public}d inst %{public}d ok!",
            __func__, tokenId, hap->GetBundleName().c_str(), hap->GetUserID(), hap->GetInstIndex());
    }
}

void AccessTokenInfoManager::InitNativeTokenInfos()
{
    std::vector<GenericValues> nativeTokenResults;
    DataStorage::GetRealDataStorage().Find(DataStorage::ACCESSTOKEN_NATIVE_INFO, nativeTokenResults);
    for (GenericValues nativeTokenValue : nativeTokenResults) {
        AccessTokenID tokenId = (AccessTokenID)nativeTokenValue.GetInt(FIELD_TOKEN_ID);
        int ret = AccessTokenIDManager::GetInstance().RegisterTokenId(tokenId, TOKEN_NATIVE);
        if (ret != RET_SUCCESS) {
            ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: tokenId 0x%{public}x add failed.", __func__, tokenId);
            continue;
        }
        std::shared_ptr<NativeTokenInfoInner> native = std::make_shared<NativeTokenInfoInner>();
        if (native == nullptr) {
            AccessTokenIDManager::GetInstance().ReleaseTokenId(tokenId);
            ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: tokenId 0x%{public}x alloc failed.", __func__, tokenId);
            continue;
        }

        ret = native->RestoreNativeTokenInfo(tokenId, nativeTokenValue);
        if (ret != RET_SUCCESS) {
            AccessTokenIDManager::GetInstance().ReleaseTokenId(tokenId);
            ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: tokenId 0x%{public}x restore failed.", __func__, tokenId);
            continue;
        }

        ret = AddNativeTokenInfo(native);
        if (ret != RET_SUCCESS) {
            AccessTokenIDManager::GetInstance().ReleaseTokenId(tokenId);
            ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: tokenId 0x%{public}x add failed.", __func__, tokenId);
            continue;
        }
        ACCESSTOKEN_LOG_INFO(LABEL,
            "%{public}s:restore native token 0x%{public}x process name %{public}s ok!",
            __func__, tokenId, native->GetProcessName().c_str());
    }
}

std::string AccessTokenInfoManager::GetHapUniqueStr(const int& userID,
    const std::string& bundleName, const int& instIndex) const
{
    return bundleName + "&" + std::to_string(userID) + "&" + std::to_string(instIndex);
}

std::string AccessTokenInfoManager::GetHapUniqueStr(const std::shared_ptr<HapTokenInfoInner>& info) const
{
    if (info == nullptr) {
        return std::string("");
    }
    return GetHapUniqueStr(info->GetUserID(), info->GetBundleName(), info->GetInstIndex());
}

int AccessTokenInfoManager::AddHapTokenInfo(const std::shared_ptr<HapTokenInfoInner>& info)
{
    if (info == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: token info is null.", __func__);
        return RET_FAILED;
    }
    AccessTokenID id = info->GetTokenID();

    {
        Utils::UniqueWriteGuard<Utils::RWLock> infoGuard(this->hapTokenInfoLock_);
        if (hapTokenInfoMap_.count(id) > 0) {
            ACCESSTOKEN_LOG_ERROR(
                LABEL, "%{public}s: token %{public}x info has exist.", __func__, id);
            return RET_FAILED;
        }

        std::string HapUniqueKey = GetHapUniqueStr(info);
        if (hapTokenIdMap_.count(HapUniqueKey) > 0) {
            ACCESSTOKEN_LOG_ERROR(
                LABEL, "%{public}s: token %{public}x Unique info has exist.", __func__, id);
            return RET_FAILED;
        }

        hapTokenInfoMap_[id] = info;
        hapTokenIdMap_[HapUniqueKey] = id;
    }
    std::shared_ptr<PermissionPolicySet> permPolicySet = info->GetHapInfoPermissionPolicySet();
    if (permPolicySet != nullptr) {
        PermissionManager::GetInstance().AddDefPermissions(permPolicySet->permList_);
    }
    return RET_SUCCESS;
}

int AccessTokenInfoManager::AddNativeTokenInfo(const std::shared_ptr<NativeTokenInfoInner>& info)
{
    if (info == nullptr) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s: token info is null.", __func__);
        return RET_FAILED;
    }

    AccessTokenID id = info->GetTokenID();
    Utils::UniqueWriteGuard<Utils::RWLock> infoGuard(this->nativeTokenInfoLock_);
    if (nativeTokenInfoMap_.count(id) > 0) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "%{public}s: token %{public}x has exist.", __func__, id);
        return RET_FAILED;
    }
    nativeTokenInfoMap_[id] = info;
    return RET_SUCCESS;
}

std::shared_ptr<HapTokenInfoInner> AccessTokenInfoManager::GetHapTokenInfoInner(AccessTokenID id)
{
    Utils::UniqueReadGuard<Utils::RWLock> infoGuard(this->hapTokenInfoLock_);
    if (hapTokenInfoMap_.count(id) == 0) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "%{public}s: token %{public}x is invalid.", __func__, id);
        return nullptr;
    }
    return hapTokenInfoMap_[id];
}

int AccessTokenInfoManager::GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo& InfoParcel)
{
    std::shared_ptr<HapTokenInfoInner> infoPtr = GetHapTokenInfoInner(tokenID);
    if (infoPtr == nullptr) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "%{public}s: token %{public}x is invalid.", __func__, tokenID);
        return RET_FAILED;
    }
    infoPtr->TranslateToHapTokenInfo(InfoParcel);
    return RET_SUCCESS;
}

std::shared_ptr<PermissionPolicySet> AccessTokenInfoManager::GetHapPermissionPolicySet(AccessTokenID id)
{
    std::shared_ptr<HapTokenInfoInner> infoPtr = GetHapTokenInfoInner(id);
    if (infoPtr == nullptr) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "%{public}s: token %{public}x is invalid.", __func__, id);
        return nullptr;
    }
    return infoPtr->GetHapInfoPermissionPolicySet();
}

std::shared_ptr<NativeTokenInfoInner> AccessTokenInfoManager::GetNativeTokenInfoInner(AccessTokenID id)
{
    Utils::UniqueReadGuard<Utils::RWLock> infoGuard(this->nativeTokenInfoLock_);
    if (nativeTokenInfoMap_.count(id) == 0) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "%{public}s: token %{public}x is invalid.", __func__, id);
        return nullptr;
    }
    return nativeTokenInfoMap_[id];
}

int AccessTokenInfoManager::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo& InfoParcel)
{
    std::shared_ptr<NativeTokenInfoInner> infoPtr = GetNativeTokenInfoInner(tokenID);
    if (infoPtr == nullptr) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "%{public}s: token %{public}x is invalid.", __func__, tokenID);
        return RET_FAILED;
    }

    infoPtr->TranslateToNativeTokenInfo(InfoParcel);
    return RET_SUCCESS;
}

int AccessTokenInfoManager::RemoveTokenInfo(AccessTokenID id)
{
    ATokenTypeEnum type = AccessTokenIDManager::GetInstance().GetTokenIdType(id);
    if (type == TOKEN_HAP) {
        // make sure that RemoveDefPermissions is called outside of the lock to avoid deadlocks.
        PermissionManager::GetInstance().RemoveDefPermissions(id);
        Utils::UniqueWriteGuard<Utils::RWLock> infoGuard(this->hapTokenInfoLock_);
        if (hapTokenInfoMap_.count(id) == 0) {
            ACCESSTOKEN_LOG_ERROR(
                LABEL, "%{public}s: hap token %{public}x is null.", __func__, id);
            return RET_FAILED;
        }

        const std::shared_ptr<HapTokenInfoInner> info = hapTokenInfoMap_[id];
        if (info == nullptr) {
            ACCESSTOKEN_LOG_ERROR(
                LABEL, "%{public}s: hap token %{public}x is null.", __func__, id);
            return RET_FAILED;
        }
        std::string HapUniqueKey = GetHapUniqueStr(info);
        if (hapTokenIdMap_.count(HapUniqueKey) != 0) {
            hapTokenIdMap_.erase(HapUniqueKey);
        }

        hapTokenInfoMap_.erase(id);
    } else if (type == TOKEN_NATIVE) {
        Utils::UniqueWriteGuard<Utils::RWLock> infoGuard(this->nativeTokenInfoLock_);
        if (nativeTokenInfoMap_.count(id) == 0) {
            ACCESSTOKEN_LOG_ERROR(
                LABEL, "%{public}s: native token %{public}x is null.", __func__, id);
            return RET_FAILED;
        }
        nativeTokenInfoMap_.erase(id);
    } else {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "%{public}s: token %{public}x unknown type.", __func__, id);
        return RET_FAILED;
    }

    AccessTokenIDManager::GetInstance().ReleaseTokenId(id);
    ACCESSTOKEN_LOG_INFO(LABEL, "%{public}s:remove hap token 0x%{public}x ok!", __func__, id);
    RefreshTokenInfoIfNeeded();
    return RET_SUCCESS;
}

int AccessTokenInfoManager::CreateHapTokenInfo(
    const HapInfoParams& info, const HapPolicyParams& policy, AccessTokenIDEx& tokenIdEx)
{
    if (!DataValidator::IsUserIdValid(info.userID) || !DataValidator::IsBundleNameValid(info.bundleName)
        || !DataValidator::IsAppIDDescValid(info.appIDDesc) || !DataValidator::IsDomainValid(policy.domain)) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s called, hap token param failed", __func__);
        return RET_FAILED;
    }

    AccessTokenID tokenId = AccessTokenIDManager::GetInstance().CreateAndRegisterTokenId(TOKEN_HAP);
    if (tokenId == 0) {
        ACCESSTOKEN_LOG_INFO(LABEL, "%{public}s called, token Id create failed", __func__);
        return RET_FAILED;
    }

    std::shared_ptr<HapTokenInfoInner> tokenInfo = std::make_shared<HapTokenInfoInner>();
    if (tokenInfo == nullptr) {
        AccessTokenIDManager::GetInstance().ReleaseTokenId(tokenId);
        ACCESSTOKEN_LOG_INFO(LABEL, "%{public}s called, alloc token info failed", __func__);
        return RET_FAILED;
    }
    tokenInfo->Init(tokenId, info, policy);

    int ret = AddHapTokenInfo(tokenInfo);
    if (ret != RET_SUCCESS) {
        ACCESSTOKEN_LOG_WARN(LABEL, "%{public}s called, %{public}s add token info failed",
            __func__, info.bundleName.c_str());
        AccessTokenIDManager::GetInstance().ReleaseTokenId(tokenId);
        return RET_FAILED;
    }
    ACCESSTOKEN_LOG_INFO(LABEL,
        "%{public}s:create hap token 0x%{public}x bundle name %{public}s user %{public}d inst %{public}d ok!",
        __func__, tokenId, tokenInfo->GetBundleName().c_str(), tokenInfo->GetUserID(), tokenInfo->GetInstIndex());

    tokenIdEx.tokenIdExStruct.tokenID = tokenId;
    tokenIdEx.tokenIdExStruct.tokenAttr = 0;
    RefreshTokenInfoIfNeeded();
    return RET_SUCCESS;
}

int AccessTokenInfoManager::CheckNativeDCap(AccessTokenID tokenID, const std::string& dcap)
{
    std::shared_ptr<NativeTokenInfoInner> infoPtr = GetNativeTokenInfoInner(tokenID);
    if (infoPtr == nullptr) {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "%{public}s: token %{public}x is invalid.", __func__, tokenID);
        return RET_FAILED;
    }

    std::vector<std::string> dcaps = infoPtr->GetDcap();
    for (auto iter = dcaps.begin(); iter != dcaps.end(); iter++) {
        if (*iter == dcap) {
            return RET_SUCCESS;
        }
    }
    return RET_FAILED;
}

AccessTokenID AccessTokenInfoManager::GetHapTokenID(int userID, const std::string& bundleName, int instIndex)
{
    Utils::UniqueReadGuard<Utils::RWLock> infoGuard(this->hapTokenInfoLock_);
    std::string HapUniqueKey = GetHapUniqueStr(userID, bundleName, instIndex);
    if (hapTokenIdMap_.count(HapUniqueKey) > 0) {
        return hapTokenIdMap_[HapUniqueKey];
    }
    return 0;
}

AccessTokenID AccessTokenInfoManager::AllocLocalTokenID(const std::string& remoteDeviceID,
    AccessTokenID remoteTokenID)
{
    return 0;
}

bool AccessTokenInfoManager::TryUpdateExistNativeToken(const std::shared_ptr<NativeTokenInfoInner>& infoPtr)
{
    if (infoPtr == nullptr) {
        ACCESSTOKEN_LOG_WARN(LABEL, "%{public}s called, info is null", __func__);
        return false;
    }

    Utils::UniqueWriteGuard<Utils::RWLock> infoGuard(this->nativeTokenInfoLock_);
    AccessTokenID id = infoPtr->GetTokenID();
    // if native token is exist, update it
    if (nativeTokenInfoMap_.count(id) == 0) {
        return false;
    }
    std::shared_ptr<NativeTokenInfoInner> oldTokenInfoPtr = nativeTokenInfoMap_[id];
    if (oldTokenInfoPtr != nullptr) {
        nativeTokenInfoMap_[id] = infoPtr;
    } else {
        ACCESSTOKEN_LOG_ERROR(
            LABEL, "%{public}s: native token exist, but is null.", __func__);
    }
    return true;
}

int AccessTokenInfoManager::AllocNativeToken(const std::shared_ptr<NativeTokenInfoInner>& infoPtr)
{
    if (infoPtr == nullptr) {
        ACCESSTOKEN_LOG_WARN(LABEL, "%{public}s called, token info is null", __func__);
        return RET_FAILED;
    }

    AccessTokenID id = infoPtr->GetTokenID();
    int ret = AccessTokenIDManager::GetInstance().RegisterTokenId(id, TOKEN_NATIVE);
    if (ret != RET_SUCCESS) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s called, token Id register fail", __func__);
        return RET_FAILED;
    }

    ret = AddNativeTokenInfo(infoPtr);
    if (ret != RET_SUCCESS) {
        ACCESSTOKEN_LOG_ERROR(LABEL, "%{public}s called, %{public}s add token info failed",
            __func__, infoPtr->GetProcessName().c_str());
        AccessTokenIDManager::GetInstance().ReleaseTokenId(id);
        return RET_FAILED;
    }

    return RET_SUCCESS;
}

void AccessTokenInfoManager::ProcessNativeTokenInfos(
    const std::vector<std::shared_ptr<NativeTokenInfoInner>>& tokenInfos)
{
    for (auto& infoPtr: tokenInfos) {
        if (infoPtr == nullptr) {
            ACCESSTOKEN_LOG_WARN(LABEL, "%{public}s called, token info from libat is null", __func__);
            continue;
        }
        bool isUpdated = TryUpdateExistNativeToken(infoPtr);
        if (!isUpdated) {
            ACCESSTOKEN_LOG_INFO(LABEL,
                "%{public}s: token 0x%{public}x process name %{public}s is new, add to manager!",
                __func__, infoPtr->GetTokenID(), infoPtr->GetProcessName().c_str());
            int ret = AllocNativeToken(infoPtr);
            if (ret != RET_SUCCESS) {
                ACCESSTOKEN_LOG_ERROR(LABEL,
                    "%{public}s: token 0x%{public}x process name %{public}s add to manager failed!",
                    __func__, infoPtr->GetTokenID(), infoPtr->GetProcessName().c_str());
            }
        }
    }
    RefreshTokenInfoIfNeeded();
}

int AccessTokenInfoManager::UpdateHapToken(AccessTokenID tokenID,
    const std::string& appIDDesc, const HapPolicyParams& policy)
{
    std::shared_ptr<HapTokenInfoInner> infoPtr = GetHapTokenInfoInner(tokenID);
    if (infoPtr == nullptr) {
        ACCESSTOKEN_LOG_INFO(LABEL, "%{public}s:token 0x%{public}x is null, can not update!", __func__, tokenID);
        return RET_FAILED;
    }

    Utils::UniqueWriteGuard<Utils::RWLock> infoGuard(this->hapTokenInfoLock_);
    infoPtr->Update(appIDDesc, policy);
    ACCESSTOKEN_LOG_INFO(LABEL,
        "%{public}s: token 0x%{public}x bundle name %{public}s user %{public}d inst %{public}d update ok!",
        __func__, tokenID, infoPtr->GetBundleName().c_str(), infoPtr->GetUserID(), infoPtr->GetInstIndex());

    RefreshTokenInfoIfNeeded();
    return RET_SUCCESS;
}

AccessTokenInfoManager& AccessTokenInfoManager::GetInstance()
{
    static AccessTokenInfoManager instance;
    return instance;
}

void AccessTokenInfoManager::StoreAllTokenInfo()
{
    std::vector<GenericValues> hapInfoValues;
    std::vector<GenericValues> permDefValues;
    std::vector<GenericValues> permStateValues;
    std::vector<GenericValues> nativeTokenValues;
    {
        Utils::UniqueReadGuard<Utils::RWLock> infoGuard(this->hapTokenInfoLock_);
        for (auto iter = hapTokenInfoMap_.begin(); iter != hapTokenInfoMap_.end(); iter++) {
            if (iter->second != nullptr) {
                iter->second->StoreHapInfo(hapInfoValues, permDefValues, permStateValues);
            }
        }
    }

    {
        Utils::UniqueReadGuard<Utils::RWLock> infoGuard(this->nativeTokenInfoLock_);
        for (auto iter = nativeTokenInfoMap_.begin(); iter != nativeTokenInfoMap_.end(); iter++) {
            if (iter->second != nullptr) {
                iter->second->StoreNativeInfo(nativeTokenValues);
            }
        }
    }

    DataStorage::GetRealDataStorage().RefreshAll(DataStorage::ACCESSTOKEN_HAP_INFO, hapInfoValues);
    DataStorage::GetRealDataStorage().RefreshAll(DataStorage::ACCESSTOKEN_NATIVE_INFO, nativeTokenValues);
    DataStorage::GetRealDataStorage().RefreshAll(DataStorage::ACCESSTOKEN_PERMISSION_DEF, permDefValues);
    DataStorage::GetRealDataStorage().RefreshAll(DataStorage::ACCESSTOKEN_PERMISSION_STATE, permStateValues);
}

void AccessTokenInfoManager::RefreshTokenInfoIfNeeded()
{
    if (tokenDataWorker_.GetCurTaskNum() > 1) {
        ACCESSTOKEN_LOG_INFO(LABEL, "%{public}s: has refresh task!", __func__);
        return;
    }
    tokenDataWorker_.AddTask([]() {
        AccessTokenInfoManager::GetInstance().StoreAllTokenInfo();

        // Sleep for one second to avoid frequent refresh of the database.
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
}

void AccessTokenInfoManager::Dump(std::string& dumpInfo)
{
    Utils::UniqueReadGuard<Utils::RWLock> hapInfoGuard(this->hapTokenInfoLock_);
    for (auto iter = hapTokenInfoMap_.begin(); iter != hapTokenInfoMap_.end(); iter++) {
        if (iter->second != nullptr) {
            iter->second->ToString(dumpInfo);
        }
    }

    Utils::UniqueReadGuard<Utils::RWLock> nativeInfoGuard(this->nativeTokenInfoLock_);
    for (auto iter = nativeTokenInfoMap_.begin(); iter != nativeTokenInfoMap_.end(); iter++) {
        if (iter->second != nullptr) {
            iter->second->ToString(dumpInfo);
        }
    }
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS

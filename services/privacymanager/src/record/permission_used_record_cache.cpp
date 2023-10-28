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

#include "permission_used_record_cache.h"
#include "accesstoken_log.h"
#include "constant.h"
#include "generic_values.h"
#include "permission_record.h"
#include "permission_record_manager.h"
#include "permission_record_node.h"
#include "permission_record_repository.h"
#include "permission_used_record_db.h"
#include "privacy_field_const.h"
#include "time_util.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PRIVACY, "PermissionUsedRecordCache"
};
}

PermissionUsedRecordCache& PermissionUsedRecordCache::GetInstance()
{
    static PermissionUsedRecordCache instance;
    return instance;
}

bool PermissionUsedRecordCache::RecordMergeCheck(const PermissionRecord& record1, const PermissionRecord& record2)
{
    // timestamp in the same minute
    if (!TimeUtil::IsTimeStampsSameMinute(record1.timestamp, record2.timestamp)) {
        return false;
    }

    // the same tokenID + opCode + status + lockScreenStatus
    if ((record1.tokenId != record2.tokenId) ||
        (record1.opCode != record2.opCode) ||
        (record1.status != record2.status) ||
        (record1.lockScreenStatus != record2.lockScreenStatus)) {
        return false;
    }

    // both success
    if (((record1.accessCount > 0) && (record2.accessCount == 0)) ||
        ((record1.accessCount == 0) && (record2.accessCount > 0))) {
        return false;
    }

    // both failure
    if (((record1.rejectCount > 0) && (record2.rejectCount == 0)) ||
        ((record1.rejectCount == 0) && (record2.rejectCount > 0))) {
        return false;
    }

    return true;
}

void PermissionUsedRecordCache::AddRecordToBuffer(const PermissionRecord& record)
{
    if (nextPersistTimestamp_ == 0) {
        nextPersistTimestamp_ = record.timestamp + INTERVAL;
    }
    std::shared_ptr<PermissionUsedRecordNode> curFindMergePos;
    std::shared_ptr<PermissionUsedRecordNode> persistPendingBufferHead;
    std::shared_ptr<PermissionUsedRecordNode> persistPendingBufferEnd = nullptr;
    PermissionRecord mergedRecord = record;
    {
        Utils::UniqueWriteGuard<Utils::RWLock> lock1(this->cacheLock1_);
        curFindMergePos = curRecordBufferPos_;
        persistPendingBufferHead = recordBufferHead_;
        int32_t remainCount = 0;
        while (curFindMergePos != recordBufferHead_) {
            auto pre = curFindMergePos->pre.lock();
            if ((record.timestamp - curFindMergePos->record.timestamp) >= INTERVAL) {
                persistPendingBufferEnd = curFindMergePos;
                break;
            } else if (RecordMergeCheck(curFindMergePos->record, record)) {
                MergeRecord(mergedRecord, curFindMergePos);
            } else {
                remainCount++;
            }
            curFindMergePos = pre;
        }
        AddRecordNode(mergedRecord); // refresh curRecordBUfferPos and readableSize
        remainCount++;
        if ((remainCount >= MAX_PERSIST_SIZE) || (persistPendingBufferEnd != nullptr)) {
            ResetRecordBufferWhenAdd(remainCount, persistPendingBufferEnd);
        }
    }
    if (persistPendingBufferEnd != nullptr) {
        AddToPersistQueue(persistPendingBufferHead);
    }
}

void PermissionUsedRecordCache::MergeRecord(PermissionRecord& record,
    std::shared_ptr<PermissionUsedRecordNode> curFindMergePos)
{
    record.accessDuration += curFindMergePos->record.accessDuration;
    record.accessCount += curFindMergePos->record.accessCount;
    record.rejectCount += curFindMergePos->record.rejectCount;
    if (curRecordBufferPos_ == curFindMergePos) {
        curRecordBufferPos_ = curRecordBufferPos_->pre.lock();
    }
    DeleteRecordNode(curFindMergePos); // delete old same node
    readableSize_--;
}

void PermissionUsedRecordCache::AddToPersistQueue(
    const std::shared_ptr<PermissionUsedRecordNode> persistPendingBufferHead)
{
    bool startPersist = false;
    {
        Utils::UniqueWriteGuard<Utils::RWLock> lock2(this->cacheLock2_);
        persistPendingBufferQueue_.emplace_back(persistPendingBufferHead);
        if ((TimeUtil::GetCurrentTimestamp() >= nextPersistTimestamp_ ||
            readableSize_ >= MAX_PERSIST_SIZE) && !persistIsRunning_) {
            startPersist = true;
        }
    }
    if (startPersist) {
        ExecuteReadRecordBufferTask();
    }
}

void PermissionUsedRecordCache::ExecuteReadRecordBufferTask()
{
    if (readRecordBufferTaskWorker_.GetCurTaskNum() > 1) {
        ACCESSTOKEN_LOG_INFO(LABEL, "Already has read record buffer task!");
        return;
    }
    auto readRecordBufferTask = [this]() {
        ACCESSTOKEN_LOG_INFO(LABEL, "ReadRecordBuffer task called");
        PersistPendingRecords();
    };
    readRecordBufferTaskWorker_.AddTask(readRecordBufferTask);
}

int32_t PermissionUsedRecordCache::PersistPendingRecords()
{
    std::shared_ptr<PermissionUsedRecordNode> persistPendingBufferHead;
    bool isEmpty;
    std::vector<GenericValues> insertValues;
    {
        Utils::UniqueWriteGuard<Utils::RWLock> lock2(this->cacheLock2_);
        isEmpty = persistPendingBufferQueue_.empty();
        persistIsRunning_ = true;
        nextPersistTimestamp_ = TimeUtil::GetCurrentTimestamp() + INTERVAL;
    }
    while (!isEmpty) {
        {
            Utils::UniqueWriteGuard<Utils::RWLock> lock2(this->cacheLock2_);
            persistPendingBufferHead = persistPendingBufferQueue_[0];
            persistPendingBufferQueue_.erase(persistPendingBufferQueue_.begin());
        }
        std::shared_ptr<PermissionUsedRecordNode> curPendingRecordNode =
            persistPendingBufferHead->next;
        while (curPendingRecordNode != nullptr) {
            auto next = curPendingRecordNode->next;
            GenericValues tmpRecordValues;
            PermissionRecord tmpRecord = curPendingRecordNode->record;
            PermissionRecord::TranslationIntoGenericValues(tmpRecord, tmpRecordValues);
            insertValues.emplace_back(tmpRecordValues);
            DeleteRecordNode(curPendingRecordNode);
            curPendingRecordNode = next;
        }
        if (!insertValues.empty() && !PermissionRecordRepository::GetInstance().AddRecordValues(insertValues)) {
            ACCESSTOKEN_LOG_ERROR(LABEL, "Failed to persist pending records");
        }
        {
            Utils::UniqueReadGuard<Utils::RWLock> lock2(this->cacheLock2_);
            isEmpty = persistPendingBufferQueue_.empty();
        }
    }
    {
        Utils::UniqueWriteGuard<Utils::RWLock> lock2(this->cacheLock2_);
        if (isEmpty) { // free persistPendingBufferQueue
            std::vector<std::shared_ptr<PermissionUsedRecordNode>> tmpPersistPendingBufferQueue;
            std::swap(tmpPersistPendingBufferQueue, persistPendingBufferQueue_);
        }
        persistIsRunning_ = false;
    }
    return true;
}

#ifdef POWER_MANAGER_ENABLE
void PermissionUsedRecordCache::PersistPendingRecordsImmediately()
{
    // this function can be use only when receive shut down callback
    {
        Utils::UniqueWriteGuard<Utils::RWLock> lock2(this->cacheLock2_);
        persistPendingBufferQueue_.emplace_back(recordBufferHead_);
    }

    PersistPendingRecords();
}
#endif

int32_t PermissionUsedRecordCache::RemoveRecords(const AccessTokenID tokenId)
{
    std::shared_ptr<PermissionUsedRecordNode> curFindDeletePos;
    std::shared_ptr<PermissionUsedRecordNode> persistPendingBufferHead;
    std::shared_ptr<PermissionUsedRecordNode> persistPendingBufferEnd = nullptr;

    {
        int32_t countPersistPendingNode = 0;
        Utils::UniqueWriteGuard<Utils::RWLock> lock1(this->cacheLock1_);
        curFindDeletePos = recordBufferHead_->next;
        persistPendingBufferHead = recordBufferHead_;
        while (curFindDeletePos != nullptr) {
            auto next = curFindDeletePos->next;
            if (curFindDeletePos->record.tokenId == tokenId) {
                if (curRecordBufferPos_ == curFindDeletePos) {
                    curRecordBufferPos_ = curFindDeletePos->pre.lock();
                }
                DeleteRecordNode(curFindDeletePos);
                readableSize_--;
            } else if (TimeUtil::GetCurrentTimestamp() -
                curFindDeletePos->record.timestamp >= INTERVAL) {
                persistPendingBufferEnd = curFindDeletePos;
                countPersistPendingNode++;
            }
            curFindDeletePos = next;
        }
        if (countPersistPendingNode != 0) { // refresh recordBufferHead
            int32_t remainCount = readableSize_ - countPersistPendingNode;
            ResetRecordBuffer(remainCount, persistPendingBufferEnd);
        }
    }
    RemoveFromPersistQueueAndDatabase(tokenId);
    if (persistPendingBufferEnd != nullptr) { // add to queue
        AddToPersistQueue(persistPendingBufferHead);
    }
    return Constant::SUCCESS;
}

void PermissionUsedRecordCache::RemoveFromPersistQueueAndDatabase(const AccessTokenID tokenId)
{
    {
        std::shared_ptr<PermissionUsedRecordNode> curFindDeletePos;
        Utils::UniqueWriteGuard<Utils::RWLock> lock2(this->cacheLock2_);
        if (!persistPendingBufferQueue_.empty()) {
            for (const auto& persistHead : persistPendingBufferQueue_) {
                curFindDeletePos = persistHead->next;
                while (curFindDeletePos != nullptr) {
                    auto next = curFindDeletePos->next;
                    if (curFindDeletePos->record.tokenId == tokenId) {
                        DeleteRecordNode(curFindDeletePos);
                    }
                    curFindDeletePos = next;
                }
            }
        }
    }
    GenericValues record;
    record.Put(PrivacyFiledConst::FIELD_TOKEN_ID, static_cast<int32_t>(tokenId));
    PermissionRecordRepository::GetInstance().RemoveRecordValues(record); // remove from database
}

void PermissionUsedRecordCache::GetRecords(const std::vector<std::string>& permissionList,
    const GenericValues& andConditionValues, std::vector<GenericValues>& findRecordsValues, int32_t cache1QueryCount)
{
    std::set<int32_t> opCodeList;
    std::shared_ptr<PermissionUsedRecordNode> curFindPos;
    std::shared_ptr<PermissionUsedRecordNode> persistPendingBufferHead;
    std::shared_ptr<PermissionUsedRecordNode> persistPendingBufferEnd = nullptr;
    int32_t countPersistPendingNode = 0;
    AccessTokenID tokenId = andConditionValues.GetInt(PrivacyFiledConst::FIELD_TOKEN_ID);
    TransferToOpcode(opCodeList, permissionList);
    {
        Utils::UniqueWriteGuard<Utils::RWLock> lock1(this->cacheLock1_);
        curFindPos = recordBufferHead_->next;
        persistPendingBufferHead = recordBufferHead_;
        while (curFindPos != nullptr) {
            if (cache1QueryCount == 0) {
                break;
            }
            auto next = curFindPos->next;
            if (RecordCompare(tokenId, opCodeList, andConditionValues, curFindPos->record)) {
                GenericValues recordValues;
                PermissionRecord::TranslationIntoGenericValues(curFindPos->record, recordValues);
                findRecordsValues.emplace_back(recordValues);
                cache1QueryCount--;
            }
            if (TimeUtil::GetCurrentTimestamp() - curFindPos->record.timestamp >= INTERVAL) {
                persistPendingBufferEnd = curFindPos;
                countPersistPendingNode++;
            }
            curFindPos = next;
        }
        if (countPersistPendingNode != 0) { // refresh recordBufferHead
            int32_t remainCount = readableSize_ - countPersistPendingNode;
            ResetRecordBuffer(remainCount, persistPendingBufferEnd);
        }
    }
    if (cache1QueryCount > 0) {
        GetFromPersistQueueAndDatabase(opCodeList, andConditionValues, findRecordsValues, cache1QueryCount);
    }
    if (countPersistPendingNode != 0) {
        AddToPersistQueue(persistPendingBufferHead);
    }
}

void PermissionUsedRecordCache::GetFromPersistQueueAndDatabase(const std::set<int32_t>& opCodeList,
    const GenericValues& andConditionValues, std::vector<GenericValues>& findRecordsValues, int32_t cache2QueryCount)
{
    AccessTokenID tokenId = andConditionValues.GetInt(PrivacyFiledConst::FIELD_TOKEN_ID);
    std::shared_ptr<PermissionUsedRecordNode> curFindPos;
    {
        Utils::UniqueReadGuard<Utils::RWLock> lock2(this->cacheLock2_);
        for (const auto& persistHead : persistPendingBufferQueue_) {
            curFindPos = persistHead->next;
            while (curFindPos != nullptr) {
                auto next = curFindPos->next;
                if (RecordCompare(tokenId, opCodeList, andConditionValues, curFindPos->record)) {
                    GenericValues recordValues;
                    PermissionRecord::TranslationIntoGenericValues(curFindPos->record, recordValues);
                    if (cache2QueryCount == 0) {
                        break;
                    }
                    findRecordsValues.emplace_back(recordValues);
                    cache2QueryCount--;
                }
                curFindPos = next;
            }
            if (cache2QueryCount == 0) {
                return;
            }
        }
    }

    if (!PermissionRecordRepository::GetInstance().FindRecordValues(opCodeList, andConditionValues,
        findRecordsValues, cache2QueryCount)) { // find records from database
        ACCESSTOKEN_LOG_ERROR(LABEL, "find records from database failed");
    }
}

void PermissionUsedRecordCache::ResetRecordBufferWhenAdd(const int32_t remainCount,
    std::shared_ptr<PermissionUsedRecordNode>& persistPendingBufferEnd)
{
    std::shared_ptr<PermissionUsedRecordNode> tmpRecordBufferHead =
        std::make_shared<PermissionUsedRecordNode>();
    if (remainCount >= MAX_PERSIST_SIZE) {
        readableSize_ = 1;
        tmpRecordBufferHead->next = curRecordBufferPos_;
        persistPendingBufferEnd = curRecordBufferPos_->pre.lock();
        persistPendingBufferEnd->next.reset(); //release the last node next
        recordBufferHead_ = tmpRecordBufferHead;
        recordBufferHead_->next->pre = recordBufferHead_;
        return;
    }
    readableSize_ = remainCount;
    // refresh recordBufferHead
    tmpRecordBufferHead->next = persistPendingBufferEnd->next;
    persistPendingBufferEnd->next.reset(); //release persistPendingBufferEnd->next
    recordBufferHead_ = tmpRecordBufferHead;
    // recordBufferHead_->next->pre equals to persistPendingBufferEnd, reset recordBufferHead_->next->pre
    recordBufferHead_->next->pre = recordBufferHead_;
}

void PermissionUsedRecordCache::ResetRecordBuffer(const int32_t remainCount,
    std::shared_ptr<PermissionUsedRecordNode>& persistPendingBufferEnd)
{
    readableSize_ = remainCount;
    // refresh recordBufferHead
    std::shared_ptr<PermissionUsedRecordNode> tmpRecordBufferHead =
        std::make_shared<PermissionUsedRecordNode>();
    tmpRecordBufferHead->next = persistPendingBufferEnd->next;
    persistPendingBufferEnd->next.reset();
    recordBufferHead_ = tmpRecordBufferHead;

    if (persistPendingBufferEnd == curRecordBufferPos_) {
        // persistPendingBufferEnd == curRecordBufferPos, reset curRecordBufferPos
        curRecordBufferPos_ = recordBufferHead_;
    } else {
        // recordBufferHead_->next->pre = persistPendingBufferEnd, reset recordBufferHead_->next->pre
        recordBufferHead_->next->pre = recordBufferHead_;
    }
}

void PermissionUsedRecordCache::TransferToOpcode(std::set<int32_t>& opCodeList,
    const std::vector<std::string>& permissionList)
{
    for (const auto& permission : permissionList) {
        int32_t opCode = Constant::OP_INVALID;
        Constant::TransferPermissionToOpcode(permission, opCode);
        opCodeList.insert(opCode);
    }
}

bool PermissionUsedRecordCache::RecordCompare(const AccessTokenID tokenId, const std::set<int32_t>& opCodeList,
    const GenericValues& andConditionValues, const PermissionRecord& record)
{
    // compare tokenId
    if (record.tokenId != tokenId) {
        return false;
    }
    // compare opCode
    if (!opCodeList.empty() && opCodeList.find(record.opCode) == opCodeList.end()) {
        return false;
    }
    // compare timestamp
    std::vector<std::string> andColumns = andConditionValues.GetAllKeys();
    if (!andColumns.empty()) {
        for (auto andColumn : andColumns) {
            if (andColumn == PrivacyFiledConst::FIELD_TIMESTAMP_BEGIN &&
                record.timestamp < andConditionValues.GetInt64(andColumn)) {
                return false;
            } else if (andColumn == PrivacyFiledConst::FIELD_TIMESTAMP_END &&
                record.timestamp > andConditionValues.GetInt64(andColumn)) {
                return false;
            } else if (andColumn == PrivacyFiledConst::FIELD_TIMESTAMP &&
                record.timestamp != andConditionValues.GetInt64(andColumn)) {
                return false;
            }
        }
    }
    return true;
}

void PermissionUsedRecordCache::FindTokenIdList(std::set<AccessTokenID>& tokenIdList)
{
    std::shared_ptr<PermissionUsedRecordNode> curFindPos;
    {
        // find tokenIdList from recordBuffer
        Utils::UniqueReadGuard<Utils::RWLock> lock1(this->cacheLock1_);
        curFindPos = recordBufferHead_->next;
        while (curFindPos != nullptr) {
            auto next = curFindPos->next;
            tokenIdList.emplace((AccessTokenID)curFindPos->record.tokenId);
            curFindPos = next;
        }
    }
    {
        // find tokenIdList from BufferQueue
        Utils::UniqueReadGuard<Utils::RWLock> lock2(this->cacheLock2_);
        if (!persistPendingBufferQueue_.empty()) {
            for (auto persistHead : persistPendingBufferQueue_) {
                curFindPos = persistHead->next;
                while (curFindPos != nullptr) {
                    auto next = curFindPos->next;
                    tokenIdList.emplace((AccessTokenID)curFindPos->record.tokenId);
                    curFindPos = next;
                }
            }
        }
    }
}

void PermissionUsedRecordCache::AddRecordNode(const PermissionRecord& record)
{
    std::shared_ptr<PermissionUsedRecordNode> tmpRecordNode = std::make_shared<PermissionUsedRecordNode>();
    tmpRecordNode->record = record;
    tmpRecordNode->pre = curRecordBufferPos_;
    curRecordBufferPos_->next = tmpRecordNode;
    curRecordBufferPos_ = curRecordBufferPos_->next;
    readableSize_++;
}

void PermissionUsedRecordCache::DeleteRecordNode(std::shared_ptr<PermissionUsedRecordNode> deleteRecordNode)
{
    std::shared_ptr<PermissionUsedRecordNode> pre = deleteRecordNode->pre.lock();
    if (deleteRecordNode->next == nullptr) { // End of the linked list
        pre->next = nullptr;
    } else {
        std::shared_ptr<PermissionUsedRecordNode> next = deleteRecordNode->next;
        pre->next = next;
        next->pre = pre;
    }
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "sqlite_helper.h"

#include "accesstoken_common_log.h"
#include "sqlite3ext.h"
#include <sys/types.h>

namespace OHOS {
namespace Security {
namespace AccessToken {

SqliteHelper::SqliteHelper(const std::string& dbName, const std::string& dbPath, int32_t version)
    : dbName_(dbName), dbPath_(dbPath), currentVersion_(version), db_(nullptr)
{}

SqliteHelper::~SqliteHelper()
{}

void SqliteHelper::Open() __attribute__((no_sanitize("cfi")))
{
    if (db_ != nullptr) {
        LOGW(ATM_DOMAIN, ATM_TAG, "Db s already open");
        return;
    }
    if (dbName_.empty() || dbPath_.empty() || currentVersion_ < 0) {
        LOGE(ATM_DOMAIN, ATM_TAG, "Param invalid, dbName: %{public}s, dbPath: %{public}s, currentVersion: %{public}d",
            dbName_.c_str(), dbPath_.c_str(), currentVersion_);
        return;
    }
    // set soft heap limit as 10KB
    const int32_t heapLimit = 10 * 1024;
    sqlite3_soft_heap_limit64(heapLimit);
    std::string fileName = dbPath_ + dbName_;
    int32_t res = sqlite3_open(fileName.c_str(), &db_);
    if (res != SQLITE_OK) {
        LOGE(ATM_DOMAIN, ATM_TAG, "Failed to open db: %{public}s", sqlite3_errmsg(db_));
        return;
    }

    SetWal();

    int32_t version = GetVersion();
    if (version == currentVersion_) {
        return;
    }

    BeginTransaction();
    if (version == DataBaseVersion::VERISION_0) {
        OnCreate();
    } else {
        if (version < currentVersion_) {
            OnUpdate(version);
        }
    }
    SetVersion();
    CommitTransaction();
}

void SqliteHelper::Close()
{
    if (db_ == nullptr) {
        LOGW(ATM_DOMAIN, ATM_TAG, "Do open data base first!");
        return;
    }
    int32_t ret = sqlite3_close(db_);
    if (ret != SQLITE_OK) {
        LOGW(ATM_DOMAIN, ATM_TAG, "Sqlite3_close error, ret=%{public}d", ret);
        return;
    }
    db_ = nullptr;
}

int32_t SqliteHelper::BeginTransaction() const
{
    if (db_ == nullptr) {
        LOGW(ATM_DOMAIN, ATM_TAG, "Do open data base first!");
        return GENERAL_ERROR;
    }
    char* errorMessage = nullptr;
    int32_t result = 0;
    int32_t ret = sqlite3_exec(db_, "BEGIN;", nullptr, nullptr, &errorMessage);
    if (ret != SQLITE_OK) {
        LOGE(ATM_DOMAIN, ATM_TAG, "Failed, errorMsg: %{public}s", errorMessage);
        result = GENERAL_ERROR;
    }
    sqlite3_free(errorMessage);
    return result;
}

int32_t SqliteHelper::CommitTransaction() const
{
    if (db_ == nullptr) {
        LOGW(ATM_DOMAIN, ATM_TAG, "Do open data base first!");
        return GENERAL_ERROR;
    }
    char* errorMessage = nullptr;
    int32_t result = 0;
    int32_t ret = sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, &errorMessage);
    if (ret != SQLITE_OK) {
        LOGE(ATM_DOMAIN, ATM_TAG, "Failed, errorMsg: %{public}s", errorMessage);
        result = GENERAL_ERROR;
    }
    sqlite3_free(errorMessage);
    sqlite3_db_cacheflush(db_);
    return result;
}

int32_t SqliteHelper::RollbackTransaction() const
{
    if (db_ == nullptr) {
        LOGW(ATM_DOMAIN, ATM_TAG, "Do open data base first!");
        return GENERAL_ERROR;
    }
    int32_t result = 0;
    char* errorMessage = nullptr;
    int32_t ret = sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, &errorMessage);
    if (ret != SQLITE_OK) {
        LOGE(ATM_DOMAIN, ATM_TAG, "Failed, errorMsg: %{public}s", errorMessage);
        result = GENERAL_ERROR;
    }
    sqlite3_free(errorMessage);
    return result;
}

Statement SqliteHelper::Prepare(const std::string& sql) const
{
    return Statement(db_, sql);
}

int32_t SqliteHelper::ExecuteSql(const std::string& sql) const
{
    if (db_ == nullptr) {
        LOGW(ATM_DOMAIN, ATM_TAG, "Do open data base first!");
        return GENERAL_ERROR;
    }
    char* errorMessage = nullptr;
    int32_t result = 0;
    int32_t res = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errorMessage);
    if (res != SQLITE_OK) {
        LOGE(ATM_DOMAIN, ATM_TAG, "Failed, errorMsg: %{public}s", errorMessage);
        result = GENERAL_ERROR;
    }
    sqlite3_free(errorMessage);
    return result;
}

void SqliteHelper::SetWal() const
{
    if (db_ == nullptr) {
        LOGW(ATM_DOMAIN, ATM_TAG, "Do open data base first!");
        return;
    }
    auto statement = Prepare(PRAGMA_WAL_COMMAND);
    if (statement.Step() != Statement::State::DONE) {
        LOGE(ATM_DOMAIN, ATM_TAG, "Set wal mode failed, errorMsg: %{public}s", SpitError().c_str());
    } else {
        LOGI(ATM_DOMAIN, ATM_TAG, "Set wal mode success!");
    }
}

int32_t SqliteHelper::GetVersion() const __attribute__((no_sanitize("cfi")))
{
    if (db_ == nullptr) {
        LOGW(ATM_DOMAIN, ATM_TAG, "Do open data base first!");
        return GENERAL_ERROR;
    }
    auto statement = Prepare(PRAGMA_VERSION_COMMAND);
    int32_t version = 0;
    while (statement.Step() == Statement::State::ROW) {
        version = statement.GetColumnInt(0);
    }
    LOGI(ATM_DOMAIN, ATM_TAG, "Version: %{public}d", version);
    return version;
}

void SqliteHelper::SetVersion() const
{
    if (db_ == nullptr) {
        LOGW(ATM_DOMAIN, ATM_TAG, "Do open data base first!");
        return;
    }
    auto statement = Prepare(PRAGMA_VERSION_COMMAND + " = " + std::to_string(currentVersion_));
    statement.Step();
}

std::string SqliteHelper::SpitError() const
{
    if (db_ == nullptr) {
        LOGW(ATM_DOMAIN, ATM_TAG, "Do open data base first!");
        return "";
    }
    return sqlite3_errmsg(db_);
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS

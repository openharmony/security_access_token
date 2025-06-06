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

#include "time_util.h"
#include <sys/time.h>

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
static constexpr int64_t ONE_SECOND_MILLISECONDS = 1000;
}
int64_t TimeUtil::GetCurrentTimestamp()
{
    struct timeval t;
    gettimeofday(&t, nullptr);
    int64_t timestamp = t.tv_sec * ONE_SECOND_MILLISECONDS + t.tv_usec / ONE_SECOND_MILLISECONDS;

    return timestamp;
}

bool TimeUtil::IsTimeStampsSameMinute(int64_t timeStamp1, int64_t timeStamp2)
{
    struct tm t1 = {0};
    time_t time1 = static_cast<time_t>(timeStamp1 / 1000);
    // localtime is not thread safe, localtime_r first param unit is second, timestamp unit is ms, so divided by 1000
    localtime_r(&time1, &t1);
    struct tm t2 = {0};
    time_t time2 = static_cast<time_t>(timeStamp2 / 1000);
    localtime_r(&time2, &t2);

    return t1.tm_min == t2.tm_min;
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS

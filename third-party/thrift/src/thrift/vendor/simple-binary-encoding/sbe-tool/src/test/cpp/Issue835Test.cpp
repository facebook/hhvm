/*
 * Copyright 2013-2024 Real Logic Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gtest/gtest.h"
#include "simple/MDUpdateAction.h"
#include "simple/MDEntryTypeBook.h"
#include "simple/MDIncrementalRefreshOrderBook47.h"

static const std::size_t BUFFER_LEN = 2048u;

using namespace simple;

class Issue835Test : public testing::Test
{
};

TEST_F(Issue835Test, shouldCompileWithSkipper)
{
    const std::uint64_t timeNs = 777;
    const std::size_t offset = 0;
    char buffer[BUFFER_LEN] = {};
    MDIncrementalRefreshOrderBook47 encoder;

    encoder
        .wrapForEncode(buffer, offset, BUFFER_LEN)
        .transactTime(timeNs)
        .matchEventIndicator()
            .lastQuoteMsg(true)
            .recoveryMsg(false)
            .reserved(true);

    MDIncrementalRefreshOrderBook47::NoMDEntries &entries = encoder.noMDEntriesCount(2);

    entries.next();
    entries.orderID(1);
    entries.mDOrderPriority(2);
    entries.mDEntryPx().mantissa(77);
    entries.securityID(33);
    entries.mDUpdateAction(MDUpdateAction::Value::New);
    entries.mDEntryType(MDEntryTypeBook::Value::Bid);

    entries.next();
    entries.orderID(2);
    entries.mDOrderPriority(3);
    entries.mDEntryPx().mantissa(88);
    entries.securityID(44);
    entries.mDUpdateAction(MDUpdateAction::Value::New);
    entries.mDEntryType(MDEntryTypeBook::Value::Offer);

    MDIncrementalRefreshOrderBook47 decoder;
    decoder.wrapForDecode(
        buffer,
        offset,
        MDIncrementalRefreshOrderBook47::sbeBlockLength(),
        MDIncrementalRefreshOrderBook47::sbeSchemaVersion(),
        BUFFER_LEN);

    EXPECT_EQ(decoder.decodeLength(), encoder.encodedLength());

    MDIncrementalRefreshOrderBook47 decoder2;
    decoder2.wrapForDecode(
        buffer,
        offset,
        MDIncrementalRefreshOrderBook47::sbeBlockLength(),
        MDIncrementalRefreshOrderBook47::sbeSchemaVersion(),
        BUFFER_LEN);

    decoder2.skip();
    EXPECT_EQ(decoder2.encodedLength(), encoder.encodedLength());
}

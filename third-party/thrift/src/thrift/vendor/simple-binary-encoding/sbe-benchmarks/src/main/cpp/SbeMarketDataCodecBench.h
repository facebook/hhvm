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
#ifndef _SBE_MARKET_DATA_CODEC_BENCH_HPP
#define _SBE_MARKET_DATA_CODEC_BENCH_HPP

#include "CodecBench.h"
#include "uk_co_real_logic_sbe_benchmarks_fix/MessageHeader.h"
#include "uk_co_real_logic_sbe_benchmarks_fix/MarketDataIncrementalRefreshTrades.h"

using namespace uk::co::real_logic::sbe::benchmarks::fix;

class SbeMarketDataCodecBench : public CodecBench<SbeMarketDataCodecBench>
{
public:
    virtual std::uint64_t encode(char *buffer, const std::uint64_t bufferLength)
    {
        messageHeader_.wrap(buffer, 0, 0, bufferLength);
        messageHeader_.blockLength(marketData_.sbeBlockLength());
        messageHeader_.templateId(marketData_.sbeTemplateId());
        messageHeader_.schemaId(marketData_.sbeSchemaId());
        messageHeader_.version(marketData_.sbeSchemaVersion());

        marketData_.wrapForEncode(buffer + messageHeader_.encodedLength(), 0, bufferLength);
        marketData_.transactTime(1234L);
        marketData_.eventTimeDelta(987);
        marketData_.matchEventIndicator(MatchEventIndicator::END_EVENT);

        MarketDataIncrementalRefreshTrades::MdIncGrp &mdIncGrp = marketData_.mdIncGrpCount(2);

        mdIncGrp.next();
        mdIncGrp.tradeId(1234L);
        mdIncGrp.securityId(56789L);
        mdIncGrp.mdEntryPx().mantissa(50);
        mdIncGrp.mdEntrySize().mantissa(10);
        mdIncGrp.numberOfOrders(1);
        mdIncGrp.mdUpdateAction(MDUpdateAction::NEW);
        mdIncGrp.rptSeq((short)1);

        mdIncGrp.next();
        mdIncGrp.tradeId(1234L);
        mdIncGrp.securityId(56789L);
        mdIncGrp.mdEntryPx().mantissa(50);
        mdIncGrp.mdEntrySize().mantissa(10);
        mdIncGrp.numberOfOrders(1);
        mdIncGrp.mdUpdateAction(MDUpdateAction::NEW);
        mdIncGrp.rptSeq((short)1);

        return MessageHeader::encodedLength() + marketData_.encodedLength();
    };

    virtual std::uint64_t decode(const char *buffer, const std::uint64_t bufferLength)
    {
        int64_t actingVersion;
        int64_t actingBlockLength;

        messageHeader_.wrap((char *)buffer, 0, 0, bufferLength);

        actingBlockLength = messageHeader_.blockLength();
        actingVersion = messageHeader_.version();


        marketData_.wrapForDecode((char *)buffer, messageHeader_.encodedLength(), actingBlockLength, actingVersion, bufferLength);

        static_cast<void>(marketData_.transactTime());
        static_cast<void>(marketData_.eventTimeDelta());
        static_cast<void>(marketData_.matchEventIndicator());

        MarketDataIncrementalRefreshTrades::MdIncGrp &mdIncGrp = marketData_.mdIncGrp();
        while (mdIncGrp.hasNext())
        {
            mdIncGrp.next();
            static_cast<void>(mdIncGrp.tradeId());
            static_cast<void>(mdIncGrp.securityId());
            static_cast<void>(mdIncGrp.mdEntryPx().mantissa());
            static_cast<void>(mdIncGrp.mdEntrySize().mantissa());
            static_cast<void>(mdIncGrp.numberOfOrders());
            static_cast<void>(mdIncGrp.mdUpdateAction());
            static_cast<void>(mdIncGrp.rptSeq());
            static_cast<void>(mdIncGrp.aggressorSide());
            static_cast<void>(mdIncGrp.mdEntryType());
        }

        return MessageHeader::encodedLength() + marketData_.encodedLength();
    };

private:
    MessageHeader messageHeader_;
    MarketDataIncrementalRefreshTrades marketData_;
};

#endif /* _SBE_MARKET_DATA_CODEC_BENCH_HPP */

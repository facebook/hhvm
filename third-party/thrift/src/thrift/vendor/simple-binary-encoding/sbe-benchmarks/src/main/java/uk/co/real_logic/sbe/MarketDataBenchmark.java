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
package uk.co.real_logic.sbe;

import org.openjdk.jmh.annotations.*;
import org.agrona.concurrent.UnsafeBuffer;
import uk.co.real_logic.sbe.benchmarks.fix.*;

import java.nio.ByteBuffer;

public class MarketDataBenchmark
{
    @State(Scope.Benchmark)
    public static class MyState
    {
        final int bufferIndex = 0;

        final MessageHeaderEncoder messageHeaderEncoder = new MessageHeaderEncoder();
        final MessageHeaderDecoder messageHeaderDecoder = new MessageHeaderDecoder();

        final MarketDataIncrementalRefreshTradesEncoder marketDataEncoder =
            new MarketDataIncrementalRefreshTradesEncoder();
        final MarketDataIncrementalRefreshTradesDecoder marketDataDecoder =
            new MarketDataIncrementalRefreshTradesDecoder();

        final UnsafeBuffer encodeBuffer = new UnsafeBuffer(ByteBuffer.allocateDirect(1024));
        final UnsafeBuffer decodeBuffer = new UnsafeBuffer(ByteBuffer.allocateDirect(1024));

        {
            MarketDataBenchmark.encode(messageHeaderEncoder, marketDataEncoder, decodeBuffer, bufferIndex);
        }
    }

    @Benchmark
    @BenchmarkMode(Mode.AverageTime)
    public int testEncode(final MyState state)
    {
        final MarketDataIncrementalRefreshTradesEncoder marketData = state.marketDataEncoder;
        final MessageHeaderEncoder messageHeader = state.messageHeaderEncoder;
        final UnsafeBuffer buffer = state.encodeBuffer;
        final int bufferIndex = state.bufferIndex;

        encode(messageHeader, marketData, buffer, bufferIndex);

        return marketData.encodedLength();
    }

    @Benchmark
    @BenchmarkMode(Mode.AverageTime)
    public int testDecode(final MyState state)
    {
        final MarketDataIncrementalRefreshTradesDecoder marketData = state.marketDataDecoder;
        final MessageHeaderDecoder messageHeader = state.messageHeaderDecoder;
        final UnsafeBuffer buffer = state.decodeBuffer;
        final int bufferIndex = state.bufferIndex;

        decode(messageHeader, marketData, buffer, bufferIndex);

        return marketData.encodedLength();
    }

    public static void encode(
        final MessageHeaderEncoder messageHeader,
        final MarketDataIncrementalRefreshTradesEncoder marketData,
        final UnsafeBuffer buffer,
        final int bufferIndex)
    {
        marketData
            .wrapAndApplyHeader(buffer, bufferIndex, messageHeader)
            .transactTime(1234L)
            .eventTimeDelta(987)
            .matchEventIndicator(MatchEventIndicator.END_EVENT);

        final MarketDataIncrementalRefreshTradesEncoder.MdIncGrpEncoder mdIncGrp = marketData.mdIncGrpCount(2);

        mdIncGrp.next();
        mdIncGrp.tradeId(1234L);
        mdIncGrp.securityId(56789L);
        mdIncGrp.mdEntryPx().mantissa(50);
        mdIncGrp.mdEntrySize().mantissa(10);
        mdIncGrp.numberOfOrders(1);
        mdIncGrp.mdUpdateAction(MDUpdateAction.NEW);
        mdIncGrp.rptSeq((short)1);
        mdIncGrp.aggressorSide(Side.BUY);

        mdIncGrp.next();
        mdIncGrp.tradeId(1234L);
        mdIncGrp.securityId(56789L);
        mdIncGrp.mdEntryPx().mantissa(50);
        mdIncGrp.mdEntrySize().mantissa(10);
        mdIncGrp.numberOfOrders(1);
        mdIncGrp.mdUpdateAction(MDUpdateAction.NEW);
        mdIncGrp.rptSeq((short)1);
        mdIncGrp.aggressorSide(Side.SELL);
    }

    private static void decode(
        final MessageHeaderDecoder messageHeader,
        final MarketDataIncrementalRefreshTradesDecoder marketData,
        final UnsafeBuffer buffer,
        final int bufferIndex)
    {
        messageHeader.wrap(buffer, bufferIndex);

        final int actingVersion = messageHeader.version();
        final int actingBlockLength = messageHeader.blockLength();

        marketData.wrap(buffer, bufferIndex + messageHeader.encodedLength(), actingBlockLength, actingVersion);

        marketData.transactTime();
        marketData.eventTimeDelta();
        marketData.matchEventIndicator();

        for (final MarketDataIncrementalRefreshTradesDecoder.MdIncGrpDecoder mdIncGrp : marketData.mdIncGrp())
        {
            mdIncGrp.tradeId();
            mdIncGrp.securityId();
            mdIncGrp.mdEntryPx().mantissa();
            mdIncGrp.mdEntrySize().mantissa();
            mdIncGrp.numberOfOrders();
            mdIncGrp.mdUpdateAction();
            mdIncGrp.rptSeq();
            mdIncGrp.aggressorSide();
            mdIncGrp.mdEntryType();
        }
    }

    /*
     * Benchmarks to allow execution outside of JMH.
     */

    public static void main(final String[] args)
    {
        for (int i = 0; i < 10; i++)
        {
            perfTestEncode(i);
            perfTestDecode(i);
        }
    }

    private static void perfTestEncode(final int runNumber)
    {
        final int reps = 10 * 1000 * 1000;
        final MyState state = new MyState();
        final MarketDataBenchmark benchmark = new MarketDataBenchmark();

        final long start = System.nanoTime();
        for (int i = 0; i < reps; i++)
        {
            benchmark.testEncode(state);
        }

        final long totalDuration = System.nanoTime() - start;

        System.out.printf(
            "%d - %d(ns) average duration for %s.testEncode() - message encodedLength %d%n",
            runNumber,
            totalDuration / reps,
            benchmark.getClass().getName(),
            state.marketDataEncoder.encodedLength() + state.messageHeaderEncoder.encodedLength());
    }

    private static void perfTestDecode(final int runNumber)
    {
        final int reps = 10 * 1000 * 1000;
        final MyState state = new MyState();
        final MarketDataBenchmark benchmark = new MarketDataBenchmark();

        final long start = System.nanoTime();
        for (int i = 0; i < reps; i++)
        {
            benchmark.testDecode(state);
        }

        final long totalDuration = System.nanoTime() - start;

        System.out.printf(
            "%d - %d(ns) average duration for %s.testDecode() - message encodedLength %d%n",
            runNumber,
            totalDuration / reps,
            benchmark.getClass().getName(),
            state.marketDataDecoder.encodedLength() + state.messageHeaderDecoder.encodedLength());
    }
}

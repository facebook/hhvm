using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Diagnosers;
using Org.SbeTool.Sbe.Dll;
using Uk.Co.Real_logic.Sbe.Benchmarks.Fix;

namespace Org.SbeTool.Sbe.Benchmarks
{
    [MemoryDiagnoser]
    public class MarketDataBenchmark
    {
        private readonly byte[] _eBuffer = new byte[1024];
        private readonly byte[] _dBuffer = new byte[1024];
        private DirectBuffer _encodeBuffer;
        private DirectBuffer _decodeBuffer;
        private readonly MarketDataIncrementalRefreshTrades _marketData = new MarketDataIncrementalRefreshTrades();
        private readonly MessageHeader _messageHeader = new MessageHeader();

        [GlobalSetup]
        public void Setup()
        {
            _encodeBuffer = new DirectBuffer(_eBuffer);
            _decodeBuffer = new DirectBuffer(_dBuffer);
            Encode(_decodeBuffer, 0);
        }

        [Benchmark]
        public int Encode()
        {
            return Encode(_encodeBuffer, 0);
        }

        public int Encode(DirectBuffer buffer, int bufferOffset)
        {
            _messageHeader.Wrap(buffer, bufferOffset, 0);
            _messageHeader.BlockLength = MarketDataIncrementalRefreshTrades.BlockLength;
            _messageHeader.TemplateId = MarketDataIncrementalRefreshTrades.TemplateId;
            _messageHeader.SchemaId = MarketDataIncrementalRefreshTrades.SchemaId;
            _messageHeader.Version = MarketDataIncrementalRefreshTrades.SchemaVersion;

            _marketData.WrapForEncode(buffer, bufferOffset + MessageHeader.Size);
            _marketData.TransactTime = 1234L;
            _marketData.EventTimeDelta = 987;
            _marketData.MatchEventIndicator = MatchEventIndicator.END_EVENT;

            var mdIncGrp = _marketData.MdIncGrpCount(2);

            mdIncGrp.Next();
            mdIncGrp.TradeId = 1234L;
            mdIncGrp.SecurityId = 56789L;
            mdIncGrp.MdEntryPx.Mantissa = 50;
            mdIncGrp.MdEntrySize.Mantissa = 10;
            mdIncGrp.NumberOfOrders = 1;
            mdIncGrp.MdUpdateAction = MDUpdateAction.NEW;
            mdIncGrp.RptSeq = 1;
            mdIncGrp.AggressorSide = Side.BUY;
            
            mdIncGrp.Next();
            mdIncGrp.TradeId = 1234L;
            mdIncGrp.SecurityId = 56789L;
            mdIncGrp.MdEntryPx.Mantissa = 50;
            mdIncGrp.MdEntrySize.Mantissa = 10;
            mdIncGrp.NumberOfOrders = 1;
            mdIncGrp.MdUpdateAction = MDUpdateAction.NEW;
            mdIncGrp.RptSeq = 1;
            mdIncGrp.AggressorSide = Side.SELL;
            
            return _marketData.Size;
        }

        [Benchmark]
        public int Decode()
        {
            return Decode(_decodeBuffer, 0);
        }

        public int Decode(DirectBuffer buffer, int bufferOffset)
        {
            _messageHeader.Wrap(buffer, bufferOffset, 0);

            int actingVersion = _messageHeader.Version;
            int actingBlockLength = _messageHeader.BlockLength;

            _marketData.WrapForDecode(buffer, bufferOffset + MessageHeader.Size, actingBlockLength, actingVersion);

            var transactTime = _marketData.TransactTime;
            var matchEventIndicator = _marketData.MatchEventIndicator;

            var mdIncGrpGroup = _marketData.MdIncGrp;
            while (mdIncGrpGroup.HasNext)
            {
                mdIncGrpGroup.Next();
                var tradeId = mdIncGrpGroup.TradeId;
                var securityId = mdIncGrpGroup.SecurityId;
                var mantissa = mdIncGrpGroup.MdEntryPx.Mantissa;
                var i = mdIncGrpGroup.MdEntrySize.Mantissa;
                var numberOfOrders = mdIncGrpGroup.NumberOfOrders;
                var mdUpdateAction = mdIncGrpGroup.MdUpdateAction;
                var rptSeq = mdIncGrpGroup.RptSeq;
                var aggressorSide = mdIncGrpGroup.AggressorSide;
                var mdEntryType = mdIncGrpGroup.MdEntryType;
            }

            return _marketData.Size;
        }
    }
}

use criterion::{black_box, criterion_group, criterion_main, Criterion};
use examples_uk_co_real_logic_sbe_benchmarks_fix::*;
use market_data_incremental_refresh_trades_codec::encoder::*;

struct State {
    buffer: Vec<u8>,
}

pub fn md_encode_benchmark(c: &mut Criterion) {
    let mut state = State {
        buffer: vec![0u8; 1024],
    };

    c.bench_function("encode md", |b| {
        b.iter(|| {
            let _length = encode_md(black_box(&mut state)).unwrap();
        })
    });
}

fn encode_md(state: &mut State) -> SbeResult<usize> {
    let buffer = state.buffer.as_mut_slice();
    let mut market_data = MarketDataIncrementalRefreshTradesEncoder::default();
    let mut md_inc_grp = MdIncGrpEncoder::default();

    market_data = market_data.wrap(WriteBuf::new(buffer), message_header_codec::ENCODED_LENGTH);
    market_data = market_data.header(0).parent()?;

    market_data.transact_time(1234);
    market_data.event_time_delta(987);
    market_data.match_event_indicator(MatchEventIndicator::END_EVENT);

    md_inc_grp = market_data.md_inc_grp_encoder(2, md_inc_grp);

    // first
    md_inc_grp.advance()?;
    md_inc_grp.trade_id(1234);
    md_inc_grp.security_id(56789);
    let mut md_entry_px = md_inc_grp.md_entry_px_encoder();
    md_entry_px.mantissa(50);
    md_inc_grp = md_entry_px.parent()?;
    let mut md_entry_size = md_inc_grp.md_entry_size_encoder();
    md_entry_size.mantissa(10);
    md_inc_grp = md_entry_size.parent()?;
    md_inc_grp.number_of_orders(1);
    md_inc_grp.md_update_action(MDUpdateAction::NEW);
    md_inc_grp.rpt_seq(1);
    md_inc_grp.aggressor_side(Side::BUY);

    // second
    md_inc_grp.advance()?;
    md_inc_grp.trade_id(1234);
    md_inc_grp.security_id(56789);
    let mut md_entry_px = md_inc_grp.md_entry_px_encoder();
    md_entry_px.mantissa(50);
    md_inc_grp = md_entry_px.parent()?;
    let mut md_entry_size = md_inc_grp.md_entry_size_encoder();
    md_entry_size.mantissa(10);
    md_inc_grp = md_entry_size.parent()?;
    md_inc_grp.number_of_orders(1);
    md_inc_grp.md_update_action(MDUpdateAction::NEW);
    md_inc_grp.rpt_seq(1);
    md_inc_grp.aggressor_side(Side::SELL);
    market_data = md_inc_grp.parent()?;
    Ok(market_data.encoded_length())
}

pub fn md_decode_benchmark(c: &mut Criterion) {
    let mut state = State {
        buffer: vec![0u8; 1024],
    };
    let encoded_len = encode_md(&mut state).unwrap();

    c.bench_function("decode md", |b| {
        b.iter(|| {
            let decoded_len = decode_md(black_box(&state)).unwrap();
            assert_eq!(encoded_len, decoded_len);
        })
    });
}

fn decode_md(state: &State) -> SbeResult<usize> {
    let mut market_data = MarketDataIncrementalRefreshTradesDecoder::default();

    let buf = ReadBuf::new(state.buffer.as_slice());
    let header = MessageHeaderDecoder::default().wrap(buf, 0);
    market_data = market_data.header(header);

    market_data.transact_time();
    market_data.event_time_delta();
    market_data.match_event_indicator();

    let mut md_inc_grp = market_data.md_inc_grp_decoder();
    let md_inc_grp_count = md_inc_grp.count();
    let mut count = 0;

    while let Ok(Some(_)) = md_inc_grp.advance() {
        count += 1;
        md_inc_grp.trade_id();
        md_inc_grp.security_id();
        let mut md_entry_px = md_inc_grp.md_entry_px_decoder();
        md_entry_px.mantissa();
        md_inc_grp = md_entry_px.parent()?;
        let mut md_entry_size = md_inc_grp.md_entry_size_decoder();
        md_entry_size.mantissa();
        md_inc_grp = md_entry_size.parent()?;
        md_inc_grp.number_of_orders();
        md_inc_grp.md_update_action();
        md_inc_grp.rpt_seq();
        md_inc_grp.aggressor_side();
        md_inc_grp.md_entry_type();
    }
    assert_eq!(md_inc_grp_count, count);
    market_data = md_inc_grp.parent()?;

    Ok(market_data.encoded_length())
}

criterion_group!(benches, md_encode_benchmark, md_decode_benchmark);
criterion_main!(benches);

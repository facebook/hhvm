use issue_895::{
    issue_895_codec::{decoder::Issue895Decoder, encoder::Issue895Encoder},
    MessageHeaderDecoder, ReadBuf, SbeResult, WriteBuf, ENCODED_LENGTH, SBE_BLOCK_LENGTH,
    SBE_SCHEMA_ID, SBE_SCHEMA_VERSION, SBE_TEMPLATE_ID,
};

fn create_encoder(buffer: &mut Vec<u8>) -> Issue895Encoder {
    let issue_895 =
        Issue895Encoder::default().wrap(WriteBuf::new(buffer.as_mut_slice()), ENCODED_LENGTH);
    let mut header = issue_895.header(0);
    header.parent().unwrap()
}

#[test]
fn issue_895_both_some() -> SbeResult<()> {
    assert_eq!(8, ENCODED_LENGTH);
    assert_eq!(12, SBE_BLOCK_LENGTH);
    assert_eq!(0, SBE_SCHEMA_VERSION);

    // encode...
    let mut buf = vec![0u8; 256];
    let mut encoder = create_encoder(&mut buf);
    encoder.optional_float(2.07);
    encoder.optional_double(4.12);

    // decode...
    let buf = ReadBuf::new(buf.as_slice());
    let header = MessageHeaderDecoder::default().wrap(buf, 0);
    assert_eq!(SBE_BLOCK_LENGTH, header.block_length());
    assert_eq!(SBE_SCHEMA_VERSION, header.version());
    assert_eq!(SBE_TEMPLATE_ID, header.template_id());
    assert_eq!(SBE_SCHEMA_ID, header.schema_id());

    let decoder = Issue895Decoder::default().header(header);
    assert_eq!(Some(2.07), decoder.optional_float());
    assert_eq!(Some(4.12), decoder.optional_double());

    Ok(())
}

#[test]
fn issue_895_float_none() -> SbeResult<()> {
    // encode...
    let mut buf = vec![0u8; 256];
    let mut encoder = create_encoder(&mut buf);
    encoder.optional_float(f32::NAN);
    encoder.optional_double(4.12);

    // decode...
    let buf = ReadBuf::new(buf.as_slice());
    let header = MessageHeaderDecoder::default().wrap(buf, 0);

    let decoder = Issue895Decoder::default().header(header);
    assert_eq!(None, decoder.optional_float());
    assert_eq!(Some(4.12), decoder.optional_double());

    Ok(())
}

#[test]
fn issue_895_double_none() -> SbeResult<()> {
    // encode...
    let mut buffer = vec![0u8; 256];
    let mut encoder = create_encoder(&mut buffer);
    encoder.optional_float(2.07);
    encoder.optional_double(f64::NAN);

    // decode...
    let buf = ReadBuf::new(buffer.as_slice());
    let header = MessageHeaderDecoder::default().wrap(buf, 0);

    let decoder = Issue895Decoder::default().header(header);
    assert_eq!(Some(2.07), decoder.optional_float());
    assert_eq!(None, decoder.optional_double());

    Ok(())
}

use ::issue_435::*;

fn create_encoder(buffer: &mut Vec<u8>) -> Issue435Encoder {
    let issue_435 = Issue435Encoder::default().wrap(
        WriteBuf::new(buffer.as_mut_slice()),
        message_header_codec::ENCODED_LENGTH,
    );
    let mut header = issue_435.header(0);
    header.s(*SetRef::default().set_one(true));
    header.parent().unwrap()
}

#[test]
fn issue_435_ref_test() -> SbeResult<()> {
    assert_eq!(9, message_header_codec::ENCODED_LENGTH);
    assert_eq!(1, SBE_BLOCK_LENGTH);
    assert_eq!(0, SBE_SCHEMA_VERSION);

    // encode...
    let mut buffer = vec![0u8; 256];
    let encoder = create_encoder(&mut buffer);
    encoder.example_encoder().e(EnumRef::Two);

    // decode...
    let buf = ReadBuf::new(buffer.as_slice());
    let header = MessageHeaderDecoder::default().wrap(buf, 0);
    assert_eq!(SBE_BLOCK_LENGTH, header.block_length());
    assert_eq!(SBE_SCHEMA_VERSION, header.version());
    assert_eq!(SBE_TEMPLATE_ID, header.template_id());
    assert_eq!(SBE_SCHEMA_ID, header.schema_id());
    assert_eq!(*SetRef::default().set_one(true), header.s());

    let decoder = Issue435Decoder::default().header(header);
    assert_eq!(EnumRef::Two, decoder.example_decoder().e());

    Ok(())
}

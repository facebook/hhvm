/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
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

use std::io::Cursor;
use std::u8;

use bytes::Buf;
use bytes::Bytes;

use super::BOOL_VALUES;
use super::BYTE_VALUES;
use super::DOUBLE_VALUES;
use super::FLOAT_VALUES;
use super::INT16_VALUES;
use super::INT32_VALUES;
use super::INT64_VALUES;
use crate::thrift_protocol::MessageType;
use crate::ttype::TType;
use crate::CompactProtocol;
use crate::Protocol;
use crate::ProtocolReader;
use crate::ProtocolWriter;

#[test]
fn read_write_bool_list() {
    let thetype = TType::Bool;
    let thelen = BOOL_VALUES.len();

    // Write
    let buf = serialize!(CompactProtocol, |p| {
        p.write_list_begin(thetype, thelen);

        for v in &BOOL_VALUES {
            p.write_bool(*v);
        }

        p.write_list_end();
    });

    // Read
    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(buf));
    {
        let (thetype2, thelen2) = deserializer
            .read_list_begin()
            .expect("failed to read header");

        assert_eq!(thetype, thetype2);
        assert_eq!(Some(thelen), thelen2);

        for v in &BOOL_VALUES {
            assert_eq!(*v, deserializer.read_bool().expect("failed to read bool"));
        }
        deserializer
            .read_list_end()
            .expect("failed to read list end");
    }
}

#[test]
fn read_write_string_list() {
    let string_values = vec![
        String::from(""),
        String::from("a"),
        String::from("st[uf]f"),
        String::from("st,u:ff with spaces"),
        String::from("stuff\twith\nescape\\characters'...\"lots{of}fun</xml>"),
    ];
    let thetype = TType::String;
    let thelen = string_values.len();

    let buf = serialize!(CompactProtocol, |p| {
        p.write_list_begin(thetype, thelen);

        for v in &string_values {
            p.write_string(v);
        }
        p.write_list_end();
    });

    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(buf));
    {
        let (thetype2, thelen2) = deserializer
            .read_list_begin()
            .expect("failed to read header");

        assert_eq!(thetype, thetype2);
        assert_eq!(Some(thelen), thelen2);

        for v in &string_values {
            assert_eq!(
                *v,
                deserializer.read_string().expect("failed to read string")
            );
        }
        deserializer
            .read_list_end()
            .expect("failed to read list end");
    }
}

#[test]
fn read_write_byte_list() {
    let thetype = TType::Byte;
    let thelen = BYTE_VALUES.len();

    let buf = serialize!(CompactProtocol, |p| {
        p.write_list_begin(thetype, thelen);

        for v in &BYTE_VALUES {
            p.write_byte(*v);
        }
        p.write_list_end();
    });

    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(buf));
    {
        let (thetype2, thelen2) = deserializer
            .read_list_begin()
            .expect("failed to read header");

        assert_eq!(thetype, thetype2);
        assert_eq!(Some(thelen), thelen2);

        for v in &BYTE_VALUES {
            assert_eq!(*v, deserializer.read_byte().expect("failed to read byte"));
        }
        deserializer
            .read_list_end()
            .expect("failed to read list end");
    }
}

#[test]
fn read_write_i16_list() {
    let thetype = TType::I16;
    let thelen = INT16_VALUES.len();

    let buf = serialize!(CompactProtocol, |p| {
        p.write_list_begin(thetype, thelen);

        for v in &INT16_VALUES {
            p.write_i16(*v);
        }
        p.write_list_end();
    });

    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(buf));
    {
        let (thetype2, thelen2) = deserializer
            .read_list_begin()
            .expect("failed to read header");

        assert_eq!(thetype, thetype2);
        assert_eq!(Some(thelen), thelen2);

        for v in &INT16_VALUES {
            assert_eq!(*v, deserializer.read_i16().expect("failed to read i16"));
        }
        deserializer
            .read_list_end()
            .expect("failed to read list end");
    }
}

#[test]
fn read_write_i32_list() {
    let thetype = TType::I32;
    let thelen = INT32_VALUES.len();

    let buf = serialize!(CompactProtocol, |p| {
        p.write_list_begin(thetype, thelen);

        for v in &INT32_VALUES {
            p.write_i32(*v);
        }
        p.write_list_end();
    });

    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(buf));
    {
        let (thetype2, thelen2) = deserializer
            .read_list_begin()
            .expect("failed to read header");

        assert_eq!(thetype, thetype2);
        assert_eq!(Some(thelen), thelen2);

        for v in &INT32_VALUES {
            assert_eq!(*v, deserializer.read_i32().expect("failed to read i32"));
        }
        deserializer
            .read_list_end()
            .expect("failed to read list end");
    }
}

#[test]
fn read_write_i64_list() {
    let thetype = TType::I64;
    let thelen = INT64_VALUES.len();

    let buf = serialize!(CompactProtocol, |p| {
        p.write_list_begin(thetype, thelen);

        for v in &INT64_VALUES {
            p.write_i64(*v);
        }
        p.write_list_end();
    });

    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(buf));
    {
        let (thetype2, thelen2) = deserializer
            .read_list_begin()
            .expect("failed to read header");

        assert_eq!(thetype, thetype2);
        assert_eq!(Some(thelen), thelen2);

        for v in &INT64_VALUES {
            assert_eq!(*v, deserializer.read_i64().expect("failed to read i64"));
        }
        deserializer
            .read_list_end()
            .expect("failed to read list end");
    }
}

#[test]
fn read_write_f32_list() {
    let thetype = TType::Float;
    let thelen = FLOAT_VALUES.len();

    let buf = serialize!(CompactProtocol, |p| {
        p.write_list_begin(thetype, thelen);

        for v in &FLOAT_VALUES {
            p.write_float(*v);
        }
        p.write_list_end();
    });

    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(buf));
    {
        let (thetype2, thelen2) = deserializer
            .read_list_begin()
            .expect("failed to read header");

        assert_eq!(thetype, thetype2);
        assert_eq!(Some(thelen), thelen2);

        for v in &FLOAT_VALUES {
            let val = deserializer.read_float().expect("failed to read f32");
            // In Rust, NaN != NaN
            if v.is_nan() {
                assert!(val.is_nan());
            } else {
                assert_eq!(*v, val);
            }
        }
        deserializer
            .read_list_end()
            .expect("failed to read list end");
    }
}

#[test]
fn read_write_f64_list() {
    let thetype = TType::Double;
    let thelen = DOUBLE_VALUES.len();

    let buf = serialize!(CompactProtocol, |p| {
        p.write_list_begin(thetype, thelen);

        for v in &DOUBLE_VALUES {
            p.write_double(*v);
        }
        p.write_list_end();
    });

    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(buf));
    {
        let (thetype2, thelen2) = deserializer
            .read_list_begin()
            .expect("failed to read header");

        assert_eq!(thetype, thetype2);
        assert_eq!(Some(thelen), thelen2);

        for v in &DOUBLE_VALUES {
            let val = deserializer.read_double().expect("failed to read f64");
            // In Rust, NaN != NaN :)
            if v.is_nan() {
                assert!(val.is_nan());
            } else {
                assert_eq!(*v, val);
            }
        }
        deserializer
            .read_list_end()
            .expect("failed to read list end");
    }
}

#[test]
fn read_write_f64_set() {
    let thetype = TType::Double;
    let thelen = DOUBLE_VALUES.len();

    let buf = serialize!(CompactProtocol, |p| {
        p.write_set_begin(thetype, thelen);

        for v in &DOUBLE_VALUES {
            p.write_double(*v);
        }
        p.write_list_end();
    });

    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(buf));
    {
        let (thetype2, thelen2) = deserializer
            .read_set_begin()
            .expect("failed to read header");

        assert_eq!(thetype, thetype2);
        assert_eq!(Some(thelen), thelen2);

        for v in &DOUBLE_VALUES {
            let val = deserializer.read_double().expect("failed to read f64");
            // In Rust, NaN != NaN :)
            if v.is_nan() {
                assert!(val.is_nan());
            } else {
                assert_eq!(*v, val);
            }
        }
        assert!(deserializer.read_set_end().is_ok());
    }
}

#[test]
fn read_write_string_i64_map() {
    let key_type = TType::String;
    let value_type = TType::I64;
    let thelen = INT64_VALUES.len();
    let string_keys: Vec<String> = INT64_VALUES.iter().map(|&v| v.to_string()).collect();

    let buf = serialize!(CompactProtocol, |p| {
        p.write_map_begin(key_type, value_type, thelen);

        for (k, v) in string_keys.iter().zip(INT64_VALUES.iter()) {
            p.write_string(k);
            p.write_i64(*v);
        }
        p.write_map_end();
    });

    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(buf));
    {
        let (key_type2, value_type2, thelen2) = deserializer
            .read_map_begin()
            .expect("failed to read header");

        assert_eq!(key_type, key_type2);
        assert_eq!(value_type, value_type2);
        assert_eq!(Some(thelen), thelen2);

        for (k, v) in string_keys.iter().zip(INT64_VALUES.iter()) {
            assert_eq!(
                *k,
                deserializer.read_string().expect("failed to read string")
            );
            assert_eq!(*v, deserializer.read_i64().expect("failed to read i64"));
        }
        assert!(deserializer.read_map_end().is_ok());
    }
}

#[test]
fn read_write_message() {
    let msg_name = String::from("hello_message");
    let msg_type = MessageType::Call;
    let seq_id = 1;

    let buf = serialize!(CompactProtocol, |p| {
        p.write_message_begin(&msg_name, msg_type, seq_id);
        p.write_message_end();
    });

    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(buf));
    {
        let (msg_name2, msg_type2, seq_id2) = deserializer
            .read_message_begin(|msg| String::from_utf8(msg.to_vec()).expect("bad msg"))
            .expect("failed to read message");
        assert_eq!(msg_name, msg_name2);
        assert_eq!(msg_type, msg_type2);
        assert_eq!(seq_id, seq_id2);

        assert!(deserializer.read_message_end().is_ok());
    }
}

#[test]
fn write_message_begin() {
    let buf = serialize!(CompactProtocol, |serializer| {
        // Write the following information:
        //  1) Version bitwise or'd with MessageType: (0x80001000 | 0x00000001) = [128, 1, 0, 1]
        //  2) String name prefixed by i32 string length: [0, 0, 0, 4, 116, 101, 115, 116]
        //  3) i32 seqid: [0, 0, 0, 1]
        let _ = serializer.write_message_begin(
            "test",
            MessageType::try_from(1).expect("try_from failed"),
            1,
        );
        let _ = serializer.write_message_end();
    });

    let vec = vec![0x82, 2 | (1 << 5), 1, 4, b't', b'e', b's', b't'];

    assert_eq!(vec, buf);
}

#[test]
fn serializer_overflow() {
    let vec: Vec<u8> = (0..u8::MAX).into_iter().collect();
    let buf = serialize!(CompactProtocol, |p| for i in 0..u8::MAX {
        let _ = p.write_byte(i as i8);
    });

    let buf = buf;

    assert_eq!(vec, buf);
}

#[test]
fn read_message_begin() {
    // Data in vec:
    let msg = vec![
        0x82,            // protocol id
        0x02 | (1 << 5), // version and MessageType
        1,               // seqid
        4,               // length
        b't',
        b'e',
        b's',
        b't',
    ];
    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(Bytes::from(msg)));

    let rmb =
        deserializer.read_message_begin(|name| String::from_utf8(name.to_vec()).expect("bad msg"));
    deserializer
        .read_message_end()
        .expect("read_message_end failed");

    match rmb {
        Ok((name, mty, sid)) => {
            assert_eq!(name, "test");
            assert_eq!(mty, MessageType::try_from(1).expect("try_from failed"));
            assert_eq!(sid, 1);
        }
        Err(bad) => panic!("unexpected error {:?}", bad),
    }
}

#[test]
fn deserializer_underflow() {
    use crate::errors::ProtocolError;

    let vec = vec![1 << 1];
    let mut deserializer = <CompactProtocol>::deserializer(Cursor::new(Bytes::from(vec)));

    let last_val = deserializer.read_i32();

    match last_val {
        Ok(i) => assert_eq!(i, 1i32, "read incorrect value for last_val"),
        Err(_) => panic!("Error encoutered reading final i32 from vec"),
    };

    match deserializer.read_i32() {
        Ok(v) => panic!("got unexpected value {}", v),
        Err(err) => match err.downcast_ref::<ProtocolError>() {
            Some(ProtocolError::EOF) => {}
            _ => panic!("got unexpected err {:?}", err),
        },
    }
}

#[test]
fn read_binary_from_chained_buffer() {
    use crate::compact_protocol::CompactProtocolDeserializer;

    let buf1 = Cursor::new(b"\x05hello\x06 ");
    let buf2 = Cursor::new(b"world");
    let joined = buf1.chain(buf2);

    let mut deserializer = CompactProtocolDeserializer::new(joined);

    let result: Vec<u8> = deserializer
        .read_binary()
        .expect("read \"hello\" from the buffer");
    assert_eq!(result.as_slice(), b"hello");

    let result: Vec<u8> = deserializer
        .read_binary()
        .expect("read \" world\" from the buffer");
    assert_eq!(result.as_slice(), b" world");
}

// Test for T29755131
#[test]
fn skip_i32_ok() {
    use crate::compact_protocol::CompactProtocolDeserializer;
    use crate::compact_protocol::CompactProtocolSerializer;
    use crate::protocol::ProtocolReader;
    use crate::protocol::ProtocolWriter;

    let buf = {
        let mut serialize =
            CompactProtocolSerializer::with_buffer(bytes::BytesMut::with_capacity(128));

        serialize.write_i32(123);

        serialize.finish()
    };

    let mut deserializer = CompactProtocolDeserializer::new(Cursor::new(buf));

    match deserializer.skip(TType::I32) {
        Ok(()) => {}
        Err(err) => panic!("Unexpected error {:?}", err),
    }
}

// Test for T29755131
#[test]
fn skip_stop() {
    use crate::compact_protocol::CompactProtocolDeserializer;
    use crate::compact_protocol::CompactProtocolSerializer;
    use crate::protocol::ProtocolReader;
    use crate::protocol::ProtocolWriter;
    use crate::ProtocolError;

    let buf = {
        let mut serialize =
            CompactProtocolSerializer::with_buffer(bytes::BytesMut::with_capacity(128));

        serialize.write_field_stop();

        serialize.finish()
    };

    let mut deserializer = CompactProtocolDeserializer::new(Cursor::new(buf));

    match deserializer.skip(TType::Stop) {
        Ok(()) => panic!("Unexpected success"),
        Err(err) => match err.downcast_ref::<ProtocolError>() {
            Some(ProtocolError::UnexpectedStopInSkip) => {}
            _ => panic!("Unexpected error {:?}", err),
        },
    }
}

// Test for T29755131
#[test]
fn skip_stop_in_container() {
    use crate::compact_protocol::CompactProtocolDeserializer;
    use crate::compact_protocol::CompactProtocolSerializer;
    use crate::protocol::ProtocolReader;
    use crate::protocol::ProtocolWriter;
    use crate::ProtocolError;

    let buf = {
        let mut serialize =
            CompactProtocolSerializer::with_buffer(bytes::BytesMut::with_capacity(128));

        serialize.write_list_begin(TType::Stop, 123);

        serialize.finish()
    };

    let mut deserializer = CompactProtocolDeserializer::new(Cursor::new(buf));

    match deserializer.skip(TType::List) {
        Ok(()) => panic!("Unexpected Success"),
        Err(err) => match err.downcast_ref::<ProtocolError>() {
            Some(ProtocolError::UnexpectedStopInSkip) => {}
            _ => panic!("unexpected error {:?}", err),
        },
    }
}

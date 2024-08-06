/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::f64::consts;

use serde::Serialize;

use super::serialize;

#[derive(Debug, Serialize)]
enum TestEnum {
    TestUnit,
    TestNewtype(#[serde(with = "serde_bytes")] &'static [u8]),
    TestTuple(i8, u64),
    TestStruct { abc: (), def: char },
}

#[derive(Debug, Serialize)]
struct SerializeStruct {
    test_list: Vec<i32>,
    test_tuple: (String, Option<f64>),
    test_enum: TestEnum,
    test_enum_list: Vec<TestEnum>,
}

const BASIC_SERIALIZED: &[u8] = b"\x00\x02\x00\x00\x00\x00\x04\xcb\x00\x01\x03\x04\r\x03\t\
                                  test_list\x00\x03\x05\x03\x03\x03\x04\x03*\x04\xdb\x03\x05\x00\
                                  \x00\x08\x00\r\x03\ntest_tuple\x00\x03\x02\r\x03\x03foo\x07\
                                  \x18-DT\xfb!\t@\r\x03\ttest_enum\r\x03\x08TestUnit\r\x03\x0e\
                                  test_enum_list\x00\x03\x03\x01\x03\x01\r\x03\x0bTestNewtype\
                                  \x02\x03\tBSER test\x01\x03\x01\r\x03\tTestTuple\x00\x03\x02\x03*\
                                  \x06\xff\xff\xff\xff\xff\xff\xff\x7f\x01\x03\x01\r\x03\n\
                                  TestStruct\x00\x03\x02\r\x03\x03abc\n\r\x03\x03\
                                  def\r\x03\x04\xf0\x9f\x92\xa9";

#[test]
fn test_basic_serialize() {
    let to_serialize = SerializeStruct {
        test_list: vec![3, 4, 42, 987, 2 << 18],
        test_tuple: ("foo".into(), Some(consts::PI)),
        test_enum: TestEnum::TestUnit,
        test_enum_list: vec![
            TestEnum::TestNewtype(&b"BSER test"[..]),
            TestEnum::TestTuple(42, i64::MAX as u64),
            TestEnum::TestStruct {
                abc: (),
                def: '\u{1f4a9}',
            },
        ],
    };

    let out = Vec::new();
    let out = serialize(out, to_serialize).unwrap();
    assert_eq!(out, BASIC_SERIALIZED);
}

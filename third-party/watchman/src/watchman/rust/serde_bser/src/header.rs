/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! Header constants for BSER.

pub const EMPTY_HEADER: &[u8] = b"\x00\x02\x00\x00\x00\x00\x05\x00\x00\x00\x00";

pub const BSER_ARRAY: u8 = 0x00;
pub const BSER_OBJECT: u8 = 0x01;
pub const BSER_BYTESTRING: u8 = 0x02;
pub const BSER_INT8: u8 = 0x03;
pub const BSER_INT16: u8 = 0x04;
pub const BSER_INT32: u8 = 0x05;
pub const BSER_INT64: u8 = 0x06;
pub const BSER_REAL: u8 = 0x07;
pub const BSER_TRUE: u8 = 0x08;
pub const BSER_FALSE: u8 = 0x09;
pub const BSER_NULL: u8 = 0x0a;
pub const BSER_TEMPLATE: u8 = 0x0b;
pub const BSER_SKIP: u8 = 0x0c;
pub const BSER_UTF8STRING: u8 = 0x0d;

// Capabilities (we would ideally want to use EnumSet here, but
// https://github.com/contain-rs/enum-set/issues/21 stops us)
#[allow(unused)]
pub const BSER_CAP_DISABLE_UNICODE: u8 = 0x01;
#[allow(unused)]
pub const BSER_CAP_DISABLE_UNICODE_FOR_ERRORS: u8 = 0x02;

pub fn header_byte_desc(byte: u8) -> String {
    match byte {
        BSER_ARRAY => "BSER_ARRAY".into(),
        BSER_OBJECT => "BSER_OBJECT".into(),
        BSER_BYTESTRING => "BSER_BYTESTRING".into(),
        BSER_INT8 => "BSER_INT8".into(),
        BSER_INT16 => "BSER_INT16".into(),
        BSER_INT32 => "BSER_INT32".into(),
        BSER_INT64 => "BSER_INT64".into(),
        BSER_REAL => "BSER_REAL".into(),
        BSER_TRUE => "BSER_TRUE".into(),
        BSER_FALSE => "BSER_FALSE".into(),
        BSER_NULL => "BSER_NULL".into(),
        BSER_TEMPLATE => "BSER_TEMPLATE".into(),
        BSER_SKIP => "BSER_SKIP".into(),
        BSER_UTF8STRING => "BSER_UTF8STRING".into(),
        ch => format!("unknown byte '{:?}'", ch),
    }
}

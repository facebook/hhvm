// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![allow(dead_code)] //for now

pub(crate) const MAGIC_NUMBER_SMALL: u32 = 0x8495A6BE; // 10000100100101011010011010111110
pub(crate) const MAGIC_NUMBER_BIG: u32 = 0x8495A6BF; // 10000100100101011010011010111111

///   Header format for the "small" model: 20 bytes
///       0   "small" magic number
///       4   length of marshaled data, in bytes
///       8   number of shared blocks
///      12   size in words when read on a 32-bit platform
///      16   size in words when read on a 64-bit platform
///   The 4 numbers are 32 bits each, in big endian.
///
///   Header format for the "big" model: 32 bytes
///       0   "big" magic number
///       4   four reserved bytes, currently set to 0
///       8   length of marshaled data, in bytes
///      16   number of shared blocks
///      24   size in words when read on a 64-bit platform
///   The 3 numbers are 64 bits each, in big endian.

// ---
// c.f. runtime/caml/config.h

#[cfg(target_arch = "arm")]
// Careful. This might not be quite right. See
// https://github.com/ocaml/ocaml/issues/4224.
pub(crate) const ARCH_FLOAT_ENDIANNESS: u32 = 0x45670123;

#[cfg(not(target_arch = "arm"))]
#[cfg(target_endian = "big")]
pub(crate) const ARCH_FLOAT_ENDIANNESS: u32 = 0x76543210;

#[cfg(not(target_arch = "arm"))]
#[cfg(target_endian = "little")]
pub(crate) const ARCH_FLOAT_ENDIANNESS: u32 = 0x01234567;

// ---

// Codes for the compact format

pub(crate) const PREFIX_SMALL_BLOCK: libc::c_int = 0x80; // 10000000
pub(crate) const PREFIX_SMALL_INT: libc::c_int = 0x40; // 01000000
pub(crate) const PREFIX_SMALL_STRING: libc::c_int = 0x20; // 00100000
pub(crate) const CODE_INT8: libc::c_int = 0x0; // 00000000
pub(crate) const CODE_INT16: libc::c_int = 0x1; // 00000001
pub(crate) const CODE_INT32: libc::c_int = 0x2; // 00000010
pub(crate) const CODE_INT64: libc::c_int = 0x3; // 00000011
pub(crate) const CODE_SHARED8: libc::c_int = 0x4; // 00000100
pub(crate) const CODE_SHARED16: libc::c_int = 0x5; // 00000101
pub(crate) const CODE_SHARED32: libc::c_int = 0x6; // 00000110
pub(crate) const CODE_SHARED64: libc::c_int = 0x14; // 00010100
pub(crate) const CODE_BLOCK32: libc::c_int = 0x8; // 00001000
pub(crate) const CODE_BLOCK64: libc::c_int = 0x13; // 00010011
pub(crate) const CODE_STRING8: libc::c_int = 0x9; // 00001001
pub(crate) const CODE_STRING32: libc::c_int = 0xA; // 00001010
pub(crate) const CODE_STRING64: libc::c_int = 0x15; // 00010101
pub(crate) const CODE_DOUBLE_BIG: libc::c_int = 0xB; // 00001011
pub(crate) const CODE_DOUBLE_LITTLE: libc::c_int = 0xC; // 00001100
pub(crate) const CODE_DOUBLE_ARRAY8_BIG: libc::c_int = 0xD; // 00001101
pub(crate) const CODE_DOUBLE_ARRAY8_LITTLE: libc::c_int = 0xE; // 00001110
pub(crate) const CODE_DOUBLE_ARRAY32_BIG: libc::c_int = 0xF; // 00001111
pub(crate) const CODE_DOUBLE_ARRAY32_LITTLE: libc::c_int = 0x7; // 00000111
pub(crate) const CODE_DOUBLE_ARRAY64_BIG: libc::c_int = 0x16; // 00010110
pub(crate) const CODE_DOUBLE_ARRAY64_LITTLE: libc::c_int = 0x17; // 00010111
pub(crate) const CODE_CODEPOINTER: libc::c_int = 0x10; // 00010000
pub(crate) const CODE_INFIXPOINTER: libc::c_int = 0x11; // 00010001
pub(crate) const CODE_CUSTOM: libc::c_int = 0x12; // 00010010 (deprecated)
pub(crate) const CODE_CUSTOM_LEN: libc::c_int = 0x18; // 00011000
pub(crate) const CODE_CUSTOM_FIXED: libc::c_int = 0x19; // 00011001

macro_rules! cond_arch_float_endian {
    ($if_:ident, $else_:ident) => {
        if ARCH_FLOAT_ENDIANNESS == 0x76543210 {
            $if_
        } else {
            $else_
        }
    };
}
pub(crate) const CODE_DOUBLE_NATIVE: libc::c_int =
    cond_arch_float_endian!(CODE_DOUBLE_BIG, CODE_DOUBLE_LITTLE);
pub(crate) const CODE_DOUBLE_ARRAY8_NATIVE: libc::c_int =
    cond_arch_float_endian!(CODE_DOUBLE_ARRAY8_BIG, CODE_DOUBLE_ARRAY8_LITTLE);
pub(crate) const CODE_DOUBLE_ARRAY32_NATIVE: libc::c_int =
    cond_arch_float_endian!(CODE_DOUBLE_ARRAY32_BIG, CODE_DOUBLE_ARRAY32_LITTLE);
pub(crate) const CODE_DOUBLE_ARRAY64_NATIVE: libc::c_int =
    cond_arch_float_endian!(CODE_DOUBLE_ARRAY64_BIG, CODE_DOUBLE_ARRAY64_LITTLE);

///   Size-ing data structures for extern.  Chosen so that
///   sizeof(struct trail_block) and sizeof(struct output_block)
///   are slightly below 8Kb.
pub(crate) const ENTRIES_PER_TRAIL_BLOCK: i16 = 1025;
pub(crate) const SIZE_EXTERN_OUTPUT_BLOCK: usize = 8100;

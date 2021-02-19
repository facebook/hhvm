// Copyright (c) 2021, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use libc::{c_char, c_int};

#[repr(C)]
pub struct CBuf {
    pub buf: *mut c_char,
    pub buf_len: c_int,
}

/// Utility for C raw pointer conversions.
/// Safety:
/// - The `usize` argument must legitmately be reinterpretable as a `*const T`;
/// - The resulting `*const T` must point to a valid properly initialized `T`.
pub unsafe fn from_ptr<'a, T: 'a, U, F: FnOnce(&'a T) -> U>(
    p: usize,
    f: F,
) -> std::option::Option<U> {
    match p {
        0 => None,
        _ => Some(f(&*(p as *const T))),
    }
}

/// C `char*` conversions.
pub mod cstr {
    use libc::{c_char, c_int};

    /// `std::vec::Vec<u8> to `char*`
    ///  Safety:
    ///  - No runtime assertion is made that `v` contains no 0 bytes.
    pub unsafe fn from_vec_u8(v: std::vec::Vec<u8>) -> *const c_char {
        std::ffi::CString::from_vec_unchecked(v).into_raw() as *const c_char
    }

    /// `char*` to `&[u8]`.
    /// Safety:
    /// - `s` must point to a properly initialized null-terminated C
    ///   string.
    pub unsafe fn to_u8<'a>(s: *const c_char) -> &'a [u8] {
        std::slice::from_raw_parts(s as *const u8, libc::strlen(s))
    }
    /// `char*` to `&mut [u8]`.
    /// Safety:
    /// - `buf` must be valid for reads and writes for `buf_len *
    ///   mem::sizeof::<u8>()` bytes.
    pub unsafe fn to_mut_u8<'a>(buf: *mut c_char, buf_len: c_int) -> &'a mut [u8] {
        std::slice::from_raw_parts_mut(buf as *mut u8, buf_len as usize)
    }
    /// `char*` to `&str`.
    /// Safety:
    /// - `s` must point to a properly initialized null-terminated C
    ///   string;
    /// - This function does not check that the bytes contained by
    ///   `s` are valid UTF-8.
    pub unsafe fn to_str<'a>(s: *const c_char) -> &'a str {
        std::str::from_utf8_unchecked(to_u8(s))
    }
    /// `char**` to `Vec<&str>`.
    /// Safety:
    /// - `cstrs` must point to `num_cstrs` consecutive properly
    ///    initialized null-terminated C strings.
    pub unsafe fn to_vec<'a>(
        cstrs: *const *const c_char,
        num_cstrs: usize,
    ) -> std::vec::Vec<&'a str> {
        std::slice::from_raw_parts(cstrs, num_cstrs)
            .iter()
            .map(|&s| to_str(s))
            .collect()
    }
}

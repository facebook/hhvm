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

/// Utility for raw pointer conversions.
pub fn from_ptr<'a, T: 'a, U, F: FnOnce(&'a T) -> U>(p: usize, f: F) -> std::option::Option<U> {
    match p {
        0 => None,
        _ => Some(f(unsafe { &*(p as *const T) })),
    }
}

/// C `char*` conversions.
pub mod cstr {
    use libc::{c_char, c_int};

    /// `char*` to `&[u8]`.
    pub fn to_u8<'a>(s: *const c_char) -> &'a [u8] {
        unsafe { std::slice::from_raw_parts(s as *const u8, libc::strlen(s)) }
    }
    /// `char*` to `&mut [u8]`.
    pub fn to_mut_u8<'a>(buf: *mut c_char, buf_len: c_int) -> &'a mut [u8] {
        unsafe { std::slice::from_raw_parts_mut(buf as *mut u8, buf_len as usize) }
    }
    /// `char*` to `&str`.
    pub fn to_str<'a>(s: *const c_char) -> &'a str {
        unsafe { std::str::from_utf8_unchecked(to_u8(s)) }
    }
    /// `char**` to `Vec<&str>`.
    pub fn to_vec<'a>(cstrs: *const *const c_char, num_cstrs: usize) -> std::vec::Vec<&'a str> {
        unsafe {
            std::slice::from_raw_parts(cstrs, num_cstrs)
                .iter()
                .map(|&s| to_str(s))
                .collect()
        }
    }
}

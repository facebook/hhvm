// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(non_camel_case_types)]

use libc::c_long;
use libc::c_ulong;
use libc::c_void;
use libc::memcpy;
use ocamlrep::Value;

type intnat = c_long;
type uintnat = c_ulong;
type value = intnat;
type mlsize_t = uintnat;

extern "C" {
    fn caml_alloc_string(len: mlsize_t) -> value;
}

#[no_mangle]
unsafe extern "C" fn ocamlrep_marshal_output_value_to_string(v: value, flags: value) -> value {
    let v = Value::from_bits(v as usize);
    let flags = Value::from_bits(flags as usize);
    let mut vec = vec![];
    ocamlrep_marshal::output_val(&mut vec, v, flags).unwrap();
    let res: value = caml_alloc_string(vec.len() as mlsize_t);
    memcpy(res as *mut c_void, vec.as_ptr() as *const c_void, vec.len());
    res
}

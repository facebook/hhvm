// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ffi::CString;
use std::os::raw::c_char;

extern "C" {
    fn caml_startup(argv: *const *const c_char);
}

/// Initialize the OCaml runtime.
///
/// # Safety
///
/// Must be invoked from the main thread. Must be invoked only once. Must
/// precede any other interaction with the OCaml runtime.
///
/// # Panics
///
/// Panics if any argument in argv contains a nul byte.
pub unsafe fn startup() {
    let mut argv: Vec<*const c_char> = (std::env::args().into_iter())
        .map(|arg| CString::new(arg.as_str()).unwrap().into_raw() as _)
        .collect();
    argv.push(std::ptr::null());
    caml_startup(argv.leak().as_ptr());
}

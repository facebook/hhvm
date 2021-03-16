// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::CLOSURE_TAG;
use std::ffi::CString;
use std::os::raw::c_char;

extern "C" {
    fn caml_named_value(name: *const c_char) -> *mut usize;
    fn caml_callback_exn(closure: usize, arg1: usize) -> usize;
}

pub type Value = usize;

/// Return a named value registered by OCaml (e.g., via `Callback.register`).
/// If no value was registered for that name, return `None`.
///
/// # Safety
///
/// OCaml runtime doesn't document thread safety for each API, we are conservative
/// to assume that APIs are not thread-safe. If any other thread interacts with
/// the OCaml runtime during the execution of this function, undefined behavior
/// will result.
///
/// # Panics
///
/// Panics if the given `name` contains a nul byte.
pub unsafe fn named_value<S: AsRef<str>>(name: S) -> Option<Value> {
    let name = CString::new(name.as_ref()).expect("string contained nul byte");
    let named = caml_named_value(name.as_ptr());
    if named.is_null() {
        return None;
    }
    Some(*named)
}
pub enum Error {
    NotInvokable,
    Exception(Value),
}

/// Call a closure with a single argument.
///
/// # Safety
///
/// The calling thread must be known to the OCaml runtime system. Threads
/// created from OCaml (via `Thread.create`) and the main thread are
/// automatically known to the runtime system. See the [OCaml manual] for more
/// details.
///
/// [OCaml manual]: (https://caml.inria.fr/pub/docs/manual-ocaml/intfc.html#s:C-multithreading)
pub unsafe fn callback_exn(f: Value, arg: Value) -> Result<Value, Error> {
    let f_block = match ocamlrep::Value::from_bits(f).as_block() {
        Some(block) => block,
        None => return Err(Error::NotInvokable),
    };
    if f_block.tag() != CLOSURE_TAG {
        return Err(Error::NotInvokable);
    }
    let res = caml_callback_exn(f, arg);

    if is_exception_result(res) {
        Err(Error::Exception(extract_exception(res)))
    } else {
        Ok(res)
    }
}

/// Exception returned by caml_callback_exn is documented
/// in runtime/caml/mlvalues.h
///
/// "Encoded exceptional return values, when functions are suffixed with
/// _exn. Encoded exceptions are invalid values and must not be seen
/// by the garbage collector."
/// #define Make_exception_result(v) ((v) | 2)
/// #define Is_exception_result(v) (((v) & 3) == 2)
/// #define Extract_exception(v) ((v) & ~3)
fn is_exception_result(v: usize) -> bool {
    v & 3 == 2
}

fn extract_exception(v: usize) -> usize {
    v & !3
}

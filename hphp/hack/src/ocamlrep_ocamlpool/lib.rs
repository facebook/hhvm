// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ffi::{CStr, CString};
use std::marker::PhantomData;
use std::panic::UnwindSafe;

use ocamlpool_rust::utils::{caml_set_field, reserve_block};
use ocamlrep::{Allocator, Value};

pub use ocamlrep::OcamlRep;

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
    static mut ocamlpool_generation: usize;
}

pub struct Pool<'a> {
    _phantom: PhantomData<Value<'a>>,
}

impl<'a> Pool<'a> {
    #[inline(always)]
    pub fn new() -> Self {
        unsafe { ocamlpool_enter() };
        Self {
            _phantom: PhantomData,
        }
    }

    #[inline(always)]
    pub fn add<T: OcamlRep>(&mut self, value: &T) -> Value<'a> {
        value.to_ocamlrep(self)
    }
}

impl Drop for Pool<'_> {
    #[inline(always)]
    fn drop(&mut self) {
        unsafe { ocamlpool_leave() };
    }
}

impl<'a> Allocator<'a> for Pool<'a> {
    #[inline(always)]
    fn generation(&self) -> usize {
        unsafe { ocamlpool_generation }
    }

    #[inline(always)]
    fn block_with_size_and_tag(&mut self, size: usize, tag: u8) -> *mut Value<'a> {
        unsafe { reserve_block(tag, size) as *mut Value<'a> }
    }

    #[inline(always)]
    unsafe fn set_field(block: *mut Value<'a>, index: usize, value: Value<'a>) {
        caml_set_field(block as usize, index, value.to_bits());
    }
}

#[inline(always)]
pub fn to_ocaml<T: OcamlRep>(value: &T) -> usize {
    let mut pool = Pool::new();
    let result = pool.add(value);
    unsafe { result.to_bits() }
}

/// Catches panics in `f` and raises a OCaml exception of type RustException
/// with the panic message (if the panic was raised with a `&str` or `String`).
///
/// Requires a RustException type to be registered on the OCaml side:
///
/// ```ocaml
/// exception RustException of string
/// let () =
///   Callback.register_exception "rust exception" (RustException "")
/// ```
pub fn catch_unwind(f: impl FnOnce() -> usize + UnwindSafe) -> usize {
    let err = match std::panic::catch_unwind(f) {
        Ok(value) => return value,
        Err(err) => err,
    };
    let msg: &str = if let Some(s) = err.downcast_ref::<&str>() {
        s
    } else if let Some(s) = err.downcast_ref::<String>() {
        s.as_str()
    } else {
        // TODO: Build a smarter message in this case (using panic::set_hook?)
        "Panicked with non-string object"
    };
    unsafe {
        let exn_value_name = CStr::from_bytes_with_nul_unchecked(b"rust exception\0");
        let exn = ocaml::core::callback::caml_named_value(exn_value_name.as_ptr() as *const u8);
        assert!(!exn.is_null());
        let msg = CString::new(msg).unwrap();
        ocaml::core::fail::caml_raise_with_string(exn as usize, msg.as_ptr());
    }
    unreachable!();
}

/// Assume that some Pool exists in some parent scope. Since ocamlpool is
/// implemented with statics, we don't need a reference to that pool to write to
/// it.
#[inline(always)]
pub unsafe fn add_to_ambient_pool<T: OcamlRep>(value: &T) -> usize {
    let mut fake_pool = Pool {
        _phantom: PhantomData,
    };
    let result = value.to_ocamlrep(&mut fake_pool).to_bits();
    std::mem::forget(fake_pool);
    result
}

#[macro_export]
macro_rules! ocaml_ffi_no_panic_fn {
    (fn $name:ident($($param:ident: $ty:ty),+  $(,)?) -> $ret:ty $code:block) => {
        #[no_mangle]
        pub extern "C" fn $name ($($param: usize,)*) -> usize {
            use $crate::OcamlRep;
            $(let $param = unsafe { <$ty>::from_ocaml($param).unwrap() };)*
            $crate::to_ocaml::<$ret>(&(|| $code)())
        }
    };

    (fn $name:ident() -> $ret:ty $code:block) => {
        #[no_mangle]
        pub extern "C" fn $name (_unit: usize) -> usize {
            $crate::to_ocaml::<$ret>(&$code)
        }
    };

    (fn $name:ident($($param:ident: $ty:ty),*  $(,)?) $code:block) => {
        $crate::ocaml_ffi_no_panic_fn! {
            fn $name($($param: $ty),*) -> () $code
        }
    };
}

/// For perf-sensitive use cases that cannot pay the cost of catch_unwind.
///
/// Take care that the function body, the parameters' implementations of
/// `OcamlRep::from_ocamlrep`, and the return type's implementation of
/// `OcamlRep::to_ocamlrep` do not panic.
#[macro_export]
macro_rules! ocaml_ffi_no_panic {
    ($(fn $name:ident($($param:ident: $ty:ty),*  $(,)?) $(-> $ret:ty)? $code:block)*) => {
        $($crate::ocaml_ffi_no_panic_fn! {
            fn $name($($param: $ty),*) $(-> $ret)* $code
        })*
    };
}

#[macro_export]
macro_rules! ocaml_ffi_fn {
    (fn $name:ident($($param:ident: $ty:ty),+  $(,)?) -> $ret:ty $code:block) => {
        #[no_mangle]
        pub extern "C" fn $name ($($param: usize,)*) -> usize {
            $crate::catch_unwind(|| {
                use $crate::OcamlRep;
                $(let $param = unsafe { <$ty>::from_ocaml($param).unwrap() };)*
                $crate::to_ocaml::<$ret>(&(|| $code)())
            })
        }
    };

    (fn $name:ident() -> $ret:ty $code:block) => {
        #[no_mangle]
        pub extern "C" fn $name (_unit: usize) -> usize {
            $crate::catch_unwind(|| $crate::to_ocaml::<$ret>(&$code))
        }
    };

    (fn $name:ident($($param:ident: $ty:ty),*  $(,)?) $code:block) => {
        $crate::ocaml_ffi_fn! {
            fn $name($($param: $ty),*) -> () $code
        }
    };
}

/// Convenience macro for declaring OCaml FFI wrappers.
///
/// Each parameter will be converted from OCaml using `ocamlrep` and the result
/// will be converted to OCaml using `ocamlrep` and allocated on the OCaml GC
/// heap using `ocamlpool`.
///
/// Panics in the function body will be caught and converted to an OCaml
/// exception of type RustException.
///
/// Requires a RustException type to be registered on the OCaml side:
///
/// ```ocaml
/// exception RustException of string
/// let () =
///   Callback.register_exception "rust exception" (RustException "")
/// ```
#[macro_export]
macro_rules! ocaml_ffi {
    ($(fn $name:ident($($param:ident: $ty:ty),*  $(,)?) $(-> $ret:ty)? $code:block)*) => {
        $($crate::ocaml_ffi_fn! {
            fn $name($($param: $ty),*) $(-> $ret)* $code
        })*
    };
}

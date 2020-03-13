// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ffi::CString;
use std::panic::UnwindSafe;

use ocamlpool_rust::utils::{caml_set_field, reserve_block};
use ocamlrep::{Allocator, BlockBuilder, Value};

pub use ocamlrep::OcamlRep;

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
    static mut ocamlpool_generation: usize;
}

pub struct Pool(());

impl Pool {
    /// Prepare the ocamlpool library to allocate values directly on the OCaml
    /// runtime's garbage-collected heap.
    ///
    /// # Safety
    ///
    /// The OCaml runtime is not thread-safe, and this function will interact
    /// with it. If any other thread interacts with the OCaml runtime or
    /// ocamlpool library during the lifetime of the `Pool`, undefined behavior
    /// will result.
    #[inline(always)]
    pub unsafe fn new() -> Self {
        ocamlpool_enter();
        Self(())
    }

    #[inline(always)]
    pub fn add<T: OcamlRep>(&mut self, value: &T) -> Value<'_> {
        value.to_ocamlrep(self)
    }
}

impl Drop for Pool {
    #[inline(always)]
    fn drop(&mut self) {
        unsafe { ocamlpool_leave() };
    }
}

impl Allocator for Pool {
    #[inline(always)]
    fn generation(&self) -> usize {
        unsafe { ocamlpool_generation }
    }

    #[inline(always)]
    fn block_with_size_and_tag(&self, size: usize, tag: u8) -> BlockBuilder<'_> {
        let block = unsafe {
            let ptr = reserve_block(tag, size) as *mut Value<'_>;
            std::slice::from_raw_parts_mut(ptr, size)
        };
        BlockBuilder::new(block)
    }

    #[inline(always)]
    fn set_field<'a>(block: &mut BlockBuilder<'a>, index: usize, value: Value<'a>) {
        assert!(index < block.size());
        unsafe { caml_set_field(block.as_mut_ptr() as usize, index, value.to_bits()) };
    }
}

/// Convert the given value to an OCaml value on the OCaml runtime's
/// garbage-collected heap.
///
/// # Safety
///
/// The OCaml runtime is not thread-safe, and this function will interact with
/// it. If any other thread interacts with the OCaml runtime or ocamlpool
/// library during the execution of `to_ocaml`, undefined behavior will result.
#[inline(always)]
pub unsafe fn to_ocaml<T: OcamlRep>(value: &T) -> usize {
    let pool = Pool::new();
    let result = pool.add(value);
    result.to_bits()
}

/// Catches panics in `f` and raises a OCaml exception of type Failure
/// with the panic message (if the panic was raised with a `&str` or `String`).
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
        let msg = CString::new(msg).unwrap();
        ocaml::core::fail::caml_failwith(msg.as_ptr());
    }
    unreachable!();
}

/// Assume that some Pool exists in some parent scope. Since ocamlpool is
/// implemented with statics, we don't need a reference to that pool to write to
/// it.
///
/// # Safety
///
/// The OCaml runtime is not thread-safe, and this function will interact with
/// it. If any other thread interacts with the OCaml runtime or ocamlpool
/// library during the execution of this function, undefined behavior will
/// result.
#[inline(always)]
pub unsafe fn add_to_ambient_pool<T: OcamlRep>(value: &T) -> usize {
    let mut fake_pool = Pool(());
    let result = value.to_ocamlrep(&mut fake_pool).to_bits();
    std::mem::forget(fake_pool);
    result
}

#[macro_export]
macro_rules! ocaml_ffi_no_panic_fn {
    (fn $name:ident($($param:ident: $ty:ty),+  $(,)?) -> $ret:ty $code:block) => {
        #[no_mangle]
        pub unsafe extern "C" fn $name ($($param: usize,)*) -> usize {
            fn inner($($param: $ty,)*) -> $ret { $code }
            use $crate::OcamlRep;
            $(let $param = <$ty>::from_ocaml($param).unwrap();)*
            let result = inner($($param,)*);
            $crate::to_ocaml(&result)
        }
    };

    (fn $name:ident() -> $ret:ty $code:block) => {
        #[no_mangle]
        pub unsafe extern "C" fn $name (_unit: usize) -> usize {
            fn inner() -> $ret { $code }
            let result = inner();
            $crate::to_ocaml(&result)
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
        pub unsafe extern "C" fn $name ($($param: usize,)*) -> usize {
            $crate::catch_unwind(|| {
                fn inner($($param: $ty,)*) -> $ret { $code }
                use $crate::OcamlRep;
                $(let $param = <$ty>::from_ocaml($param).unwrap();)*
                let result = inner($($param,)*);
                $crate::to_ocaml(&result)
            })
        }
    };

    (fn $name:ident() -> $ret:ty $code:block) => {
        #[no_mangle]
        pub unsafe extern "C" fn $name (_unit: usize) -> usize {
            $crate::catch_unwind(|| {
                fn inner() -> $ret { $code }
                let result = inner();
                $crate::to_ocaml(&result)
            })
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
/// exception of type Failure.
#[macro_export]
macro_rules! ocaml_ffi {
    ($(fn $name:ident($($param:ident: $ty:ty),*  $(,)?) $(-> $ret:ty)? $code:block)*) => {
        $($crate::ocaml_ffi_fn! {
            fn $name($($param: $ty),*) $(-> $ret)* $code
        })*
    };
}

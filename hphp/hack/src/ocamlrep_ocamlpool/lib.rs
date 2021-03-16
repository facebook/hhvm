// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ffi::CString;
use std::panic::UnwindSafe;

use ocamlrep::{Allocator, BlockBuilder, MemoizationCache, OpaqueValue, ToOcamlRep};

pub use bumpalo::Bump;
pub use ocamlrep::{FromOcamlRep, FromOcamlRepIn, Value};

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
    fn ocamlpool_reserve_block(tag: u8, size: usize) -> usize;
    fn caml_failwith(msg: *const i8);
    fn caml_initialize(addr: *mut usize, value: usize);
    static ocamlpool_generation: usize;
    static ocamlpool_color: usize;
}

pub struct Pool {
    cache: MemoizationCache,
}

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
        Self {
            cache: MemoizationCache::new(),
        }
    }

    #[inline(always)]
    pub fn add<T: ToOcamlRep + ?Sized>(&mut self, value: &T) -> OpaqueValue<'_> {
        value.to_ocamlrep(self)
    }
}

impl Drop for Pool {
    #[inline(always)]
    fn drop(&mut self) {
        unsafe {
            ocamlpool_leave()
        };
    }
}

impl Allocator for Pool {
    #[inline(always)]
    fn generation(&self) -> usize {
        unsafe { ocamlpool_generation }
    }

    #[inline(always)]
    fn block_with_size_and_tag(&self, size: usize, tag: u8) -> BlockBuilder<'_> {
        let ptr = unsafe { ocamlpool_reserve_block(tag, size) as *mut OpaqueValue<'_> };
        BlockBuilder::new(ptr as usize, size)
    }

    #[inline(always)]
    fn set_field<'a>(&self, block: &mut BlockBuilder<'a>, index: usize, value: OpaqueValue<'a>) {
        assert!(index < block.size());
        unsafe {
            caml_initialize(
                (self.block_ptr_mut(block) as *mut usize).add(index),
                value.to_bits(),
            )
        };
    }

    unsafe fn block_ptr_mut<'a>(&self, block: &mut BlockBuilder<'a>) -> *mut OpaqueValue<'a> {
        block.address() as *mut _
    }

    fn memoized<'a>(
        &'a self,
        ptr: usize,
        size: usize,
        f: impl FnOnce(&'a Self) -> OpaqueValue<'a>,
    ) -> OpaqueValue<'a> {
        let bits = self.cache.memoized(ptr, size, || f(self).to_bits());
        // SAFETY: The only memoized values in the cache are those computed in
        // the closure on the previous line. Since f returns OpaqueValue<'a>, any
        // cached bits must represent a valid OpaqueValue<'a>,
        unsafe { OpaqueValue::from_bits(bits) }
    }

    fn add_root<T: ToOcamlRep + ?Sized>(&self, value: &T) -> OpaqueValue<'_> {
        self.cache.with_cache(|| value.to_ocamlrep(self))
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
///
/// # Panics
///
/// Panics upon attempts to re-enter `to_ocaml`.
#[inline(always)]
pub unsafe fn to_ocaml<T: ToOcamlRep + ?Sized>(value: &T) -> usize {
    let pool = Pool::new();
    let result = pool.add_root(value);
    result.to_bits()
}

/// Catches panics in `f` and raises a OCaml exception of type Failure
/// with the panic message (if the panic was raised with a `&str` or `String`).
pub fn catch_unwind(f: impl FnOnce() -> usize + UnwindSafe) -> usize {
    catch_unwind_with_handler(f, |msg: &str| -> Result<usize, String> { Err(msg.into()) })
}

/// Catches panics in `f` and raises a OCaml exception of type Failure
/// with the panic message (if the panic was raised with a `&str` or `String`).
/// `h` handles panic msg, it may re-raise by returning Err.
pub fn catch_unwind_with_handler(
    f: impl FnOnce() -> usize + UnwindSafe,
    h: impl FnOnce(&str) -> Result<usize, String>,
) -> usize {
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
    match h(msg) {
        Ok(value) => return value,
        Err(err) => unsafe {
            let msg = CString::new(err).unwrap();
            caml_failwith(msg.as_ptr());
        },
    }
    unreachable!();
}

/// Assume that some Pool exists in some parent scope. Since ocamlpool is
/// implemented with statics, we don't need a reference to that pool to write to
/// it.
///
/// Does not preserve sharing of values referred to by multiple references or
/// Rcs (but sharing is preserved for `ocamlrep::rc::RcOc`).
///
/// # Safety
///
/// The OCaml runtime is not thread-safe, and this function will interact with
/// it. If any other thread interacts with the OCaml runtime or ocamlpool
/// library during the execution of this function, undefined behavior will
/// result.
#[inline(always)]
pub unsafe fn add_to_ambient_pool<T: ToOcamlRep>(value: &T) -> usize {
    let mut fake_pool = Pool {
        cache: MemoizationCache::new(),
    };
    let result = value.to_ocamlrep(&mut fake_pool).to_bits();
    std::mem::forget(fake_pool);
    result
}

/// # Safety
///
/// The OCaml runtime is not thread-safe, and this function will interact with
/// it. If any other thread interacts with the OCaml runtime or ocamlpool
/// library during the execution of this function, undefined behavior will
/// result.
pub unsafe fn copy_slab_into_ocaml_heap(slab: ocamlrep::slab::SlabReader<'_>) -> usize {
    // Enter an ocamlpool region. Use `Pool` instead of `ocamlpool_enter`
    // directly so that `Pool` will invoke `ocamlpool_leave` in the event of a
    // panic (it does so in its `Drop` implementation).
    let _pool = Pool::new();

    // Allocate a block large enough for the entire slab contents and copy the
    // slab into it. Use `size - 1`, since we intend to overwrite the header.
    let size = slab.value_size_in_words();
    let block = ocamlpool_reserve_block(0, size - 1) as *mut usize;
    let block = block.sub(1);
    let block_words = std::slice::from_raw_parts_mut(block, size);
    let value = ocamlrep::slab::copy_and_rebase_value(slab, block_words);
    let value = value.to_bits();

    // Write the correct GC color to every header in the slab (else values will
    // be collected prematurely).
    let mut idx = 0;
    while idx < block_words.len() {
        let size = block_words[idx] >> 10;
        block_words[idx] |= ocamlpool_color;
        idx += size + 1;
    }

    value
}

#[macro_export]
macro_rules! ocaml_ffi_fn {
    (fn $name:ident($($param:ident: $ty:ty),+  $(,)?) -> $ret:ty $code:block) => {
        #[no_mangle]
        pub unsafe extern "C" fn $name ($($param: usize,)*) -> usize {
            $crate::catch_unwind(|| {
                fn inner($($param: $ty,)*) -> $ret { $code }
                use $crate::FromOcamlRep;
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

#[macro_export]
macro_rules! ocaml_ffi_with_arena_fn {
    (fn $name:ident<$lifetime:lifetime>($arena:ident: $arena_ty:ty, $($param:ident: $ty:ty),+ $(,)?) -> $ret:ty $code:block) => {
        #[no_mangle]
        pub unsafe extern "C" fn $name ($($param: usize,)*) -> usize {
            $crate::catch_unwind(|| {
                use $crate::FromOcamlRep;
                let arena = &$crate::Bump::new();
                fn inner<$lifetime>($arena: &$lifetime $crate::Bump, $($param: usize,)*) -> $ret {
                    $(let $param = unsafe { <$ty>::from_ocamlrep_in($crate::Value::from_bits($param), $arena).unwrap() };)*
                    $code
                }
                let result = inner(arena, $($param,)*);
                $crate::to_ocaml(&result)
            })
        }
    };

    (fn $name:ident<$lifetime:lifetime>($arena:ident: $arena_ty:ty $(,)?) -> $ret:ty $code:block) => {
        #[no_mangle]
        pub unsafe extern "C" fn $name (_unit: usize) -> usize {
            $crate::catch_unwind(|| {
                fn inner<$lifetime>($arena: &$lifetime $crate::Bump) -> $ret { $code }
                let arena = &$crate::Bump::new();
                let result = inner(arena);
                $crate::to_ocaml(&result)
            })
        }
    };

    (fn $name:ident<$lifetime:lifetime>($($param:ident: $ty:ty),* $(,)?) $code:block) => {
        $crate::ocaml_ffi_with_arena_fn! {
            fn $name<$lifetime>($($param: $ty),*) -> () $code
        }
    };
}

/// Convenience macro for declaring OCaml FFI wrappers which use an arena to
/// allocate the arguments and return value.
///
/// FFI functions declared with this macro must declare exactly one lifetime
/// parameter. The function's first value parameter must be a reference to a
/// `bumpalo::Bump` arena with that lifetime:
///
/// ```
/// ocaml_ffi_with_arena! {
///     fn swap_str_pair<'a>(arena: &'a Bump, pair: (&'a str, &'a str)) -> (&'a str, &'a str) {
///         (pair.1, pair.0)
///     }
/// }
/// ```
///
/// An OCaml extern declaration for this function would look like this:
///
/// ```
/// external swap_str_pair : string * string -> string * string = "swap_str_pair"
/// ```
///
/// Note that no parameter for the arena appears on the OCaml side--it is
/// constructed on the Rust side and lives only for the duration of one FFI
/// call.
///
/// Each (non-arena) parameter will be converted from OCaml using
/// `ocamlrep::FromOcamlRepIn`, and allocated in the given arena (if its
/// `FromOcamlRepIn` implementation makes use of the arena).
///
/// The return value (which may be allocated in the given arena, if convenient)
/// will be converted to OCaml using `ocamlrep::ToOcamlRep`. The converted OCaml
/// value will be allocated on the OCaml heap using `ocamlpool`.
///
/// Panics in the function body will be caught and converted to an OCaml
/// exception of type `Failure`.
#[macro_export]
macro_rules! ocaml_ffi_with_arena {
    ($(fn $name:ident<$lifetime:lifetime>($($param:ident: $ty:ty),* $(,)?) $(-> $ret:ty)? $code:block)*) => {
        $($crate::ocaml_ffi_with_arena_fn! {
            fn $name<$lifetime>($($param: $ty),*) $(-> $ret)* $code
        })*
    };
}

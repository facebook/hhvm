// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! FFI types for representing pointers-to-OCaml-managed-data in Rust
//! (`UnsafeOcamlPtr`) and pointers-to-Rust-managed-data in OCaml (`NakedPtr`).

use std::fmt;
use std::num::NonZeroUsize;

use crate::{Allocator, FromError, OcamlRep, Value};

/// Unsafe pointer to an OCaml value which is (possibly) managed by the garbage
/// collector.
///
/// Take care that the value stays rooted or the garbage collector does not run
/// while an UnsafeOcamlPtr wrapper for it exists.
///
/// While this can be used with an ocamlrep::Arena via to_ocamlrep, caution is
/// required--the pointed-to value will *not* be cloned into the Arena, so a
/// data structure containing UnsafeOcamlPtrs which is allocated into an Arena
/// may contain pointers into the OCaml GC-ed heap.
#[repr(transparent)]
#[derive(Clone, Copy, Hash, PartialEq, Eq)]
pub struct UnsafeOcamlPtr(NonZeroUsize);

impl UnsafeOcamlPtr {
    pub unsafe fn new(ptr: usize) -> Self {
        Self(NonZeroUsize::new(ptr).unwrap())
    }

    pub fn as_usize(self) -> usize {
        self.0.get()
    }

    /// Interpret this pointer as an OCaml value which is valid for lifetime 'a.
    /// The OCaml garbage collector must not run during this lifetime (even if
    /// the value is rooted).
    pub unsafe fn as_value<'a>(&'a self) -> Value<'a> {
        Value::from_bits(self.0.get())
    }
}

impl fmt::Debug for UnsafeOcamlPtr {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "0x{:x}", self.0)
    }
}

impl OcamlRep for UnsafeOcamlPtr {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        unsafe { Value::from_bits(self.0.get()) }
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        if value.is_immediate() {
            return Err(FromError::ExpectedBlock(value.as_int().unwrap()));
        }
        Ok(unsafe { Self::new(value.to_bits()) })
    }
}

/// Any kind of foreign pointer (i.e., a pointer to any data at all--it need not
/// look like a valid OCaml value).
///
/// On the OCaml side, these are represented as opaque types, e.g. `type addr;`.
///
/// The pointer must not be within a memory page currently in use by the OCaml
/// runtime for the garbage-collected heap (i.e., it must in fact be a foreign
/// pointer).
///
/// Can only be used when linking against a binary built with an OCaml compiler
/// which was **not** configured with the `-no-naked-pointers` option (which
/// forbids naked pointers, requiring foreign pointers to be wrapped in a block
/// tagged with `Abstract_tag` instead).
#[repr(transparent)]
#[derive(Clone, Copy, Hash, PartialEq, Eq)]
pub struct NakedPtr<T>(*const T);

impl<T> NakedPtr<T> {
    pub fn new(ptr: *const T) -> Self {
        Self(ptr)
    }

    pub fn as_ptr(self) -> *const T {
        self.0
    }
}

impl<T> fmt::Debug for NakedPtr<T> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{:p}", self.0)
    }
}

impl<T> OcamlRep for NakedPtr<T> {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> Value<'a> {
        unsafe { Value::from_bits(self.0 as usize) }
    }

    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        if value.is_immediate() {
            return Err(FromError::ExpectedBlock(value.as_int().unwrap()));
        }
        Ok(Self::new(value.to_bits() as *const T))
    }
}

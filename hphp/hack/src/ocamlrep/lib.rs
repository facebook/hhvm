// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod arena;
mod block;
mod error;
mod impls;
mod slab;
mod value;

pub mod from;
pub mod rc;

pub use arena::Arena;
pub use block::Block;
pub use error::{FromError, SlabIntegrityError};
pub use impls::{bytes_from_ocamlrep, bytes_to_ocamlrep, str_from_ocamlrep, str_to_ocamlrep};
pub use slab::OwnedSlab;
pub use value::{OpaqueValue, Value};

pub trait OcamlRep: Sized {
    fn to_ocamlrep<'a, A: Allocator<'a>>(&self, alloc: &mut A) -> Value<'a>;
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError>;

    unsafe fn from_ocaml(value: usize) -> Result<Self, FromError> {
        Self::from_ocamlrep(Value::from_bits(value))
    }
}

pub trait Allocator<'a>: Sized {
    /// Return a token which uniquely identifies this Allocator. The token must
    /// be unique (among all instances of all implementors of the Allocator
    /// trait), and an instance of an implementor of Allocator must return the
    /// same token throughout its entire lifetime.
    fn generation(&self) -> usize;

    /// Allocate space a block with enough space for `size` fields, write its
    /// header, and return a pointer to the first field. This pointer is
    /// suitable to be passed to `Allocator::set_field` and `Value::from_ptr`
    /// (and implementors of `to_ocamlrep` are expected to do so with pointers
    /// returned by this method).
    fn block_with_size_and_tag(&mut self, size: usize, tag: u8) -> *mut Value<'a>;

    /// This method is marked unsafe because there is no bounds checking for
    /// this write. It requires that it requires that `block` points to is the
    /// first field of a block allocated by an `Allocator` method and that the
    /// index is strictly less than the size of the block.
    unsafe fn set_field(block: *mut Value<'a>, index: usize, value: Value<'a>);

    #[inline(always)]
    fn block_with_size(&mut self, size: usize) -> *mut Value<'a> {
        self.block_with_size_and_tag(size, 0)
    }

    #[inline(always)]
    fn add<T: OcamlRep>(&mut self, value: &T) -> Value<'a> {
        value.to_ocamlrep(self)
    }
}

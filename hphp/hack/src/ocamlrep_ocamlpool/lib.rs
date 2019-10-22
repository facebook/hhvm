// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::marker::PhantomData;

use ocamlpool_rust::utils::{caml_set_field, reserve_block};
use ocamlrep::{Allocator, OcamlRep, Value};

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

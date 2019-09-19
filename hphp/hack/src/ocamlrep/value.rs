// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::{self, Debug};
use std::marker::PhantomData;
use std::mem;

use crate::block::Block;

#[repr(transparent)]
#[derive(Clone, Copy)]
pub struct Value<'arena>(pub(crate) usize, PhantomData<&'arena ()>);

impl<'a> Value<'a> {
    pub fn is_immediate(&self) -> bool {
        self.0 & 1 == 1
    }

    pub fn int(value: isize) -> Value<'static> {
        Value(((value as usize) << 1) | 1, PhantomData)
    }

    pub(crate) fn bits(value: usize) -> Value<'a> {
        Value(value, PhantomData)
    }

    pub fn as_int(&self) -> Option<isize> {
        if self.is_immediate() {
            Some((self.0 as isize) >> 1)
        } else {
            None
        }
    }

    pub fn as_block(&self) -> Option<Block<'a>> {
        if self.is_immediate() {
            return None;
        }
        let block = unsafe {
            let ptr: *const Value = mem::transmute(self.0);
            let header = ptr.offset(-1);
            let size = ((*header).0 >> 10) + 1;
            std::slice::from_raw_parts(header, size)
        };
        Some(Block(block))
    }

    /// This method is unsafe because it decouples the value from the lifetime
    /// of the arena. Take care that the returned value does not outlive the
    /// arena.
    pub unsafe fn as_usize(&self) -> usize {
        self.0
    }
}

impl Debug for Value<'_> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self.as_block() {
            None => write!(f, "{}", self.as_int().unwrap()),
            Some(block) => write!(f, "{:?}", block),
        }
    }
}

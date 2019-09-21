// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::{self, Debug};
use std::marker::PhantomData;

use crate::block::{Block, Header};

fn is_ocaml_int(value: usize) -> bool {
    value & 1 == 1
}

fn isize_to_ocaml_int(value: isize) -> usize {
    ((value as usize) << 1) | 1
}

fn ocaml_int_to_isize(value: usize) -> isize {
    (value as isize) >> 1
}

#[repr(transparent)]
#[derive(Clone, Copy)]
pub struct Value<'arena>(pub(crate) usize, PhantomData<&'arena ()>);

impl<'a> Value<'a> {
    pub fn is_immediate(self) -> bool {
        is_ocaml_int(self.0)
    }

    pub fn int(value: isize) -> Value<'static> {
        Value(isize_to_ocaml_int(value), PhantomData)
    }

    pub fn as_int(self) -> Option<isize> {
        if self.is_immediate() {
            Some(ocaml_int_to_isize(self.0))
        } else {
            None
        }
    }

    pub fn as_block(self) -> Option<Block<'a>> {
        if self.is_immediate() {
            return None;
        }
        let block = unsafe {
            let ptr = self.0 as *const Value;
            let header = ptr.offset(-1);
            let size = Header::from_bits((*header).to_bits()).size() + 1;
            std::slice::from_raw_parts(header, size)
        };
        Some(Block(block))
    }

    pub unsafe fn from_bits(value: usize) -> Value<'a> {
        Value(value, PhantomData)
    }

    /// This method is unsafe because it decouples the value from the lifetime
    /// of its containing arena or slab. Take care that the returned value does
    /// not outlive the arena.
    pub unsafe fn to_bits(self) -> usize {
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

#[repr(transparent)]
#[derive(Clone, Copy)]
pub struct OpaqueValue<'arena>(usize, PhantomData<&'arena ()>);

impl<'a> OpaqueValue<'a> {
    pub(crate) fn is_immediate(self) -> bool {
        is_ocaml_int(self.0)
    }

    fn as_int(self) -> Option<isize> {
        if self.is_immediate() {
            Some(ocaml_int_to_isize(self.0))
        } else {
            None
        }
    }

    pub(crate) fn as_header(self) -> Header {
        Header::from_bits(self.0)
    }

    pub(crate) unsafe fn from_bits(value: usize) -> OpaqueValue<'a> {
        OpaqueValue(value, PhantomData)
    }

    pub(crate) unsafe fn to_bits(self) -> usize {
        self.0
    }

    pub(crate) unsafe fn add_ptr_offset(&mut self, diff: isize) {
        if !self.is_immediate() {
            self.0 = (self.0 as isize + diff) as usize;
        }
    }
}

impl Debug for OpaqueValue<'_> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self.as_int() {
            Some(x) => write!(f, "{}", x),
            None => write!(f, "0x{:x}", self.0),
        }
    }
}

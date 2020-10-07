// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ocamlrep::{from, Allocator, FromError, FromOcamlRep, OpaqueValue, ToOcamlRep, Value};

#[derive(Clone, Copy, Debug, Hash, PartialEq, Eq, PartialOrd, Ord)]
pub struct Dep(u64);

impl Dep {
    pub fn new(x: u64) -> Self {
        Dep(x)
    }

    pub fn is_class(self) -> bool {
        self.0 & (1 << 62) != 0
    }

    pub fn class_to_extends(self) -> Option<Self> {
        if !self.is_class() {
            None
        } else {
            Some(Dep(self.0 & ((1 << 62) - 1)))
        }
    }
}

impl Into<u64> for Dep {
    fn into(self) -> u64 {
        self.0
    }
}

impl FromOcamlRep for Dep {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let x: isize = from::expect_int(value)?;
        // In Rust, a numeric cast between two integers of the same size
        // is a no-op. We require a 64-bit word size.
        let x = x as u64;
        // The conversion from an OCaml integer to a Rust integer involves a
        // right arithmetic bitshift. Due to sign extension, if the OCaml
        // integer was negative, the resulting Rust integer's MSB will be set.
        // We don't want that, because we are disguising unsigned 63-bit Rust
        // integers as signed 63-bit integers in OCaml.
        let x = x & !(1 << 63);
        Ok(Dep(x))
    }
}

impl ToOcamlRep for Dep {
    fn to_ocamlrep<'a, A: Allocator>(&self, _alloc: &'a A) -> OpaqueValue<'a> {
        let x: u64 = self.0;
        // In Rust, a numeric cast between two integers of the same size
        // is a no-op. We require a 64-bit word size.
        let x = x as isize;
        OpaqueValue::int(x)
    }
}

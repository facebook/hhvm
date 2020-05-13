// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![deny(clippy::mut_from_ref)]

use bumpalo::Bump;

pub trait Arena {
    #[allow(clippy::mut_from_ref)]
    fn alloc<T: TrivialDrop>(&self, val: T) -> &mut T;
}

impl Arena for Bump {
    #[allow(clippy::mut_from_ref)]
    #[inline(always)]
    fn alloc<T: TrivialDrop>(&self, val: T) -> &mut T {
        self.alloc(val)
    }
}

/// Marker trait for types whose implementation of `Drop` is a no-op.
///
/// Used to denote types which can be moved into an arena (which does not drop
/// its contents) without leaking memory.
///
/// Must not be implemented for any type which owns heap memory or otherwise
/// needs to run cleanup in its `Drop` implementation (e.g., `Box`, `Vec`,
/// `std::fs::File`, etc.), or any type containing a field with such a
/// nontrivial `Drop` implementation.
///
/// All `Copy` types implement `TrivialDrop` via a blanket implementation.
pub trait TrivialDrop {}
impl<T: Copy> TrivialDrop for T {}

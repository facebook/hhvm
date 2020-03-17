// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![deny(clippy::mut_from_ref)]

use bumpalo::Bump;

pub trait Arena {
    fn alloc<T>(&self, val: T) -> &mut T;
}

impl Arena for Bump {
    #![deny(clippy::mut_from_ref)]
    #[inline(always)]
    fn alloc<T>(&self, val: T) -> &mut T {
        self.alloc(val)
    }
}

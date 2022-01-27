// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod alloc_bytes;
mod alloc_decl_defs;
mod alloc_pos;
mod alloc_reason;
mod alloc_typing_defs;

use lazy_static::lazy_static;

use hcons::Conser;

use crate::decl_defs::{DeclTy, DeclTy_};
use crate::reason::{BReason, NReason, Reason};
use crate::typing_defs::{Ty, Ty_};

pub struct GlobalAllocator {
    bytes: Conser<[u8]>,
}

pub struct Allocator<R: Reason> {
    global: &'static GlobalAllocator,
    decl_tys: Conser<DeclTy_<R, DeclTy<R>>>,
    typing_tys: Conser<Ty_<R, Ty<R>>>,
}

/// Get references to the allocator singletons. Please use this in the main
/// function only.
pub fn get_allocators_for_main() -> (
    &'static GlobalAllocator,
    &'static Allocator<NReason>,
    &'static Allocator<BReason>,
) {
    (&GLOBAL_ALLOC, &NO_REASON_ALLOC, &BOX_REASON_ALLOC)
}

lazy_static! {
    static ref GLOBAL_ALLOC: &'static GlobalAllocator = Box::leak(Box::new(GlobalAllocator::new()));
}

lazy_static! {
    static ref NO_REASON_ALLOC: &'static Allocator<NReason> =
        Box::leak(Box::new(Allocator::new(&GLOBAL_ALLOC)));
}

lazy_static! {
    static ref BOX_REASON_ALLOC: &'static Allocator<BReason> =
        Box::leak(Box::new(Allocator::new(&GLOBAL_ALLOC)));
}

impl GlobalAllocator {
    fn new() -> Self {
        Self {
            bytes: Conser::new(),
        }
    }
}

impl<R: Reason> Allocator<R> {
    fn new(global: &'static GlobalAllocator) -> Self {
        Self {
            global,
            decl_tys: Conser::new(),
            typing_tys: Conser::new(),
        }
    }
}

impl std::fmt::Debug for GlobalAllocator {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("GlobalAllocator").finish()
    }
}

impl<T: Reason> std::fmt::Debug for Allocator<T> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Allocator").finish()
    }
}

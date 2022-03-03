// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod alloc_decl_defs;

use std::marker::PhantomData;

use lazy_static::lazy_static;

use crate::reason::{BReason, NReason, Reason};

pub struct Allocator<R: Reason>(PhantomData<R>);

/// Get references to the allocator singletons. Please use this in the main
/// function only.
pub fn get_allocators_for_main() -> (&'static Allocator<NReason>, &'static Allocator<BReason>) {
    (&NO_REASON_ALLOC, &BOX_REASON_ALLOC)
}

lazy_static! {
    static ref NO_REASON_ALLOC: &'static Allocator<NReason> = Box::leak(Box::new(Allocator::new()));
}

lazy_static! {
    static ref BOX_REASON_ALLOC: &'static Allocator<BReason> =
        Box::leak(Box::new(Allocator::new()));
}

impl<R: Reason> Allocator<R> {
    fn new() -> Self {
        Self(PhantomData)
    }
}

impl<T: Reason> std::fmt::Debug for Allocator<T> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Allocator").finish()
    }
}

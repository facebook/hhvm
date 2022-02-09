// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use crate::alloc::Allocator;
use crate::reason::Reason;
use crate::special_names::SpecialNames;
use crate::typing_decl_provider::TypingDeclProvider;

#[derive(Debug)]
pub struct TypingCtx<R: Reason> {
    pub alloc: &'static Allocator<R>,
    pub typing_decl_provider: Arc<TypingDeclProvider<R>>,
    pub special_names: &'static SpecialNames,
}

impl<R: Reason> TypingCtx<R> {
    pub fn new(
        alloc: &'static Allocator<R>,
        typing_decl_provider: Arc<TypingDeclProvider<R>>,
        special_names: &'static SpecialNames,
    ) -> Self {
        Self {
            alloc,
            typing_decl_provider,
            special_names,
        }
    }
}

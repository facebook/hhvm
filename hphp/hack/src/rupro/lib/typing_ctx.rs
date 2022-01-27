// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::rc::Rc;

use crate::alloc::Allocator;
use crate::folded_decl_provider::FoldedDeclProvider;
use crate::reason::Reason;
use crate::sn_provider::SpecialNamesProvider;
use crate::typing_decl_provider::TypingDeclProvider;

#[derive(Debug)]
pub struct TypingCtx<R: Reason> {
    pub alloc: &'static Allocator<R>,
    pub folded_decl_provider: Rc<FoldedDeclProvider<R>>,
    pub typing_decl_provider: Rc<TypingDeclProvider<R>>,
    pub special_names: Rc<SpecialNamesProvider>,
}

impl<R: Reason> TypingCtx<R> {
    pub fn new(
        alloc: &'static Allocator<R>,
        folded_decl_provider: Rc<FoldedDeclProvider<R>>,
        typing_decl_provider: Rc<TypingDeclProvider<R>>,
        special_names: Rc<SpecialNamesProvider>,
    ) -> Self {
        Self {
            alloc,
            folded_decl_provider,
            typing_decl_provider,
            special_names,
        }
    }
}

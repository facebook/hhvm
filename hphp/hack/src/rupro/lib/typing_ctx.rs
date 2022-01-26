// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::rc::Rc;

use crate::ast_provider::AstProvider;
use crate::decl_ty_provider::DeclTyProvider;
use crate::folded_decl_provider::FoldedDeclProvider;
use crate::pos_provider::PosProvider;
use crate::reason::Reason;
use crate::sn_provider::SpecialNamesProvider;
use crate::typing_decl_provider::TypingDeclProvider;
use crate::typing_ty_provider::TypingTyProvider;

#[derive(Debug)]
pub struct TypingCtx<R: Reason> {
    pub pos_provider: Rc<PosProvider>,
    pub decl_ty_provider: Rc<DeclTyProvider<R>>,
    pub folded_decl_provider: Rc<FoldedDeclProvider<R>>,
    pub typing_decl_provider: Rc<TypingDeclProvider<R>>,
    pub typing_ty_provider: Rc<TypingTyProvider<R>>,
    pub ast_provider: Rc<AstProvider>,
    pub special_names: Rc<SpecialNamesProvider>,
}

impl<R: Reason> TypingCtx<R> {
    pub fn new(
        typing_decl_provider: Rc<TypingDeclProvider<R>>,
        typing_ty_provider: Rc<TypingTyProvider<R>>,
        ast_provider: Rc<AstProvider>,
    ) -> Self {
        let folded_decl_provider = Rc::clone(typing_decl_provider.get_folded_decl_provider());
        let shallow_decl_provider = Rc::clone(folded_decl_provider.get_shallow_decl_provider());
        let decl_ty_provider = Rc::clone(shallow_decl_provider.get_decl_ty_provider());
        let pos_provider = Rc::clone(decl_ty_provider.get_pos_provider());
        let special_names = Rc::new(SpecialNamesProvider::new(Rc::clone(&pos_provider)));
        Self {
            pos_provider,
            decl_ty_provider,
            folded_decl_provider,
            typing_decl_provider,
            typing_ty_provider,
            ast_provider,
            special_names,
        }
    }
}

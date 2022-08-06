// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

use ty::reason::Reason;

use crate::typing_decl_provider::TypingDeclProvider;

#[derive(Debug)]
pub struct TypingCtx<R: Reason> {
    pub typing_decl_provider: Rc<dyn TypingDeclProvider<R>>,
}

impl<R: Reason> TypingCtx<R> {
    pub fn new(typing_decl_provider: Rc<dyn TypingDeclProvider<R>>) -> Self {
        Self {
            typing_decl_provider,
        }
    }
}

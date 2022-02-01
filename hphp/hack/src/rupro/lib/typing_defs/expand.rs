// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]
use crate::reason::{Reason, ReasonImpl};
use crate::typing_ctx::TypingCtx;
use crate::typing_defs::{Ty, Ty_};
use crate::typing_error::ReasonsCallback;
use hcons::Hc;
use pos::{Symbol, SymbolMap};

pub type TypeExpansion<R> = (<R as Reason>::Pos, Symbol);

pub struct TypeExpansions<R: Reason> {
    report_cycle: Option<TypeExpansion<R>>,
    expansions: Vec<TypeExpansion<R>>,
}

pub struct ExpandEnv<'a, R: Reason> {
    type_expansions: TypeExpansions<R>,
    expand_visible_newtype: bool,
    substs: SymbolMap<Ty<R>>,
    this_ty: Ty<R>,
    on_error: ReasonsCallback<'a, R>,
}

impl<R: Reason> TypeExpansions<R> {
    pub fn with_report_cycle(report_cycle: Option<(R::Pos, Symbol)>) -> Self {
        TypeExpansions {
            report_cycle,
            expansions: Vec::new(),
        }
    }

    pub fn new() -> Self {
        Self::with_report_cycle(None)
    }
}

impl<'a, R: Reason> ExpandEnv<'a, R> {
    pub fn new(ctx: &TypingCtx<R>) -> Self {
        Self {
            type_expansions: TypeExpansions::new(),
            expand_visible_newtype: true,
            substs: Default::default(),
            this_ty: ctx.alloc.ty(
                R::mk(&|| ReasonImpl::Rnone),
                Ty_::Tgeneric(Hc::clone(&ctx.special_names.special_idents.this), vec![]),
            ),
            on_error: ReasonsCallback::new(&|| ReasonsCallback::ignore()),
        }
    }

    pub fn set_type_expansions(&mut self, exp: TypeExpansions<R>) -> &mut Self {
        self.type_expansions = exp;
        self
    }

    pub fn set_on_error(&mut self, on_error: ReasonsCallback<'a, R>) -> &mut Self {
        self.on_error = on_error;
        self
    }

    pub fn with_expand_visible_newtype<T>(&mut self, f: impl FnOnce(&mut Self) -> T) -> T {
        let prev_expand_visible_newtype = self.expand_visible_newtype;
        self.expand_visible_newtype = true;
        let r = f(self);
        self.expand_visible_newtype = prev_expand_visible_newtype;
        r
    }
}

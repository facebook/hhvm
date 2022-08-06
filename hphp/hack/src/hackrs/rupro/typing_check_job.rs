// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;

use oxidized::nast;
use ty::reason::Reason;

use crate::errors::HackError;
use crate::tast;
use crate::typing::typing_error::Result;
use crate::typing_ctx::TypingCtx;
use crate::typing_toplevel::TypingToplevel;

pub struct TypingCheckJob;

impl TypingCheckJob {
    pub fn type_fun<R: Reason>(
        ctx: Rc<TypingCtx<R>>,
        ast: &nast::FunDef,
    ) -> Result<(tast::Def<R>, Vec<HackError<R>>)> {
        let (def, typing_errors) = TypingToplevel::fun_def(ctx, ast)?;
        let def = oxidized::aast::Def::Fun(Box::new(def));
        Ok((def, typing_errors.into_iter().map(Into::into).collect()))
    }

    pub fn type_class<R: Reason>(
        ctx: Rc<TypingCtx<R>>,
        ast: &nast::Class_,
    ) -> Result<(tast::Def<R>, Vec<HackError<R>>)> {
        let (def, typing_errors) = TypingToplevel::class_def(ctx, ast)?;
        let def = oxidized::aast::Def::Class(Box::new(def));
        Ok((def, typing_errors.into_iter().map(Into::into).collect()))
    }

    pub fn type_typedef<R: Reason>(
        _ctx: Rc<TypingCtx<R>>,
        _ast: &nast::Typedef,
    ) -> Result<(tast::Def<R>, Vec<HackError<R>>)> {
        todo!()
    }

    pub fn type_const<R: Reason>(
        _ctx: Rc<TypingCtx<R>>,
        _ast: &nast::Gconst,
    ) -> Result<(tast::Def<R>, Vec<HackError<R>>)> {
        todo!()
    }
}

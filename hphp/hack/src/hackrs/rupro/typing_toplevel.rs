// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::rc::Rc;

use ty::local_error::TypingError;
use ty::reason::Reason;

use crate::tast;
use crate::typing::ast::Infer;
use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Result;
use crate::typing_ctx::TypingCtx;

pub struct TypingToplevel;

impl TypingToplevel {
    pub fn fun_def<R: Reason>(
        ctx: Rc<TypingCtx<R>>,
        fd: &oxidized::aast::FunDef<(), ()>,
    ) -> Result<(tast::FunDef<R>, Vec<TypingError<R>>)> {
        let mut env = TEnv::fun_env(ctx, fd);
        let def = fd.infer(&mut env, ())?;
        Ok((def, env.destruct()))
    }

    pub fn class_def<R: Reason>(
        ctx: Rc<TypingCtx<R>>,
        cd: &oxidized::aast::Class_<(), ()>,
    ) -> Result<(tast::Class_<R>, Vec<TypingError<R>>)> {
        rupro_todo_mark!(Modules);
        rupro_todo_mark!(Dynamic);
        rupro_todo_mark!(MissingError, "decl errors");
        rupro_todo_mark!(MissingError, "duplicate error");
        let mut env = TEnv::class_env(ctx, cd);
        let def = cd.infer(&mut env, ())?;
        Ok((def, env.destruct()))
    }
}

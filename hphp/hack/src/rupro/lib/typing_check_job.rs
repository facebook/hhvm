// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::rc::Rc;

use crate::errors::HackError;
use crate::pos::RelativePath;
use crate::reason::Reason;
use crate::typing_ctx::TypingCtx;
use crate::typing_toplevel::TypingToplevel;

use crate::tast;

pub struct TypingCheckJob;

impl TypingCheckJob {
    pub fn type_fun<R: Reason>(
        ctx: Rc<TypingCtx<R>>,
        fln: &RelativePath,
        x: &str,
    ) -> (Option<tast::Def<R>>, Vec<HackError<R>>) {
        let mut errors = Vec::new();
        let def = match ctx.ast_provider.find_fun_in_file(true, fln, x) {
            Ok(None) => None,
            Err(e) => {
                errors.push(e.into());
                None
            }
            Ok(Some(ast)) => {
                errors.extend(ast.as_owner().1.iter().map(|e| e.clone().into()));
                let (def, typing_errors) = TypingToplevel::fun_def(ctx.clone(), &*ast);
                errors.extend(typing_errors.into_iter().map(|e| e.into()));
                Some(oxidized::aast::Def::Fun(Box::new(def)))
            }
        };
        (def, errors)
    }

    pub fn type_class<R: Reason>(
        _ctx: Rc<TypingCtx<R>>,
        _fln: &RelativePath,
        _x: &str,
    ) -> (Option<tast::Def<R>>, Vec<HackError<R>>) {
        todo!()
    }

    pub fn type_record_def<R: Reason>(
        _ctx: Rc<TypingCtx<R>>,
        _fln: &RelativePath,
        _x: &str,
    ) -> (Option<tast::Def<R>>, Vec<HackError<R>>) {
        todo!()
    }

    pub fn type_typedef<R: Reason>(
        _ctx: Rc<TypingCtx<R>>,
        _fln: &RelativePath,
        _x: &str,
    ) -> (Option<tast::Def<R>>, Vec<HackError<R>>) {
        todo!()
    }

    pub fn type_const<R: Reason>(
        _ctx: Rc<TypingCtx<R>>,
        _fln: &RelativePath,
        _x: &str,
    ) -> (Option<tast::Def<R>>, Vec<HackError<R>>) {
        todo!()
    }
}

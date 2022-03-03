// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use oxidized::{aast, nast};

use crate::errors::HackError;
use crate::parsing_error::ParsingError;
use crate::reason::Reason;
use crate::typing::Result;
use crate::typing_check_job::TypingCheckJob;
use crate::typing_ctx::TypingCtx;

use crate::tast;

pub struct TypingCheckUtils;

impl TypingCheckUtils {
    pub fn type_file<R: Reason>(
        ctx: Arc<TypingCtx<R>>,
        ast: &nast::Program,
        parsing_errors: Vec<ParsingError>,
    ) -> Result<(tast::Program<R>, Vec<HackError<R>>)> {
        use aast::Def;
        let mut tast = vec![];
        let mut errs = vec![];
        errs.extend(parsing_errors.into_iter().map(Into::into));
        let mut accumulate = |(new_def, new_errs)| {
            tast.push(new_def);
            errs.extend(new_errs);
        };
        for def in ast.defs() {
            let ctx = Arc::clone(&ctx);
            match def {
                Def::Fun(f) => accumulate(TypingCheckJob::type_fun(ctx, &*f)?),
                Def::Class(c) => accumulate(TypingCheckJob::type_class(ctx, &*c)?),
                Def::Typedef(t) => accumulate(TypingCheckJob::type_typedef(ctx, &*t)?),
                Def::Constant(cst) => accumulate(TypingCheckJob::type_const(ctx, &*cst)?),
                Def::Namespace(_)
                | Def::Stmt(_)
                | Def::Module(_)
                | Def::NamespaceUse(_)
                | Def::SetNamespaceEnv(_)
                | Def::FileAttributes(_) => unreachable!("Program::defs won't emit these"),
            }
        }
        Ok((aast::Program(tast), errs))
    }
}

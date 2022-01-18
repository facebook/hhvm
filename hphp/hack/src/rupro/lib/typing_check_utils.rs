// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::rc::Rc;

use crate::errors::HackError;
use crate::naming::FileInfo;
use crate::pos::RelativePath;
use crate::reason::Reason;
use crate::typing_check_job::TypingCheckJob;
use crate::typing_ctx::TypingCtx;

use crate::tast;

pub struct TypingCheckUtils;

impl TypingCheckUtils {
    pub fn type_file<R: Reason>(
        ctx: Rc<TypingCtx<R>>,
        fln: &RelativePath,
        fi: &FileInfo,
    ) -> (Vec<tast::Def<R>>, Vec<HackError<R>>) {
        let mut defs = vec![];
        let mut errs = vec![];
        let mut accumulate = |(new_def, new_errs)| {
            if let Some(new_def) = new_def {
                defs.push(new_def);
            }
            errs.extend(new_errs);
        };
        fi.funs
            .iter()
            .for_each(|x| accumulate(TypingCheckJob::type_fun(ctx.clone(), fln, &x.id)));
        fi.classes
            .iter()
            .for_each(|x| accumulate(TypingCheckJob::type_class(ctx.clone(), fln, &x.id)));
        fi.record_defs
            .iter()
            .for_each(|x| accumulate(TypingCheckJob::type_record_def(ctx.clone(), fln, &x.id)));
        fi.typedefs
            .iter()
            .for_each(|x| accumulate(TypingCheckJob::type_typedef(ctx.clone(), fln, &x.id)));
        fi.consts
            .iter()
            .for_each(|x| accumulate(TypingCheckJob::type_const(ctx.clone(), fln, &x.id)));
        (defs, errs)
    }
}

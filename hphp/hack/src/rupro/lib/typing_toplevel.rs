// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]
use std::rc::Rc;

use pos::Symbol;

use crate::decl_defs::DeclTy;
use crate::decl_hint::DeclHintEnv;
use crate::reason::Reason;
use crate::typing::{BindParamFlags, Result, Typing};
use crate::typing_ctx::TypingCtx;
use crate::typing_env::TEnv;
use crate::typing_error::{Primary, TypingError};
use crate::typing_param::TypingParam;
use crate::typing_phase::Phase;
use crate::typing_return::TypingReturn;

use crate::tast;

pub struct TypingToplevel<'a, R: Reason> {
    ctx: Rc<TypingCtx<R>>,
    env: &'a TEnv<R>,
}

impl<'a, R: Reason> TypingToplevel<'a, R> {
    fn decl_hint_env(&self) -> DeclHintEnv<R> {
        DeclHintEnv::new()
    }

    fn hint_fun_header<'ast>(
        &self,
        params: &'ast [oxidized::aast::FunParam<(), ()>],
        ret: &oxidized::aast::TypeHint<()>,
    ) -> (
        Option<DeclTy<R>>,
        Vec<(&'ast oxidized::aast::FunParam<(), ()>, Option<DeclTy<R>>)>,
    ) {
        let ret = ret.1.as_ref().map(|h| self.decl_hint_env().hint(h));
        let params = params
            .iter()
            .map(|fp| {
                (
                    fp,
                    fp.type_hint
                        .1
                        .as_ref()
                        .map(|h| self.decl_hint_env().hint(h)),
                )
            })
            .collect();
        (ret, params)
    }

    fn fun_def_impl(&mut self, fd: &oxidized::aast::FunDef<(), ()>) -> Result<tast::FunDef<R>> {
        let f = &fd.fun;
        let fname = Symbol::new(&f.name.1);
        let fpos = R::Pos::from(&f.name.0);

        let (return_decl_ty, params_decl_ty) = self.hint_fun_header(&f.params, &f.ret);
        let return_ty = match return_decl_ty.clone() {
            None => TypingReturn::make_default_return(self.env, false, &fpos, fname),
            Some(ty) => TypingReturn::make_return_type(
                self.env,
                |env, ty| Phase::localize_no_subst(env, false, None, ty),
                ty,
            )?,
        };
        let ret_pos = match &f.ret.1 {
            Some(h) => R::Pos::from(&h.0),
            None => R::Pos::from(f.name.pos()),
        };
        let return_ = TypingReturn::make_info(
            self.env,
            ret_pos,
            &f.fun_kind,
            &f.user_attributes,
            f.ret.1.is_some(),
            return_ty.clone(),
            return_decl_ty,
        );
        let param_tys: Vec<_> = params_decl_ty
            .iter()
            .map(|(param, hint)| {
                Ok((
                    param,
                    TypingParam::make_param_local_ty(self.env, hint.clone(), param)?,
                ))
            })
            .collect::<Result<Vec<_>>>()?;
        param_tys
            .iter()
            .for_each(|(param, ty)| TypingParam::check_param_has_hint(self.env, param, ty));
        // TOOD(hrust): capabilities, immutable
        let typed_params: Vec<_> = param_tys
            .iter()
            .map(|(param, ty)| {
                Typing::bind_param(
                    self.env,
                    BindParamFlags {
                        immutable: false,
                        can_read_globals: true,
                    },
                    ty.clone(),
                    param,
                )
            })
            .collect();
        // TODO(variance)
        // TODO(memoize)
        let tb = Typing::fun_(
            self.env,
            Default::default(),
            return_,
            fpos.clone(),
            &f.body,
            &f.fun_kind,
        );
        match f.ret.1 {
            Some(_) => {}
            None => self
                .env
                .add_error(TypingError::primary(Primary::ExpectingReturnTypeHint(fpos))),
        }
        let fun = oxidized::aast::Fun_ {
            annotation: self.env.save(()),
            readonly_this: f.readonly_this.clone(),
            span: f.span.clone(),
            readonly_ret: f.readonly_ret.clone(),
            ret: oxidized::aast::TypeHint(return_ty, f.ret.1.clone()),
            name: f.name.clone(),
            tparams: vec![],
            where_constraints: vec![],
            params: typed_params,
            ctxs: f.ctxs.clone(),
            unsafe_ctxs: f.unsafe_ctxs.clone(),
            fun_kind: f.fun_kind.clone(),
            user_attributes: vec![],
            body: oxidized::aast::FuncBody { fb_ast: tb },
            external: f.external.clone(),
            doc_comment: f.doc_comment.clone(),
        };
        Ok(oxidized::aast::FunDef {
            namespace: fd.namespace.clone(),
            file_attributes: vec![],
            mode: fd.mode,
            fun,
        })
    }

    pub fn fun_def(
        ctx: Rc<TypingCtx<R>>,
        fd: &oxidized::aast::FunDef<(), ()>,
    ) -> Result<(tast::FunDef<R>, Vec<TypingError<R>>)> {
        let env = TEnv::fun_env(Rc::clone(&ctx), fd);
        let def = TypingToplevel { ctx, env: &env }.fun_def_impl(fd)?;
        Ok((def, env.destruct()))
    }
}

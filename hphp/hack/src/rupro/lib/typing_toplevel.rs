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
use crate::tast;
use crate::typing::{BindParamFlags, Result, Typing};
use crate::typing_ctx::TypingCtx;
use crate::typing_decl_provider::Class;
use crate::typing_defs::{ExpandEnv, Ty};
use crate::typing_env::TEnv;
use crate::typing_error::{Primary, TypingError};
use crate::typing_param::{TypingParam, TypingParamFlags};
use crate::typing_phase::Phase;
use crate::typing_return::{TypingReturn, TypingReturnInfo};
use pos::TypeName;

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
        let param_tys = TypingParam::make_param_local_tys(
            TypingParamFlags {
                dynamic_mode: false,
            },
            self.env,
            params_decl_ty.into_iter(),
        )?;
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

    fn method_return(
        &mut self,
        pos: &R::Pos,
        name: Symbol,
        m: &oxidized::aast::Method_<(), ()>,
        ret_decl_ty: Option<DeclTy<R>>,
    ) -> Result<(TypingReturnInfo<R>, Ty<R>)> {
        let ret_ty = match ret_decl_ty.clone() {
            None => TypingReturn::make_default_return(self.env, false, pos, name),
            Some(ty) => {
                // TODO(hrust): empty_expand_env_with_on_error
                TypingReturn::make_return_type(
                    self.env,
                    |env, ty| Phase::localize(env, &mut ExpandEnv::new(&env.ctx), ty),
                    ty,
                )?
            }
        };
        // TODO(hrust): force_return_kind
        let ret_pos = match &m.ret.1 {
            Some(h) => R::Pos::from(&h.0),
            None => R::Pos::from(m.name.pos()),
        };
        let return_ = TypingReturn::make_info(
            self.env,
            ret_pos,
            &m.fun_kind,
            &m.user_attributes,
            m.ret.1.is_some(),
            ret_ty.clone(),
            ret_decl_ty,
        );
        Ok((return_, ret_ty))
    }

    fn class_var_def(
        &mut self,
        _is_static: bool,
        _cls: &dyn Class<R>,
        cv: &oxidized::aast::ClassVar<(), ()>,
    ) -> Result<tast::ClassVar<R>> {
        // TODO(hrust): missing type hint
        let decl_cty = self.decl_hint_env().hint(cv.type_.1.as_ref().unwrap());
        // TODO(hrust): enforcability
        let cty = Phase::localize_no_subst(self.env, false, None, decl_cty)?;
        // TODO(hrust): coerce_type, user_attributes
        let typed_cv_expr = cv
            .expr
            .as_ref()
            .map(|e| Typing::expr_with_pure_coeffects(self.env, Some(cty.clone()), e))
            .map(|(typed_cv_expr, _expr_ty)| typed_cv_expr);
        assert!(cv.user_attributes.is_empty());
        // TODO(hrust): sound dynamic
        Ok(tast::ClassVar {
            final_: cv.final_,
            xhp_attr: cv.xhp_attr.clone(),
            abstract_: cv.abstract_,
            readonly: cv.readonly,
            visibility: cv.visibility.clone(),
            type_: oxidized::aast::TypeHint(cty, cv.type_.1.clone()),
            id: cv.id.clone(),
            expr: typed_cv_expr,
            user_attributes: vec![],
            doc_comment: cv.doc_comment.clone(),
            is_promoted_variadic: cv.is_promoted_variadic,
            is_static: cv.is_static,
            span: cv.span.clone(),
        })
    }

    fn method_def(
        &mut self,
        _cls: &dyn Class<R>,
        m: &oxidized::aast::Method_<(), ()>,
    ) -> Result<Option<tast::Method_<R>>> {
        let pos = R::Pos::from(&m.name.0);
        let name = Symbol::new(&m.name.1);

        // TODO(hrust): FunUtils.check_params
        // TODO(hrust): tyvars
        self.env.reinitialize_locals();
        // TODO(hrust): callable
        assert!(m.user_attributes.is_empty());
        // TODO(hrust): support_dynamic, coeefects
        // TODO(hrust): is_constructor
        // TODO(hrust): localize_and_add_ast_generic_parameters_and_where_constraints
        // TODO(hrust): get_self_ty
        // TODO(hrust): is_disposable
        self.env.clear_params();
        let (ret_decl_ty, params_decl_ty) = self.hint_fun_header(&m.params, &m.ret);
        // TODO(hrust): set_fn_kind
        let (return_, return_ty) = self.method_return(&pos, name, m, ret_decl_ty)?;
        let param_tys = TypingParam::make_param_local_tys(
            TypingParamFlags {
                dynamic_mode: false,
            },
            self.env,
            params_decl_ty.into_iter(),
        )?;
        // TODO(hrust): memoize, coeefects, can_read_globals
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
        // TODO(hrust): variance, tpenv, disable
        let tb = Typing::fun_(
            self.env,
            Default::default(),
            return_,
            pos.clone(),
            &m.body,
            &m.fun_kind,
        );
        // TODO(hrust): construct return type
        match m.ret.1 {
            Some(_) => {}
            None => {
                assert_ne!(
                    m.name.1,
                    self.ctx.special_names.members.__construct.as_str()
                );
                self.env
                    .add_error(TypingError::primary(Primary::ExpectingReturnTypeHint(pos)))
            }
        };
        // TODO(hrust): tparams
        assert!(m.tparams.is_empty());
        // TODO(hrust): tyvar close and solve
        // TODO(hrust): check_support_dynamic_type
        let _ = typed_params;
        let _ = tb;
        let _ = return_ty;
        let m = oxidized::aast::Method_ {
            span: m.span.clone(),
            annotation: self.env.save(()),
            final_: m.final_,
            abstract_: m.abstract_,
            static_: m.static_,
            readonly_this: m.readonly_this,
            visibility: m.visibility.clone(),
            name: m.name.clone(),
            tparams: vec![],
            where_constraints: m.where_constraints.clone(),
            params: typed_params,
            ctxs: m.ctxs.clone(),
            unsafe_ctxs: m.unsafe_ctxs.clone(),
            body: oxidized::aast::FuncBody { fb_ast: tb },
            fun_kind: m.fun_kind.clone(),
            user_attributes: vec![],
            readonly_ret: m.readonly_ret.clone(),
            ret: oxidized::aast::TypeHint(return_ty, m.ret.1.clone()),
            external: m.external,
            doc_comment: m.doc_comment.clone(),
        };
        Ok(Some(m))
    }

    fn setup_env_for_class_def_check(
        ctx: Rc<TypingCtx<R>>,
        cd: &oxidized::aast::Class_<(), ()>,
    ) -> TEnv<R> {
        // TODO(hrust): pessimize, file_attributes, user_attributes, support_dynamic_type
        TEnv::class_env(ctx, cd)
    }

    fn split_methods<'aast>(
        &self,
        all_methods: impl Iterator<Item = &'aast oxidized::aast::Method_<(), ()>>,
    ) -> (
        Option<&'aast oxidized::aast::Method_<(), ()>>,
        Vec<&'aast oxidized::aast::Method_<(), ()>>,
        Vec<&'aast oxidized::aast::Method_<(), ()>>,
    ) {
        let mut constructor = None;
        let mut static_methods = vec![];
        let mut methods = vec![];
        for m in all_methods {
            if m.name.1 == self.ctx.special_names.members.__construct.as_str() {
                constructor = Some(m);
            } else if m.static_ {
                static_methods.push(m);
            } else {
                methods.push(m);
            }
        }
        (constructor, static_methods, methods)
    }

    fn split_vars<'aast>(
        &self,
        all_vars: impl Iterator<Item = &'aast oxidized::aast::ClassVar<(), ()>>,
    ) -> (
        Vec<&'aast oxidized::aast::ClassVar<(), ()>>,
        Vec<&'aast oxidized::aast::ClassVar<(), ()>>,
    ) {
        let mut static_vars = vec![];
        let mut vars = vec![];
        for v in all_vars {
            if v.is_static {
                static_vars.push(v);
            } else {
                vars.push(v);
            }
        }
        (static_vars, vars)
    }

    fn check_class_members(
        &mut self,
        cd: &oxidized::aast::Class_<(), ()>,
        tc: &dyn Class<R>,
    ) -> Result<(
        Vec<tast::ClassConst<R>>,
        Vec<tast::ClassTypeconstDef<R>>,
        Vec<tast::ClassVar<R>>,
        Vec<tast::ClassVar<R>>,
        Vec<tast::Method_<R>>,
    )> {
        let (static_vars, vars) = self.split_vars(cd.vars.iter());
        let (constructor, static_methods, methods) = self.split_methods(cd.methods.iter());

        // TODO(hrust): is_disposable_class
        let mut typed_methods: Vec<_> = static_methods
            .into_iter()
            .filter_map(|m| self.method_def(tc, m).transpose())
            .collect::<Result<_>>()?;
        for m in methods {
            if let Some(tm) = self.method_def(tc, m)? {
                typed_methods.push(tm);
            }
        }

        let typed_vars: Vec<_> = vars
            .into_iter()
            .map(|v| self.class_var_def(true, tc, v))
            .collect::<Result<_>>()?;
        let typed_static_vars: Vec<_> = static_vars
            .into_iter()
            .map(|v| self.class_var_def(false, tc, v))
            .collect::<Result<_>>()?;

        assert!(constructor.is_none());
        assert!(cd.typeconsts.is_empty());
        assert!(cd.consts.is_empty());
        Ok((vec![], vec![], typed_vars, typed_static_vars, typed_methods))
    }

    fn class_def_impl(
        &mut self,
        cd: &oxidized::aast::Class_<(), ()>,
        tc: &dyn Class<R>,
    ) -> Result<tast::Class_<R>> {
        // TODO(hrust): class_wellformedness_checks
        // TODO(hrust): class_hierarchy_checks
        assert!(cd.user_attributes.is_empty());
        assert!(cd.file_attributes.is_empty());

        let (typed_consts, typed_typeconsts, typed_vars, mut typed_static_vars, typed_methods) =
            self.check_class_members(cd, tc)?;
        typed_static_vars.extend(typed_vars);

        // TODO(hrust): class_type_params
        let tparams = cd
            .tparams
            .iter()
            .map(|tp| Typing::type_param(self.env, tp))
            .collect();

        Ok(oxidized::aast::Class_ {
            span: cd.span.clone(),
            annotation: self.env.save(()),
            mode: cd.mode.clone(),
            final_: cd.final_.clone(),
            is_xhp: cd.is_xhp,
            has_xhp_keyword: cd.has_xhp_keyword,
            kind: cd.kind.clone(),
            name: cd.name.clone(),
            tparams,
            extends: cd.extends.clone(),
            uses: cd.uses.clone(),
            // use_as_alias and insteadof_alias are PHP features not supported
            // in Hack but are required since we have runtime support for it
            use_as_alias: vec![],
            insteadof_alias: vec![],
            xhp_attr_uses: cd.xhp_attr_uses.clone(),
            xhp_category: cd.xhp_category.clone(),
            reqs: cd.reqs.clone(),
            implements: cd.implements.clone(),
            where_constraints: cd.where_constraints.clone(),
            consts: typed_consts,
            typeconsts: typed_typeconsts,
            vars: typed_static_vars,
            methods: typed_methods,
            attributes: vec![],
            xhp_children: cd.xhp_children.clone(),
            xhp_attrs: vec![],
            namespace: cd.namespace.clone(),
            user_attributes: vec![],
            file_attributes: vec![],
            enum_: cd.enum_.clone(),
            doc_comment: cd.doc_comment.clone(),
            emit_id: cd.emit_id.clone(),
        })
    }

    pub fn class_def(
        ctx: Rc<TypingCtx<R>>,
        cd: &oxidized::aast::Class_<(), ()>,
    ) -> Result<(tast::Class_<R>, Vec<TypingError<R>>)> {
        // TODO(hrust): add_decl_errors
        // TODO(hrust): duplicate check
        let env = Self::setup_env_for_class_def_check(Rc::clone(&ctx), cd);
        let tc = env.decls().get_class_or_error(TypeName::new(&cd.name.1))?;
        let def = TypingToplevel { ctx, env: &env }.class_def_impl(cd, &*tc)?;

        Ok((def, env.destruct()))
    }
}

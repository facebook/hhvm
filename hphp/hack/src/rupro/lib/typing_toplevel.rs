// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(dead_code)]
use std::rc::Rc;

use crate::reason::Reason;
use crate::special_names;
use crate::tast;
use crate::typing::ast::TC;
use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Result;
use crate::typing_ctx::TypingCtx;
use crate::typing_error::TypingError;

pub struct TypingToplevel<'a, R: Reason> {
    ctx: Rc<TypingCtx<R>>,
    env: &'a TEnv<R>,
}

impl<'a, R: Reason> TypingToplevel<'a, R> {
    pub fn fun_def(
        ctx: Rc<TypingCtx<R>>,
        fd: &oxidized::aast::FunDef<(), ()>,
    ) -> Result<(tast::FunDef<R>, Vec<TypingError<R>>)> {
        let env = TEnv::fun_env(Rc::clone(&ctx), fd);
        let def = fd.infer(&env, ())?;
        Ok((def, env.destruct()))
    }

    fn setup_env_for_class_def_check(
        ctx: Rc<TypingCtx<R>>,
        cd: &oxidized::aast::Class_<(), ()>,
    ) -> TEnv<R> {
        // TODO(hrust): file_attributes, user_attributes, support_dynamic_type
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
            if m.name.1 == special_names::members::__construct.as_str() {
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
        let typed_methods: Vec<_> = static_methods
            .iter()
            .chain(methods.iter())
            .map(|m| m.infer(self.env, ()))
            .collect::<Result<_>>()?;

        let typed_vars: Vec<_> = vars.infer(self.env, ())?;
        let typed_static_vars: Vec<_> = static_vars.infer(self.env, ())?;

        assert!(constructor.is_none());
        assert!(cd.typeconsts.is_empty());
        assert!(cd.consts.is_empty());
        Ok((vec![], vec![], typed_vars, typed_static_vars, typed_methods))
    }

    fn class_def_impl(&mut self, cd: &oxidized::aast::Class_<(), ()>) -> Result<tast::Class_<R>> {
        // TODO(hrust): class_wellformedness_checks
        // TODO(hrust): class_hierarchy_checks
        assert!(cd.user_attributes.is_empty());
        assert!(cd.file_attributes.is_empty());

        let (typed_consts, typed_typeconsts, typed_vars, mut typed_static_vars, typed_methods) =
            self.check_class_members(cd)?;
        typed_static_vars.extend(typed_vars);

        // TODO(hrust): class_type_params
        let tparams = cd.tparams.infer(self.env, ())?;

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
        let def = TypingToplevel { ctx, env: &env }.class_def_impl(cd)?;

        Ok((def, env.destruct()))
    }
}

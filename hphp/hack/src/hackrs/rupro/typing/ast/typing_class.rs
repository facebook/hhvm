// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ty::reason::Reason;

use crate::tast;
use crate::typing::ast::typing_trait::Infer;
use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Error;
use crate::typing::typing_error::Result;

impl<R: Reason> Infer<R> for oxidized::aast::Class_<(), ()> {
    type Params = ();
    type Typed = tast::Class_<R>;

    fn infer(&self, env: &mut TEnv<R>, _params: ()) -> Result<Self::Typed> {
        rupro_todo_assert!(self.user_attributes.is_empty(), AST);
        rupro_todo_assert!(self.file_attributes.is_empty(), AST);
        rupro_todo_mark!(Wellformedness);
        rupro_todo_mark!(ClassHierarchyChecks);

        let class_name = pos::TypeName::from(&self.name.1);
        let tc = env
            .decls()
            .get_class(class_name)?
            .ok_or_else(|| Error::DeclNotFound(class_name.into()))?;

        for err in tc.decl_errors() {
            env.add_error(err.into());
        }

        let (typed_consts, typed_typeconsts, typed_vars, mut typed_static_vars, typed_methods) =
            check_class_members(env, self)?;
        typed_static_vars.extend(typed_vars);

        let tparams = self.tparams.infer(env, ())?;
        rupro_todo_assert!(env.solve_all_unsolved_tyvars()?.is_none(), MissingError);

        let res = oxidized::aast::Class_ {
            span: self.span.clone(),
            annotation: env.save(()),
            mode: self.mode.clone(),
            final_: self.final_.clone(),
            is_xhp: self.is_xhp,
            has_xhp_keyword: self.has_xhp_keyword,
            kind: self.kind.clone(),
            name: self.name.clone(),
            tparams,
            extends: self.extends.clone(),
            uses: self.uses.clone(),
            xhp_attr_uses: self.xhp_attr_uses.clone(),
            xhp_category: self.xhp_category.clone(),
            reqs: self.reqs.clone(),
            implements: self.implements.clone(),
            where_constraints: self.where_constraints.clone(),
            consts: typed_consts,
            typeconsts: typed_typeconsts,
            vars: typed_static_vars,
            methods: typed_methods,
            xhp_children: self.xhp_children.clone(),
            xhp_attrs: vec![],
            namespace: self.namespace.clone(),
            user_attributes: vec![],
            file_attributes: vec![],
            enum_: self.enum_.clone(),
            doc_comment: self.doc_comment.clone(),
            emit_id: self.emit_id.clone(),
            // TODO(T116039119): Populate value with presence of internal attribute
            internal: false,
            module: None,
            docs_url: None,
        };
        Ok(res)
    }
}

fn check_class_members<R: Reason>(
    env: &mut TEnv<R>,
    cd: &oxidized::aast::Class_<(), ()>,
) -> Result<(
    Vec<tast::ClassConst<R>>,
    Vec<tast::ClassTypeconstDef<R>>,
    Vec<tast::ClassVar<R>>,
    Vec<tast::ClassVar<R>>,
    Vec<tast::Method_<R>>,
)> {
    rupro_todo_mark!(Disposable);
    let (static_vars, vars) = split_vars(cd.vars.iter());
    let (constructor, static_methods, methods) = split_methods(cd.methods.iter());

    let typed_methods: Vec<_> = static_methods
        .iter()
        .chain(methods.iter())
        .map(|m| m.infer(env, ()))
        .collect::<Result<_>>()?;

    let typed_vars: Vec<_> = vars.infer(env, ())?;
    let typed_static_vars: Vec<_> = static_vars.infer(env, ())?;

    rupro_todo_assert!(constructor.is_none(), AST);
    rupro_todo_assert!(cd.typeconsts.is_empty(), AST);
    rupro_todo_assert!(cd.consts.is_empty(), AST);
    Ok((vec![], vec![], typed_vars, typed_static_vars, typed_methods))
}

fn split_methods<'aast>(
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

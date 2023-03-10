// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::HashSet;
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::aast::Class_;
use oxidized::aast::Method_;
use oxidized::ast::Id;
use oxidized::naming_error::NamingError;
use oxidized::nast_check_error::NastCheckError;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Default)]
pub struct ValidateClassMethodsPass;

impl Pass for ValidateClassMethodsPass {
    fn on_ty_class__bottom_up<Ex: Default, En>(
        &mut self,
        class: &mut Class_<Ex, En>,
        env: &Env,
    ) -> ControlFlow<(), ()> {
        let mut seen = HashSet::<&str>::new();
        for method in class.methods.iter() {
            let Id(pos, name) = &method.name;
            if seen.contains(name as &str) {
                env.emit_error(NamingError::AlreadyBound {
                    pos: pos.clone(),
                    name: name.clone(),
                });
            }
            seen.insert(name);
        }
        ControlFlow::Continue(())
    }

    fn on_ty_method__bottom_up<Ex: Default, En>(
        &mut self,
        method: &mut Method_<Ex, En>,
        env: &Env,
    ) -> ControlFlow<(), ()> {
        if method.abstract_
            && method.user_attributes.iter().any(|attr| {
                let Id(_, ua) = &attr.name;
                ua == sn::user_attributes::MEMOIZE || ua == sn::user_attributes::MEMOIZE_LSB
            })
        {
            env.emit_error(NastCheckError::AbstractMethodMemoize(method.span.clone()))
        }
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {

    use ocamlrep::rc::RcOc;
    use oxidized::aast::Block;
    use oxidized::aast::FuncBody;
    use oxidized::aast::Pos;
    use oxidized::aast::TypeHint;
    use oxidized::aast::UserAttribute;
    use oxidized::aast::UserAttributes;
    use oxidized::aast::Visibility;
    use oxidized::ast::FunKind;
    use oxidized::ast::Id;
    use oxidized::ast_defs::Abstraction;
    use oxidized::ast_defs::ClassishKind;
    use oxidized::namespace_env;
    use oxidized::naming_phase_error::NamingPhaseError;
    use oxidized::typechecker_options::TypecheckerOptions;

    use super::*;
    use crate::env::ProgramSpecificOptions;
    use crate::Transform;

    fn mk_method(
        name: String,
        r#abstract: bool,
        attrs: Vec<UserAttribute<(), ()>>,
    ) -> Method_<(), ()> {
        Method_ {
            span: Pos::NONE,
            annotation: (),
            final_: false,
            abstract_: r#abstract,
            static_: true,
            readonly_this: false,
            visibility: Visibility::Public,
            name: Id(Pos::NONE, name),
            tparams: vec![],
            where_constraints: vec![],
            params: vec![],
            ctxs: None,
            unsafe_ctxs: None,
            body: FuncBody {
                fb_ast: Block(vec![]),
            },
            fun_kind: FunKind::FSync,
            user_attributes: UserAttributes(attrs),
            readonly_ret: None,
            ret: TypeHint((), None),
            external: false,
            doc_comment: None,
        }
    }

    fn mk_class(name: String, methods: Vec<Method_<(), ()>>) -> Class_<(), ()> {
        Class_ {
            span: Pos::NONE,
            annotation: (),
            mode: file_info::Mode::Mstrict,
            final_: false,
            is_xhp: false,
            has_xhp_keyword: false,
            kind: ClassishKind::Cclass(Abstraction::Concrete),
            name: Id(Pos::NONE, name),
            tparams: vec![],
            extends: vec![],
            uses: vec![],
            xhp_attr_uses: vec![],
            xhp_category: None,
            reqs: vec![],
            implements: vec![],
            where_constraints: vec![],
            consts: vec![],
            typeconsts: vec![],
            vars: vec![],
            methods,
            xhp_children: vec![],
            xhp_attrs: vec![],
            namespace: RcOc::new(namespace_env::Env::empty(vec![], false, false)),
            user_attributes: Default::default(),
            file_attributes: vec![],
            docs_url: None,
            enum_: None,
            doc_comment: None,
            emit_id: None,
            internal: false,
            module: None,
        }
    }

    #[test]
    fn test_multiply_bound_method_name() {
        let env = Env::new(
            &TypecheckerOptions::default(),
            &ProgramSpecificOptions::default(),
        );

        let m = mk_method("foo".to_string(), false, vec![]);
        let n = mk_method("foo".to_string(), false, vec![]);
        let mut class = mk_class("Foo".to_string(), vec![m, n]);

        class.transform(&env, &mut ValidateClassMethodsPass);
        assert!(matches!(
            env.into_errors().as_slice(),
            [NamingPhaseError::Naming(NamingError::AlreadyBound { .. })]
        ));
    }

    #[test]
    fn test_abstract_memoized_method() {
        let env = Env::new(
            &TypecheckerOptions::default(),
            &ProgramSpecificOptions::default(),
        );

        let memoized_attr = UserAttribute::<(), ()> {
            name: Id(Pos::NONE, sn::user_attributes::MEMOIZE.to_string()),
            params: vec![],
        };
        let mut method = mk_method("foo".to_string(), true, vec![memoized_attr]);
        method.transform(&env, &mut ValidateClassMethodsPass);
        assert!(matches!(
            env.into_errors().as_slice(),
            [NamingPhaseError::NastCheck(
                NastCheckError::AbstractMethodMemoize(_)
            )]
        ));
    }
}

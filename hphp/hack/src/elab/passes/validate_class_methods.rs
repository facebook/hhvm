// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hash::HashSet;
use nast::Class_;
use nast::Id;
use nast::Method_;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ValidateClassMethodsPass;

impl Pass for ValidateClassMethodsPass {
    fn on_ty_class__bottom_up(&mut self, env: &Env, class: &mut Class_) -> ControlFlow<()> {
        let mut seen = HashSet::<&str>::default();
        for method in class.methods.iter() {
            let Id(pos, name) = &method.name;
            if seen.contains(name as &str) {
                env.emit_error(NamingError::MethodNameAlreadyBound {
                    pos: pos.clone(),
                    meth_name: name.clone(),
                });
            }
            seen.insert(name);
        }
        Continue(())
    }

    fn on_ty_method__bottom_up(&mut self, env: &Env, method: &mut Method_) -> ControlFlow<()> {
        if method.abstract_
            && method.user_attributes.iter().any(|attr| {
                let Id(_, ua) = &attr.name;
                ua == sn::user_attributes::MEMOIZE || ua == sn::user_attributes::MEMOIZE_LSB
            })
        {
            env.emit_error(NastCheckError::AbstractMethodMemoize(
                method.name.pos().clone(),
            ))
        }
        Continue(())
    }
}

#[cfg(test)]
mod tests {

    use nast::Abstraction;
    use nast::Block;
    use nast::ClassishKind;
    use nast::FunKind;
    use nast::FuncBody;
    use nast::Id;
    use nast::Pos;
    use nast::TypeHint;
    use nast::UserAttribute;
    use nast::UserAttributes;
    use nast::Visibility;
    use ocamlrep::rc::RcOc;
    use oxidized::namespace_env;
    use oxidized::typechecker_options::TypecheckerOptions;

    use super::*;
    use crate::env::ProgramSpecificOptions;

    fn mk_method(name: String, r#abstract: bool, attrs: Vec<UserAttribute>) -> Method_ {
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

    fn mk_class(name: String, methods: Vec<Method_>) -> Class_ {
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
            [NamingPhaseError::Naming(
                NamingError::MethodNameAlreadyBound { .. }
            )]
        ));
    }

    #[test]
    fn test_abstract_memoized_method() {
        let env = Env::new(
            &TypecheckerOptions::default(),
            &ProgramSpecificOptions::default(),
        );

        let memoized_attr = UserAttribute {
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

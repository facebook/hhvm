// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use oxidized::naming_error::NamingError;
use oxidized::nast::Class_;
use oxidized::nast::Id;
use oxidized::nast::Sid;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Default)]
pub struct ValidateClassMemberPass;

impl Pass for ValidateClassMemberPass {
    fn on_ty_class__bottom_up(&mut self, env: &Env, class: &mut Class_) -> ControlFlow<()> {
        let typeconst_names = class.typeconsts.iter().map(|tc| &tc.name);
        let const_names = class.consts.iter().map(|c| &c.id);
        error_if_repeated_name(env, typeconst_names.chain(const_names));
        ControlFlow::Continue(())
    }
}

// We use the same namespace as constants within the class so we cannot have
// a const and type const with the same name.
fn error_if_repeated_name<'a>(env: &Env, names: impl Iterator<Item = &'a Sid>) {
    let mut seen = hash::HashSet::<&str>::default();
    for Id(pos, name) in names {
        if seen.contains(name as &str) {
            env.emit_error(NamingError::AlreadyBound {
                pos: pos.clone(),
                name: name.clone(),
            });
        }
        seen.insert(name);
    }
}

#[cfg(test)]
mod tests {
    use ocamlrep::rc::RcOc;
    use oxidized::namespace_env;
    use oxidized::naming_phase_error::NamingPhaseError;
    use oxidized::nast::Abstraction;
    use oxidized::nast::ClassConcreteTypeconst;
    use oxidized::nast::ClassConst;
    use oxidized::nast::ClassConstKind;
    use oxidized::nast::ClassTypeconst;
    use oxidized::nast::ClassTypeconstDef;
    use oxidized::nast::ClassishKind;
    use oxidized::nast::Id;
    use oxidized::nast::Pos;
    use oxidized::nast::UserAttributes;
    use oxidized::typechecker_options::TypecheckerOptions;

    use super::*;
    use crate::elab_utils;
    use crate::env::Env;
    use crate::env::ProgramSpecificOptions;
    use crate::Transform;

    fn mk_class_const(name: String) -> ClassConst {
        ClassConst {
            user_attributes: UserAttributes::default(),
            type_: None,
            id: Id(Pos::NONE, name),
            kind: ClassConstKind::CCConcrete(elab_utils::expr::null()),
            span: Pos::NONE,
            doc_comment: None,
        }
    }

    fn mk_class_typeconst(name: String) -> ClassTypeconstDef {
        ClassTypeconstDef {
            user_attributes: UserAttributes::default(),
            name: Id(Pos::NONE, name),
            kind: ClassTypeconst::TCConcrete(ClassConcreteTypeconst {
                c_tc_type: elab_utils::hint::null(),
            }),
            span: Pos::NONE,
            doc_comment: None,
            is_ctx: false,
        }
    }

    fn mk_class(
        name: String,
        consts: Vec<ClassConst>,
        typeconsts: Vec<ClassTypeconstDef>,
    ) -> Class_ {
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
            consts,
            typeconsts,
            vars: vec![],
            methods: vec![],
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
    fn test_class_constant_names_clash() {
        let env = Env::new(
            &TypecheckerOptions::default(),
            &ProgramSpecificOptions::default(),
        );
        let mut class = mk_class(
            "Foo".to_string(),
            vec![mk_class_const("FOO".to_string())],
            vec![mk_class_typeconst("FOO".to_string())],
        );
        class.transform(&env, &mut ValidateClassMemberPass);
        assert!(matches!(
            env.into_errors().as_slice(),
            [NamingPhaseError::Naming(NamingError::AlreadyBound { .. })]
        ));
    }
}

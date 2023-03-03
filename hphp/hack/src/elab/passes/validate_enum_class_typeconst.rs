// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use oxidized::aast_defs::Class_;
use oxidized::ast::ClassishKind;
use oxidized::naming_error::NamingError;
use oxidized::naming_phase_error::NamingPhaseError;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ValidateEnumClassTypeconstPass;

impl Pass for ValidateEnumClassTypeconstPass {
    fn on_ty_class__top_down<Ex: Default, En>(
        &mut self,
        class: &mut Class_<Ex, En>,
        config: &Config,
        errs: &mut Vec<NamingPhaseError>,
    ) -> ControlFlow<(), ()> {
        if !config.allow_type_constant_in_enum_class()
            && !class.typeconsts.is_empty()
            && matches!(class.kind, ClassishKind::CenumClass(_))
        {
            errs.push(NamingPhaseError::Naming(
                NamingError::TypeConstantInEnumClassOutsideAllowedLocations(class.span.clone()),
            ));
        }
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {
    use ocamlrep::rc::RcOc;
    use oxidized::aast::ClassConcreteTypeconst;
    use oxidized::aast::ClassTypeconst;
    use oxidized::aast::ClassTypeconstDef;
    use oxidized::aast::Hint;
    use oxidized::aast::Hint_;
    use oxidized::aast::Pos;
    use oxidized::aast::UserAttributes;
    use oxidized::ast::Abstraction;
    use oxidized::ast::Id;
    use oxidized::namespace_env;
    use oxidized::typechecker_options::TypecheckerOptions;

    use super::*;
    use crate::config::ProgramSpecificOptions;
    use crate::Transform;

    fn mk_class_typeconst_def(name: &str) -> ClassTypeconstDef<(), ()> {
        ClassTypeconstDef {
            user_attributes: UserAttributes::default(),
            name: Id(Pos::NONE, name.to_string()),
            kind: ClassTypeconst::TCConcrete(ClassConcreteTypeconst {
                c_tc_type: Hint(Pos::NONE, Box::new(Hint_::Hnothing)),
            }),
            span: Pos::NONE,
            doc_comment: None,
            is_ctx: false,
        }
    }

    fn mk_enum_class(
        class_name: &str,
        typeconsts: Vec<ClassTypeconstDef<(), ()>>,
    ) -> Class_<(), ()> {
        Class_ {
            span: Pos::NONE,
            annotation: (),
            mode: file_info::Mode::Mstrict,
            final_: false,
            is_xhp: false,
            has_xhp_keyword: false,
            kind: ClassishKind::CenumClass(Abstraction::Concrete),
            name: Id(Pos::NONE, class_name.to_string()),
            tparams: vec![],
            extends: vec![],
            uses: vec![],
            xhp_attr_uses: vec![],
            xhp_category: None,
            reqs: vec![],
            implements: vec![],
            where_constraints: vec![],
            consts: vec![],
            typeconsts,
            vars: vec![],
            methods: vec![],
            xhp_children: vec![],
            xhp_attrs: vec![],
            namespace: RcOc::new(namespace_env::Env::empty(vec![], false, false)),
            user_attributes: UserAttributes::<(), ()>::default(),
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
    fn test_type_constant_allowed() {
        let mut errs = Vec::default();
        let config = Config::new(
            &TypecheckerOptions::default(),
            &ProgramSpecificOptions {
                allow_type_constant_in_enum_class: true,
                ..Default::default()
            },
        );
        let mut class = mk_enum_class("Foo", vec![mk_class_typeconst_def("foo")]);
        class.transform(&config, &mut errs, &mut ValidateEnumClassTypeconstPass);
        assert!(errs.is_empty());
    }

    #[test]
    fn test_type_constant_not_allowed() {
        let mut errs = Vec::default();
        let config = Config::new(
            &TypecheckerOptions::default(),
            &ProgramSpecificOptions {
                allow_type_constant_in_enum_class: false,
                ..Default::default()
            },
        );
        let mut class = mk_enum_class("Foo", vec![mk_class_typeconst_def("foo")]);
        class.transform(&config, &mut errs, &mut ValidateEnumClassTypeconstPass);
        assert!(matches!(
            &errs[..],
            [NamingPhaseError::Naming(
                NamingError::TypeConstantInEnumClassOutsideAllowedLocations(_)
            )]
        ));
    }
}

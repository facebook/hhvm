// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::aast_defs::Class_;
use oxidized::aast_defs::Hint;
use oxidized::aast_defs::Hint_;
use oxidized::ast_defs::Abstraction;
use oxidized::ast_defs::ClassishKind;
use oxidized::ast_defs::Id;
use oxidized::naming_error::NamingError;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ElabEnumClassPass;

impl Pass for ElabEnumClassPass {
    fn on_ty_class__top_down<Ex: Default, En>(
        &mut self,
        elem: &mut Class_<Ex, En>,
        _env: &Env,
    ) -> ControlFlow<(), ()> {
        if let Some(enum_) = &elem.enum_ {
            let Id(pos, _) = &elem.name;
            let enum_hint = Hint(
                pos.clone(),
                Box::new(Hint_::Happly(elem.name.clone(), vec![])),
            );
            let (cls_name, bounds) = match elem.kind {
                ClassishKind::CenumClass(Abstraction::Concrete) => (
                    sn::classes::HH_BUILTIN_ENUM_CLASS,
                    vec![Hint(
                        pos.clone(),
                        Box::new(Hint_::Happly(
                            Id(pos.clone(), sn::classes::MEMBER_OF.to_string()),
                            vec![enum_hint, enum_.base.clone()],
                        )),
                    )],
                ),
                ClassishKind::CenumClass(Abstraction::Abstract) => {
                    (sn::classes::HH_BUILTIN_ABSTRACT_ENUM_CLASS, vec![])
                }
                _ => (sn::classes::HH_BUILTIN_ENUM, vec![enum_hint]),
            };
            let extend_hint = Hint(
                pos.clone(),
                Box::new(Hint_::Happly(Id(pos.clone(), cls_name.to_string()), bounds)),
            );
            elem.extends.push(extend_hint)
        }
        ControlFlow::Continue(())
    }

    fn on_ty_hint__top_down(
        &mut self,
        elem: &mut oxidized::tast::Hint_,
        env: &Env,
    ) -> ControlFlow<(), ()> {
        if !(env.is_hhi() || env.is_systemlib()) {
            match elem {
                Hint_::Happly(Id(pos, ty_name), _)
                    if ty_name == sn::classes::HH_BUILTIN_ENUM
                        || ty_name == sn::classes::HH_BUILTIN_ENUM_CLASS
                        || ty_name == sn::classes::HH_BUILTIN_ABSTRACT_ENUM_CLASS =>
                {
                    env.emit_error(NamingError::UsingInternalClass {
                        pos: pos.clone(),
                        class_name: ty_name.clone(),
                    })
                }
                _ => (),
            }
        }
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {

    use ocamlrep::rc::RcOc;
    use oxidized::aast_defs::Enum_;
    use oxidized::aast_defs::UserAttributes;
    use oxidized::namespace_env;
    use oxidized::s_map::SMap;
    use oxidized::tast::Pos;

    use super::*;
    use crate::Transform;

    fn make_enum_class_(kind: ClassishKind, enum_: Enum_) -> Class_<(), ()> {
        Class_ {
            span: Pos::NONE,
            annotation: (),
            mode: file_info::Mode::Mstrict,
            final_: false,
            is_xhp: false,
            has_xhp_keyword: false,
            kind,
            name: Id(Pos::NONE, "Classy".to_string()),
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
            methods: vec![],
            xhp_children: vec![],
            xhp_attrs: vec![],
            namespace: RcOc::new(namespace_env::Env {
                ns_uses: SMap::default(),
                class_uses: SMap::default(),
                fun_uses: SMap::default(),
                const_uses: SMap::default(),
                name: None,
                auto_ns_map: vec![],
                is_codegen: false,
                disable_xhp_element_mangling: false,
            }),
            user_attributes: UserAttributes::default(),
            file_attributes: vec![],
            docs_url: None,
            enum_: Some(enum_),
            doc_comment: None,
            emit_id: None,
            internal: false,
            module: None,
        }
    }

    #[test]
    fn test_enum_class_concrete() {
        let env = Env::default();

        let mut pass = ElabEnumClassPass;

        let mut elem: Class_<(), ()> = make_enum_class_(
            ClassishKind::CenumClass(Abstraction::Concrete),
            Enum_ {
                base: Hint(Pos::NONE, Box::new(Hint_::Herr)),
                constraint: None,
                includes: vec![],
            },
        );

        elem.transform(&env, &mut pass);
        let Hint(_, hint_) = &mut elem.extends.pop().unwrap();

        assert!(match &mut **hint_ {
            Hint_::Happly(Id(_, cname), bounds) => {
                cname == sn::classes::HH_BUILTIN_ENUM_CLASS
                    && bounds.pop().map_or(false, |Hint(_, hint_)| match *hint_ {
                        Hint_::Happly(Id(_, member_of), mut tparams) => {
                            (member_of == sn::classes::MEMBER_OF)
                                && (tparams
                                    .pop()
                                    .map_or(false, |Hint(_, hint_)| matches!(*hint_, Hint_::Herr)))
                                && (tparams.pop().map_or(false, |Hint(_, hint_)| {
                                    matches!(*hint_, Hint_::Happly(..))
                                }))
                        }
                        _ => false,
                    })
            }
            _ => false,
        })
    }

    #[test]
    fn test_enum_class_abstract() {
        let env = Env::default();

        let mut pass = ElabEnumClassPass;

        let mut elem: Class_<(), ()> = make_enum_class_(
            ClassishKind::CenumClass(Abstraction::Abstract),
            Enum_ {
                base: Hint(Pos::NONE, Box::new(Hint_::Herr)),
                constraint: None,
                includes: vec![],
            },
        );

        elem.transform(&env, &mut pass);
        let Hint(_, hint_) = &mut elem.extends.pop().unwrap();

        assert!(match &mut **hint_ {
            Hint_::Happly(Id(_, cname), bounds) => {
                cname == sn::classes::HH_BUILTIN_ABSTRACT_ENUM_CLASS && bounds.is_empty()
            }
            _ => false,
        })
    }

    #[test]
    fn test_enum() {
        let env = Env::default();

        let mut pass = ElabEnumClassPass;

        let mut elem: Class_<(), ()> = make_enum_class_(
            ClassishKind::Cenum,
            Enum_ {
                base: Hint(Pos::NONE, Box::new(Hint_::Herr)),
                constraint: None,
                includes: vec![],
            },
        );

        elem.transform(&env, &mut pass);
        let Hint(_, hint_) = &mut elem.extends.pop().unwrap();

        assert!(match &mut **hint_ {
            Hint_::Happly(Id(_, cname), bounds) => {
                cname == sn::classes::HH_BUILTIN_ENUM
                    && bounds
                        .pop()
                        .map_or(false, |Hint(_, hint_)| matches!(*hint_, Hint_::Happly(..)))
            }
            _ => false,
        })
    }
}

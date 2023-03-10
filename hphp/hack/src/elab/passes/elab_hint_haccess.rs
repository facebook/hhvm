// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use bitflags::bitflags;
use naming_special_names_rust as sn;
use oxidized::naming_error::NamingError;
use oxidized::nast::Class_;
use oxidized::nast::Contexts;
use oxidized::nast::Hint;
use oxidized::nast::Hint_;
use oxidized::nast::WhereConstraintHint;

use crate::env::Env;
use crate::Pass;

#[derive(Clone, Default)]
pub struct ElabHintHaccessPass {
    current_class: Option<String>,
    flags: Flags,
}

bitflags! {
    #[derive(Default)]
    struct Flags: u8 {
        const IN_CONTEXT = 1 << 0;
        const IN_HACCESS = 1 << 1;
        const IN_WHERE_CLAUSE = 1 << 2;
    }
}

impl ElabHintHaccessPass {
    fn set_in_class(&mut self, cls: &Class_) {
        self.current_class = Some(cls.name.name().to_string())
    }

    fn in_context(&self) -> bool {
        self.flags.contains(Flags::IN_CONTEXT)
    }

    fn set_in_context(&mut self, value: bool) {
        self.flags.set(Flags::IN_CONTEXT, value)
    }

    fn in_haccess(&self) -> bool {
        self.flags.contains(Flags::IN_HACCESS)
    }

    fn set_in_haccess(&mut self, value: bool) {
        self.flags.set(Flags::IN_HACCESS, value)
    }

    fn in_where_clause(&self) -> bool {
        self.flags.contains(Flags::IN_WHERE_CLAUSE)
    }

    fn set_in_where_clause(&mut self, value: bool) {
        self.flags.set(Flags::IN_WHERE_CLAUSE, value)
    }
}

impl Pass for ElabHintHaccessPass {
    fn on_ty_hint_top_down(&mut self, elem: &mut Hint, env: &Env) -> ControlFlow<()> {
        if !self.in_haccess() {
            return ControlFlow::Continue(());
        }
        match &mut *elem.1 {
            Hint_::Happly(id, hints) if id.name() == sn::classes::SELF => {
                if let Some(name) = &self.current_class {
                    id.1 = name.to_string();
                    // TODO[mjt] we appear to be discarding type arguments on `Happly` here?
                    hints.clear();
                    ControlFlow::Continue(())
                } else {
                    env.emit_error(NamingError::SelfOutsideClass(id.0.clone()));
                    *elem.1 = Hint_::Herr;
                    ControlFlow::Break(())
                }
            }

            Hint_::Happly(id, _)
                if id.name() == sn::classes::STATIC || id.name() == sn::classes::PARENT =>
            {
                env.emit_error(NamingError::InvalidTypeAccessRoot {
                    pos: id.pos().clone(),
                    id: Some(id.name().to_string()),
                });
                *elem.1 = Hint_::Herr;
                ControlFlow::Break(())
            }

            Hint_::Hthis | Hint_::Happly(..) => ControlFlow::Continue(()),

            Hint_::Habstr(..) if self.in_where_clause() || self.in_context() => {
                ControlFlow::Continue(())
            }

            // TODO[mjt] I don't understand what this case corresponds to
            Hint_::Hvar(..) => ControlFlow::Continue(()),

            _ => {
                env.emit_error(NamingError::InvalidTypeAccessRoot {
                    pos: elem.0.clone(),
                    id: None,
                });
                *elem.1 = Hint_::Herr;
                ControlFlow::Break(())
            }
        }
    }

    fn on_ty_hint__top_down(&mut self, elem: &mut Hint_, _: &Env) -> ControlFlow<()> {
        self.set_in_haccess(matches!(elem, Hint_::Haccess(..)));
        ControlFlow::Continue(())
    }

    fn on_ty_class__top_down(&mut self, elem: &mut Class_, _: &Env) -> ControlFlow<()> {
        self.set_in_class(elem);
        ControlFlow::Continue(())
    }

    fn on_ty_where_constraint_hint_top_down(
        &mut self,
        _: &mut WhereConstraintHint,
        _: &Env,
    ) -> ControlFlow<()> {
        self.set_in_where_clause(true);
        ControlFlow::Continue(())
    }

    fn on_ty_contexts_top_down(&mut self, _: &mut Contexts, _: &Env) -> ControlFlow<()> {
        self.set_in_context(true);
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {

    use oxidized::naming_phase_error::NamingPhaseError;
    use oxidized::nast::ConstraintKind;
    use oxidized::nast::Id;
    use oxidized::nast::Pos;

    use super::*;
    use crate::Transform;

    // -- type access through `self` -------------------------------------------

    #[test]
    // type access through `self` is valid inside a class
    fn test_haccess_self_in_class_valid() {
        let env = Env::default();

        let class_name = "Classy";
        let mut pass = ElabHintHaccessPass {
            current_class: Some(class_name.to_string()),
            ..Default::default()
        };
        let mut elem = Hint(
            Pos::default(),
            Box::new(Hint_::Haccess(
                Hint(
                    Pos::default(),
                    Box::new(Hint_::Happly(
                        Id(Pos::default(), sn::classes::SELF.to_string()),
                        vec![],
                    )),
                ),
                vec![Id(Pos::default(), "T".to_string())],
            )),
        );
        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());
        assert!(match elem {
            Hint(_, box Hint_::Haccess(Hint(_, box Hint_::Happly(id, hints)), _)) =>
                id.name() == class_name && hints.is_empty(),
            _ => false,
        })
    }

    #[test]
    // currently we erase any type params to which `self` is applied; adding
    // test for tracking
    fn test_haccess_self_in_class_erased_tparams_valid() {
        let env = Env::default();

        let class_name = "Classy";
        let mut pass = ElabHintHaccessPass {
            current_class: Some(class_name.to_string()),
            ..Default::default()
        };
        let mut elem = Hint(
            Pos::default(),
            Box::new(Hint_::Haccess(
                Hint(
                    Pos::default(),
                    Box::new(Hint_::Happly(
                        Id(Pos::default(), sn::classes::SELF.to_string()),
                        vec![Hint(Pos::default(), Box::new(Hint_::Herr))],
                    )),
                ),
                vec![Id(Pos::default(), "T".to_string())],
            )),
        );
        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());
        assert!(match elem {
            Hint(_, box Hint_::Haccess(Hint(_, box Hint_::Happly(id, hints)), _)) =>
                id.name() == class_name && hints.is_empty(),
            _ => false,
        })
    }

    #[test]
    // type access through `self` is invalid outside a class
    fn test_haccess_self_outside_class_invalid() {
        let env = Env::default();

        let mut pass = ElabHintHaccessPass::default();
        let mut elem = Hint(
            Pos::default(),
            Box::new(Hint_::Haccess(
                Hint(
                    Pos::default(),
                    Box::new(Hint_::Happly(
                        Id(Pos::default(), sn::classes::SELF.to_string()),
                        vec![],
                    )),
                ),
                vec![Id(Pos::default(), "T".to_string())],
            )),
        );
        elem.transform(&env, &mut pass);

        let err_opt = env.into_errors().pop();
        assert!(matches!(
            err_opt,
            Some(NamingPhaseError::Naming(NamingError::SelfOutsideClass(..)))
        ));
        assert!(matches!(
            elem,
            Hint(_, box Hint_::Haccess(Hint(_, box Hint_::Herr), _))
        ))
    }
    // -- type access through `this` -------------------------------------------

    #[test]
    // type access through `this` is always valid
    fn test_haccess_this_valid() {
        let env = Env::default();

        let mut pass = ElabHintHaccessPass::default();
        let mut elem = Hint(
            Pos::default(),
            Box::new(Hint_::Haccess(
                Hint(Pos::default(), Box::new(Hint_::Hthis)),
                vec![Id(Pos::default(), "T".to_string())],
            )),
        );
        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());
        assert!(matches!(
            elem,
            Hint(_, box Hint_::Haccess(Hint(_, box Hint_::Hthis), _))
        ))
    }

    // -- type access through `static` / `parent` ------------------------------
    #[test]
    // type access through `static` is invalid
    fn test_haccess_static_in_class_invalid() {
        let env = Env::default();

        let mut pass = ElabHintHaccessPass {
            current_class: Some("Classy".to_string()),
            ..Default::default()
        };
        let mut elem = Hint(
            Pos::default(),
            Box::new(Hint_::Haccess(
                Hint(
                    Pos::default(),
                    Box::new(Hint_::Happly(
                        Id(Pos::default(), sn::classes::STATIC.to_string()),
                        vec![],
                    )),
                ),
                vec![Id(Pos::default(), "T".to_string())],
            )),
        );
        elem.transform(&env, &mut pass);

        let err_opt = env.into_errors().pop();
        assert!(matches!(
            err_opt,
            Some(NamingPhaseError::Naming(
                NamingError::InvalidTypeAccessRoot { .. }
            ))
        ));
        assert!(matches!(
            elem,
            Hint(_, box Hint_::Haccess(Hint(_, box Hint_::Herr), _))
        ))
    }

    // -- type access through type param ---------------------------------------

    #[test]
    // type access through type parameter is valid inside a context
    fn test_haccess_tparam_in_context_valid() {
        let env = Env::default();

        let mut pass = ElabHintHaccessPass::default();
        let mut elem = Contexts(
            Pos::default(),
            vec![Hint(
                Pos::default(),
                Box::new(Hint_::Haccess(
                    Hint(
                        Pos::default(),
                        Box::new(Hint_::Habstr("T".to_string(), vec![])),
                    ),
                    vec![Id(Pos::default(), "C".to_string())],
                )),
            )],
        );
        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());
        let Contexts(_, mut hints) = elem;
        assert!(matches!(
            hints.pop(),
            Some(Hint(
                _,
                box Hint_::Haccess(Hint(_, box Hint_::Habstr(..)), _)
            ))
        ))
    }

    #[test]
    // type access through type parameter is valid inside a where clause
    fn test_haccess_tparam_in_where_clause_valid() {
        let env = Env::default();

        let mut pass = ElabHintHaccessPass::default();
        let mut elem = WhereConstraintHint(
            Hint(
                Pos::default(),
                Box::new(Hint_::Haccess(
                    Hint(
                        Pos::default(),
                        Box::new(Hint_::Habstr("T".to_string(), vec![])),
                    ),
                    vec![Id(Pos::default(), "C".to_string())],
                )),
            ),
            ConstraintKind::ConstraintSuper,
            Hint(Pos::default(), Box::new(Hint_::Herr)),
        );
        elem.transform(&env, &mut pass);

        assert!(env.into_errors().is_empty());
        let WhereConstraintHint(hint, _, _) = elem;
        assert!(matches!(
            hint,
            Hint(_, box Hint_::Haccess(Hint(_, box Hint_::Habstr(..)), _))
        ))
    }

    #[test]
    // type access through type parameter in any other context is invalid
    fn test_haccess_tparam_invalid() {
        let env = Env::default();

        let mut pass = ElabHintHaccessPass::default();
        let mut elem = Hint(
            Pos::default(),
            Box::new(Hint_::Haccess(
                Hint(
                    Pos::default(),
                    Box::new(Hint_::Habstr("T".to_string(), vec![])),
                ),
                vec![Id(Pos::default(), "C".to_string())],
            )),
        );
        elem.transform(&env, &mut pass);

        let err_opt = env.into_errors().pop();
        assert!(matches!(
            err_opt,
            Some(NamingPhaseError::Naming(
                NamingError::InvalidTypeAccessRoot { .. }
            ))
        ));
        assert!(matches!(
            elem,
            Hint(_, box Hint_::Haccess(Hint(_, box Hint_::Herr), _))
        ))
    }
}

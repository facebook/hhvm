// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::Hint;
use nast::Hint_;
use nast::Id;
use nast::Tparam;

use crate::prelude::*;

#[derive(Clone, Copy, Default)]
pub struct ElabHktPass;

impl Pass for ElabHktPass {
    fn on_ty_hint_top_down(&mut self, env: &Env, elem: &mut Hint) -> ControlFlow<()> {
        if !env.hkt_enabled() {
            let Hint(pos, hint_) = elem;
            if let Hint_::Habstr(tp_name, tp_params) = &mut **hint_ {
                if !tp_params.is_empty() {
                    env.emit_error(NamingError::TparamAppliedToType {
                        pos: pos.clone(),
                        tparam_name: tp_name.clone(),
                    });
                    tp_params.clear()
                }
            }
        }
        Continue(())
    }

    fn on_ty_tparam_top_down(&mut self, env: &Env, elem: &mut Tparam) -> ControlFlow<()> {
        if !env.hkt_enabled() {
            if !elem.parameters.is_empty() {
                let Id(pos, tp_name) = &elem.name;
                env.emit_error(NamingError::TparamWithTparam {
                    pos: pos.clone(),
                    tparam_name: tp_name.clone(),
                });
                elem.parameters.clear()
            }
        }
        Continue(())
    }
}

#[cfg(test)]
mod tests {

    use nast::Pos;
    use nast::ReifyKind;
    use nast::UserAttributes;
    use nast::Variance;
    use oxidized::typechecker_options::TypecheckerOptions;

    use super::*;
    use crate::env::ProgramSpecificOptions;

    #[test]
    fn test_hint() {
        let env_hkt_disabled = Env::new(
            &TypecheckerOptions::default(),
            &ProgramSpecificOptions::default(),
        );
        let env_hkt_enabled = Env::new(
            &TypecheckerOptions {
                tco_higher_kinded_types: true,
                ..Default::default()
            },
            &ProgramSpecificOptions::default(),
        );

        let mut pass = ElabHktPass;
        let mut elem = Hint(
            Pos::NONE,
            Box::new(Hint_::Habstr(
                "T".to_string(),
                vec![Hint(Pos::NONE, Box::new(Hint_::Hmixed))],
            )),
        );

        elem.transform(&env_hkt_enabled, &mut pass);
        let Hint(_, hint_) = &elem;
        assert!(match &**hint_ {
            Hint_::Habstr(_, hints) => !hints.is_empty(),
            _ => false,
        });
        assert_eq!(env_hkt_enabled.into_errors().len(), 0);

        elem.transform(&env_hkt_disabled, &mut pass);
        let Hint(_, hint_) = &elem;
        assert!(match &**hint_ {
            Hint_::Habstr(_, hints) => hints.is_empty(),
            _ => false,
        });
        assert_eq!(env_hkt_disabled.into_errors().len(), 1);
    }

    #[test]
    fn test_tparam() {
        let env_hkt_disabled = Env::new(
            &TypecheckerOptions::default(),
            &ProgramSpecificOptions::default(),
        );
        let env_hkt_enabled = Env::new(
            &TypecheckerOptions {
                tco_higher_kinded_types: true,
                ..Default::default()
            },
            &ProgramSpecificOptions::default(),
        );

        let mut pass = ElabHktPass;

        let mut elem = Tparam {
            variance: Variance::Invariant,
            name: Id(Pos::NONE, "T".to_string()),
            parameters: vec![Tparam {
                variance: Variance::Invariant,
                name: Id(Pos::NONE, "TInner".to_string()),
                parameters: vec![],
                constraints: vec![],
                reified: ReifyKind::Erased,
                user_attributes: UserAttributes::default(),
            }],
            constraints: vec![],
            reified: ReifyKind::Erased,
            user_attributes: UserAttributes::default(),
        };

        elem.transform(&env_hkt_enabled, &mut pass);
        assert!(!elem.parameters.is_empty());
        assert_eq!(env_hkt_enabled.into_errors().len(), 0);

        elem.transform(&env_hkt_disabled, &mut pass);
        assert!(elem.parameters.is_empty());
        assert_eq!(env_hkt_disabled.into_errors().len(), 1);
    }
}

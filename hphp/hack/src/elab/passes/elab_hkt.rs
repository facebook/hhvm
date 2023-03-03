// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use oxidized::aast_defs::Hint;
use oxidized::aast_defs::Hint_;
use oxidized::aast_defs::Tparam;
use oxidized::ast_defs::Id;
use oxidized::naming_error::NamingError;
use oxidized::naming_phase_error::NamingPhaseError;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ElabHktPass;

impl Pass for ElabHktPass {
    fn on_ty_hint_top_down(
        &mut self,
        elem: &mut Hint,
        cfg: &Config,
        errs: &mut Vec<NamingPhaseError>,
    ) -> ControlFlow<(), ()> {
        if !cfg.hkt_enabled() {
            let Hint(pos, hint_) = elem;
            if let Hint_::Habstr(tp_name, tp_params) = &mut **hint_ {
                if !tp_params.is_empty() {
                    let err = NamingPhaseError::Naming(NamingError::TparamAppliedToType {
                        pos: pos.clone(),
                        tparam_name: tp_name.clone(),
                    });
                    errs.push(err);
                    tp_params.clear()
                }
            }
        }
        ControlFlow::Continue(())
    }

    fn on_ty_tparam_top_down<Ex: Default, En>(
        &mut self,
        elem: &mut Tparam<Ex, En>,
        cfg: &Config,
        errs: &mut Vec<NamingPhaseError>,
    ) -> ControlFlow<(), ()> {
        if !cfg.hkt_enabled() {
            if !elem.parameters.is_empty() {
                let Id(pos, tp_name) = &elem.name;
                let err = NamingPhaseError::Naming(NamingError::TparamWithTparam {
                    pos: pos.clone(),
                    tparam_name: tp_name.clone(),
                });
                errs.push(err);
                elem.parameters.clear()
            }
        }
        ControlFlow::Continue(())
    }
}

#[cfg(test)]
mod tests {

    use oxidized::aast_defs::ReifyKind;
    use oxidized::aast_defs::UserAttributes;
    use oxidized::ast_defs::Variance;
    use oxidized::tast::Pos;
    use oxidized::typechecker_options::TypecheckerOptions;

    use super::*;
    use crate::config::ProgramSpecificOptions;
    use crate::Transform;

    #[test]
    fn test_hint() {
        let cfg_hkt_disabled = Config::new(
            &TypecheckerOptions {
                ..Default::default()
            },
            &ProgramSpecificOptions {
                ..Default::default()
            },
        );
        let cfg_hkt_enabled = Config::new(
            &TypecheckerOptions {
                tco_higher_kinded_types: true,
                ..Default::default()
            },
            &ProgramSpecificOptions {
                ..Default::default()
            },
        );

        let mut errs = Vec::default();
        let mut pass = ElabHktPass;
        let mut elem = Hint(
            Pos::NONE,
            Box::new(Hint_::Habstr(
                "T".to_string(),
                vec![Hint(Pos::NONE, Box::new(Hint_::Hmixed))],
            )),
        );

        elem.transform(&cfg_hkt_enabled, &mut errs, &mut pass);
        let Hint(_, hint_) = &elem;
        assert!(match &**hint_ {
            Hint_::Habstr(_, hints) => !hints.is_empty(),
            _ => false,
        });
        assert_eq!(errs.len(), 0);

        elem.transform(&cfg_hkt_disabled, &mut errs, &mut pass);
        let Hint(_, hint_) = &elem;
        assert!(match &**hint_ {
            Hint_::Habstr(_, hints) => hints.is_empty(),
            _ => false,
        });
        assert_eq!(errs.len(), 1);
    }

    #[test]
    fn test_tparam() {
        let cfg_hkt_disabled = Config::new(
            &TypecheckerOptions {
                ..Default::default()
            },
            &ProgramSpecificOptions {
                ..Default::default()
            },
        );
        let cfg_hkt_enabled = Config::new(
            &TypecheckerOptions {
                tco_higher_kinded_types: true,
                ..Default::default()
            },
            &ProgramSpecificOptions {
                ..Default::default()
            },
        );

        let mut errs = Vec::default();
        let mut pass = ElabHktPass;

        let mut elem: Tparam<(), ()> = Tparam {
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

        elem.transform(&cfg_hkt_enabled, &mut errs, &mut pass);
        assert!(!elem.parameters.is_empty());
        assert_eq!(errs.len(), 0);

        elem.transform(&cfg_hkt_disabled, &mut errs, &mut pass);
        assert!(elem.parameters.is_empty());
        assert_eq!(errs.len(), 1);
    }
}

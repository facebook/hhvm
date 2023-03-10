// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::aast_defs::Contexts;
use oxidized::aast_defs::Expr_;
use oxidized::aast_defs::Hint;
use oxidized::aast_defs::Hint_;
use oxidized::naming_error::NamingError;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ElabHintWildcardPass {
    depth: usize,
    allow_wildcard: bool,
}

impl ElabHintWildcardPass {
    pub fn reset_depth(&mut self) {
        self.depth = 0
    }
    pub fn incr_depth(&mut self) {
        self.depth += 1
    }
}

impl Pass for ElabHintWildcardPass {
    fn on_ty_hint_top_down(&mut self, elem: &mut Hint, cfg: &Config) -> ControlFlow<(), ()> {
        let Hint(pos, box hint_) = elem;
        //   Swap for `Herr`
        let in_hint_ = std::mem::replace(hint_, Hint_::Herr);
        match &in_hint_ {
            Hint_::Happly(id, hints) if id.name() == sn::typehints::WILDCARD => {
                if self.allow_wildcard && self.depth >= 1 {
                    // TODO[mjt] this looks like an HKT rule has been mixed in
                    // with wildcard checks?
                    if !hints.is_empty() {
                        // Since we are at depth >= 1, `_` must be a type parameter
                        // and should not be applied to other types
                        cfg.emit_error(NamingError::TparamAppliedToType {
                            pos: pos.clone(),
                            tparam_name: sn::typehints::WILDCARD.to_string(),
                        });
                        //  We've already set the hint to `Herr` so just break
                        ControlFlow::Break(())
                    } else {
                        // This is valid; restore the hint and continue
                        *hint_ = in_hint_;
                        ControlFlow::Continue(())
                    }
                } else {
                    // Wildcard hints are disallowed here; add an error
                    cfg.emit_error(NamingError::WildcardHintDisallowed(pos.clone()));
                    //  We've already set the hint to `Herr` so just break
                    ControlFlow::Break(())
                }
            }
            _ => {
                // This isn't a wildcard hint; restore the original hint and continue
                *hint_ = in_hint_;
                ControlFlow::Continue(())
            }
        }
    }

    // Wildcard hints are _always_ disallowed in contexts
    // TODO: we define this on `context` in OCaml - we need a newtype
    // to do the same here
    fn on_ty_contexts_top_down(
        &mut self,
        elem: &mut Contexts,
        cfg: &Config,
    ) -> ControlFlow<(), ()> {
        let Contexts(_, hints) = elem;
        hints
            .iter_mut()
            .filter(|hint| is_wildcard(hint))
            .for_each(|hint| {
                let Hint(pos, box hint_) = hint;
                cfg.emit_error(NamingError::InvalidWildcardContext(pos.clone()));
                *hint_ = Hint_::Herr
            });
        ControlFlow::Continue(())
    }

    fn on_ty_expr__top_down<Ex, En>(
        &mut self,
        elem: &mut oxidized::aast::Expr_<Ex, En>,
        _cfg: &Config,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        match elem {
            Expr_::Cast(..) => self.incr_depth(),
            Expr_::Is(..) | Expr_::As(..) => self.allow_wildcard = true,
            Expr_::Upcast(..) => self.allow_wildcard = false,
            _ => (),
        }
        ControlFlow::Continue(())
    }

    fn on_ty_targ_top_down<Ex>(
        &mut self,
        _elem: &mut oxidized::aast::Targ<Ex>,
        _cfg: &Config,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.allow_wildcard = true;
        self.incr_depth();
        ControlFlow::Continue(())
    }

    fn on_ty_hint__top_down(&mut self, elem: &mut Hint_, _cfg: &Config) -> ControlFlow<(), ()> {
        match elem {
            Hint_::Hunion(_)
            | Hint_::Hintersection(_)
            | Hint_::Hoption(_)
            | Hint_::Hlike(_)
            | Hint_::Hsoft(_)
            | Hint_::Hrefinement(..) => self.reset_depth(),
            Hint_::Htuple(_) | Hint_::Happly(..) | Hint_::Habstr(..) | Hint_::HvecOrDict(..) => {
                self.incr_depth()
            }
            _ => (),
        }
        ControlFlow::Continue(())
    }

    fn on_ty_shape_field_info_top_down(
        &mut self,
        _elem: &mut oxidized::tast::ShapeFieldInfo,
        _cfg: &Config,
    ) -> ControlFlow<(), ()> {
        self.incr_depth();
        ControlFlow::Continue(())
    }
}

fn is_wildcard(hint: &Hint) -> bool {
    match hint {
        Hint(_, box Hint_::Happly(id, _)) => id.name() == sn::typehints::WILDCARD,
        _ => false,
    }
}

#[cfg(test)]
mod tests {

    use oxidized::aast_defs::Targ;
    use oxidized::ast_defs::Id;
    use oxidized::naming_phase_error::NamingPhaseError;
    use oxidized::tast::Pos;

    use super::*;
    use crate::elab_utils;
    use crate::Transform;

    fn make_wildcard(hints: Vec<Hint>) -> Hint {
        Hint(
            Pos::default(),
            Box::new(Hint_::Happly(
                Id(Pos::default(), sn::typehints::WILDCARD.to_string()),
                hints,
            )),
        )
    }

    // -- Wildcard hints in expressions ----------------------------------------

    // -- Wilcard hints in `Is` (equivalently `As`) expressions ----------------

    #[test]
    // Wildcard hint at the top-level of a `Is` expression. `allow_wildcard`
    // will be true because of the `Is` expression but depth will be 0 since
    // we reach the `on_ty_hint_top_down` handler _before_ `on_ty_hint__top_down`
    // As a result, we will mark the hint as invalid and raise an error
    fn test_expr_is_top_level_invalid() {
        let cfg = Config::default();

        let mut pass = ElabHintWildcardPass::default();
        let mut elem: Expr_<(), ()> =
            Expr_::Is(Box::new((elab_utils::expr::null(), make_wildcard(vec![]))));
        elem.transform(&cfg, &mut pass);

        let wildcard_hint_disallowed_err_opt = cfg.into_errors().pop();
        assert!(matches!(
            wildcard_hint_disallowed_err_opt,
            Some(NamingPhaseError::Naming(
                NamingError::WildcardHintDisallowed(_)
            ))
        ));

        assert!(matches!(elem, Expr_::Is(box (_, Hint(_, box Hint_::Herr)))))
    }

    #[test]
    // Wildcard hint inside a tuple hint, in a `Is` expression. This means
    // we have (i) wildcards are allowed, (ii) depth is 1, (iii) the
    // wildcard isn't applied to other types so we expect it to be valid
    fn test_expr_is_nested_valid() {
        let cfg = Config::default();

        let mut pass = ElabHintWildcardPass::default();
        let mut elem: Expr_<(), ()> = Expr_::Is(Box::new((
            elab_utils::expr::null(),
            Hint(
                Pos::default(),
                Box::new(Hint_::Htuple(vec![make_wildcard(vec![])])),
            ),
        )));
        elem.transform(&cfg, &mut pass);

        assert!(cfg.into_errors().is_empty());
        assert!(match elem {
            Expr_::Is(box (_, Hint(_, box Hint_::Htuple(mut hints)))) =>
                hints.pop().map_or(false, |hint| is_wildcard(&hint)),
            _ => false,
        })
    }

    #[test]
    // Wildcard hint applied to another type at the top-level of a `Is` expression
    // Because we are at depth 0 we will raise the `WildcardDisallowedHint` error
    fn test_expr_is_top_level_hkt_invalid() {
        let cfg = Config::default();

        let mut pass = ElabHintWildcardPass::default();
        let mut elem: Expr_<(), ()> = Expr_::Is(Box::new((
            elab_utils::expr::null(),
            make_wildcard(vec![Hint(Pos::default(), Box::new(Hint_::Herr))]),
        )));
        elem.transform(&cfg, &mut pass);

        let wildcard_hint_disallowed_err_opt = cfg.into_errors().pop();
        assert!(matches!(
            wildcard_hint_disallowed_err_opt,
            Some(NamingPhaseError::Naming(
                NamingError::WildcardHintDisallowed(_)
            ))
        ));

        assert!(matches!(elem, Expr_::Is(box (_, Hint(_, box Hint_::Herr)))))
    }

    #[test]
    // Wildcard hint applied to another type at the top-level of a `Is` expression
    // Because we are at depth 1 we will raise the `TparamAppliedToType` error
    fn test_expr_is_nest_hkt_invalid() {
        let cfg = Config::default();

        let mut pass = ElabHintWildcardPass::default();
        let mut elem: Expr_<(), ()> = Expr_::Is(Box::new((
            elab_utils::expr::null(),
            Hint(
                Pos::default(),
                Box::new(Hint_::Htuple(vec![make_wildcard(vec![Hint(
                    Pos::default(),
                    Box::new(Hint_::Herr),
                )])])),
            ),
        )));
        elem.transform(&cfg, &mut pass);

        let tparam_applied_to_type_err_opt = cfg.into_errors().pop();
        assert!(matches!(
            tparam_applied_to_type_err_opt,
            Some(NamingPhaseError::Naming(
                NamingError::TparamAppliedToType { .. }
            ))
        ));

        assert!(match elem {
            Expr_::Is(box (_, Hint(_, box Hint_::Htuple(hints)))) => match hints.as_slice() {
                [Hint(_, box Hint_::Herr)] => true,
                _ => false,
            },
            _ => false,
        })
    }

    // -- Wildcard hint in `Upcast` expressions --------------------------------

    #[test]
    // Since `Upcast` sets `allow_wildcard` to false and the flag is only
    // set  to true in `targ`s or `Is`/`As` expressions we expect all wildcard
    // hints appearing here to be invalid
    fn test_expr_upcast_top_level_invalid() {
        let cfg = Config::default();

        let mut pass = ElabHintWildcardPass::default();
        let mut elem: Expr_<(), ()> =
            Expr_::Upcast(Box::new((elab_utils::expr::null(), make_wildcard(vec![]))));
        elem.transform(&cfg, &mut pass);

        let wildcard_hint_disallowed_err_opt = cfg.into_errors().pop();
        assert!(matches!(
            wildcard_hint_disallowed_err_opt,
            Some(NamingPhaseError::Naming(
                NamingError::WildcardHintDisallowed(_)
            ))
        ));

        assert!(matches!(
            elem,
            Expr_::Upcast(box (_, Hint(_, box Hint_::Herr)))
        ))
    }

    #[test]
    // Since `Upcast` sets `allow_wildcard` to false and the flag is only
    // set  to true in `targ`s or `Is`/`As` expressions we expect all wildcard
    // hints appearing here to be invalid
    fn test_expr_upcast_nested_invalid() {
        let cfg = Config::default();

        let mut pass = ElabHintWildcardPass::default();
        let mut elem: Expr_<(), ()> = Expr_::Upcast(Box::new((
            elab_utils::expr::null(),
            Hint(
                Pos::default(),
                Box::new(Hint_::Htuple(vec![make_wildcard(vec![Hint(
                    Pos::default(),
                    Box::new(Hint_::Herr),
                )])])),
            ),
        )));
        elem.transform(&cfg, &mut pass);

        let wildcard_hint_disallowed_err_opt = cfg.into_errors().pop();
        assert!(matches!(
            wildcard_hint_disallowed_err_opt,
            Some(NamingPhaseError::Naming(
                NamingError::WildcardHintDisallowed(_)
            ))
        ));

        assert!(match elem {
            Expr_::Upcast(box (_, Hint(_, box Hint_::Htuple(hints)))) => match hints.as_slice() {
                [Hint(_, box Hint_::Herr)] => true,
                _ => false,
            },
            _ => false,
        })
    }

    // -- Wildcard hint in `Cast` expressions ----------------------------------

    #[test]
    // Since `Cast` does not modify `allow_wildcard` we never expect it to be
    // valid
    fn test_expr_cast_top_level_invalid() {
        let cfg = Config::default();

        let mut pass = ElabHintWildcardPass::default();
        let mut elem: Expr_<(), ()> =
            Expr_::Cast(Box::new((make_wildcard(vec![]), elab_utils::expr::null())));
        elem.transform(&cfg, &mut pass);

        let wildcard_hint_disallowed_err_opt = cfg.into_errors().pop();
        assert!(matches!(
            wildcard_hint_disallowed_err_opt,
            Some(NamingPhaseError::Naming(
                NamingError::WildcardHintDisallowed(_)
            ))
        ));

        assert!(matches!(
            elem,
            Expr_::Cast(box (Hint(_, box Hint_::Herr), _))
        ))
    }

    // -- Wildcard hints in `Contexts` -----------------------------------------

    #[test]
    // Wildcard hints are always invalid inside `Contexts`
    fn test_contexts_top_level_invalid() {
        let cfg = Config::default();

        let mut pass = ElabHintWildcardPass::default();
        let mut elem: Contexts = Contexts(Pos::default(), vec![make_wildcard(vec![])]);
        elem.transform(&cfg, &mut pass);

        let invalid_wildcard_context_err_opt = cfg.into_errors().pop();
        assert!(matches!(
            invalid_wildcard_context_err_opt,
            Some(NamingPhaseError::Naming(
                NamingError::InvalidWildcardContext(_)
            ))
        ));

        let Contexts(_, mut hints) = elem;
        let hint_opt = hints.pop();

        assert!(matches!(hint_opt, Some(Hint(_, box Hint_::Herr))))
    }

    // -- Wildcard hints in `Targ`s --------------------------------------------

    #[test]
    // Wildcard hints are valid inside `Targ`s
    fn test_contexts_top_level_valid() {
        let cfg = Config::default();

        let mut pass = ElabHintWildcardPass::default();
        let mut elem: Targ<()> = Targ((), make_wildcard(vec![]));
        elem.transform(&cfg, &mut pass);

        assert!(cfg.into_errors().is_empty());
        let Targ(_, ref hint) = elem;
        assert!(is_wildcard(hint))
    }
}

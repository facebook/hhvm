// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::ops::ControlFlow;

use naming_special_names_rust as sn;
use oxidized::aast_defs::Afield;
use oxidized::aast_defs::CollectionTarg;
use oxidized::aast_defs::Expr;
use oxidized::aast_defs::Expr_;
use oxidized::aast_defs::Field;
use oxidized::aast_defs::Hint;
use oxidized::aast_defs::Hint_;
use oxidized::aast_defs::KvcKind;
use oxidized::aast_defs::Lid;
use oxidized::aast_defs::Targ;
use oxidized::aast_defs::VcKind;
use oxidized::ast_defs::Id;
use oxidized::local_id;
use oxidized::naming_error::NamingError;
use oxidized::tast::Pos;

use crate::config::Config;
use crate::Pass;

#[derive(Clone, Copy, Default)]
pub struct ElabExprCollectionPass;

impl Pass for ElabExprCollectionPass {
    /// Translate `Collection1 expressions received from lowering into
    /// the canonical representation of either:
    ///   - `ValCollection` for `Keyset`, `Vec`, (`Imm`)`Vector`, (`Imm`)`Set`
    ///   - `KeyValCollection` for `Dict` and (`Imm`)`Map`
    ///   - `Pair` for `Pair`
    ///
    /// If we have a collection with some other class name, we wrap in
    /// the `Invalid` expression marker.
    ///
    /// Elaboration into canonical representation may also reveal
    /// errors in the explicit type arguments and the expressions
    /// within the collection literal.

    fn on_ty_expr_top_down<Ex: Default, En>(
        &mut self,
        elem: &mut Expr<Ex, En>,
        cfg: &Config,
    ) -> ControlFlow<(), ()> {
        let Expr(_annot, _pos, expr_) = elem;

        if let Expr_::Collection(c) = expr_ {
            let (Id(pos, cname), ctarg_opt, afields) = c as &mut (_, _, Vec<Afield<Ex, En>>);

            match collection_kind(cname) {
                CollectionKind::VcKind(vc_kind) => {
                    let targ_opt = targ_from_collection_targs(cfg, ctarg_opt, pos);
                    let exprs: Vec<Expr<Ex, En>> = afields
                        .iter_mut()
                        .map(|afield| expr_from_afield(cfg, afield, cname))
                        .collect();
                    *expr_ = Expr_::ValCollection(Box::new((
                        (std::mem::replace(pos, Pos::NONE), vc_kind),
                        targ_opt,
                        exprs,
                    )));
                    ControlFlow::Continue(())
                }

                CollectionKind::KvcKind(kvc_kind) => {
                    let targs_opt = targs_from_collection_targs(cfg, ctarg_opt, pos);
                    let fields: Vec<Field<Ex, En>> = afields
                        .iter_mut()
                        .map(|afield| field_from_afield(cfg, afield, cname))
                        .collect();
                    *expr_ = Expr_::KeyValCollection(Box::new((
                        (std::mem::replace(pos, Pos::NONE), kvc_kind),
                        targs_opt,
                        fields,
                    )));
                    ControlFlow::Continue(())
                }

                CollectionKind::Pair => {
                    match &mut (afields.pop(), afields.pop(), afields.pop()) {
                        // We have exactly two args so this _may_ be a valid pair
                        (Some(afield2), Some(afield1), None) => {
                            let targs_opt = targs_from_collection_targs(cfg, ctarg_opt, pos);
                            let expr1 = expr_from_afield(cfg, afield1, cname);
                            let expr2 = expr_from_afield(cfg, afield2, cname);
                            *expr_ = Expr_::Pair(Box::new((targs_opt, expr1, expr2)));
                            ControlFlow::Continue(())
                        }

                        // We have fewer than two args, this cannot be a valid [Pair] so replace
                        // with [Invalid]
                        (_, None, _) => {
                            cfg.emit_error(NamingError::TooFewArguments(pos.clone()));
                            let inner_expr = std::mem::replace(
                                elem,
                                Expr(Ex::default(), Pos::NONE, Expr_::Null),
                            );
                            *elem = Expr(
                                Ex::default(),
                                inner_expr.1.clone(),
                                Expr_::Invalid(Box::new(Some(inner_expr))),
                            );
                            ControlFlow::Break(())
                        }

                        // We have more than two args, this cannot be a valid `Pair` so replace
                        // with `Invalid`.
                        _ => {
                            cfg.emit_error(NamingError::TooManyArguments(pos.clone()));
                            let inner_expr = std::mem::replace(
                                elem,
                                Expr(Ex::default(), Pos::NONE, Expr_::Null),
                            );
                            *elem = Expr(
                                Ex::default(),
                                inner_expr.1.clone(),
                                Expr_::Invalid(Box::new(Some(inner_expr))),
                            );
                            ControlFlow::Break(())
                        }
                    }
                }

                CollectionKind::NotACollection => {
                    cfg.emit_error(NamingError::ExpectedCollection {
                        pos: pos.clone(),
                        cname: cname.clone(),
                    });
                    let inner_expr =
                        std::mem::replace(elem, Expr(Ex::default(), Pos::NONE, Expr_::Null));
                    let Expr(_, expr_pos, _) = &inner_expr;
                    *elem = Expr(
                        Ex::default(),
                        expr_pos.clone(),
                        Expr_::Invalid(Box::new(Some(inner_expr))),
                    );
                    ControlFlow::Break(())
                }
            }
        } else {
            ControlFlow::Continue(())
        }
    }
}

/// Extract the expression from [AFvalue]s; if we encounter an [AFkvalue] we
/// raise an error and drop the value expression
fn expr_from_afield<Ex: Default, En>(
    cfg: &Config,
    afield: &mut Afield<Ex, En>,
    cname: &str,
) -> Expr<Ex, En> {
    match afield {
        Afield::AFvalue(e) => std::mem::replace(e, Expr(Ex::default(), Pos::NONE, Expr_::Null)),
        Afield::AFkvalue(e, _) => {
            let Expr(_, expr_pos, _) = &e;
            cfg.emit_error(NamingError::UnexpectedArrow {
                pos: expr_pos.clone(),
                cname: cname.to_string(),
            });
            std::mem::replace(e, Expr(Ex::default(), Pos::NONE, Expr_::Null))
        }
    }
}

/// Extract the expressions from [AFkvalue]s into a `Field`; if we encounter an
/// `AFvalue` we raise an error and generate a synthetic lvar as the second
/// expression in the `Field`
fn field_from_afield<Ex: Default, En>(
    cfg: &Config,
    afield: &mut Afield<Ex, En>,
    cname: &str,
) -> Field<Ex, En> {
    match afield {
        Afield::AFkvalue(ek, ev) => {
            let ek = std::mem::replace(ek, Expr(Ex::default(), Pos::NONE, Expr_::Null));
            let ev = std::mem::replace(ev, Expr(Ex::default(), Pos::NONE, Expr_::Null));
            Field(ek, ev)
        }
        Afield::AFvalue(e) => {
            cfg.emit_error(NamingError::MissingArrow {
                pos: e.1.clone(),
                cname: cname.to_string(),
            });
            let ek = std::mem::replace(e, Expr(Ex::default(), Pos::NONE, Expr_::Null));
            // TODO[mjt]: replace with `Invalid` expression?
            let ev = Expr(
                Ex::default(),
                Pos::NONE,
                Expr_::Lvar(Box::new(Lid(
                    ek.1.clone(),
                    local_id::make_unscoped("__internal_placeholder"),
                ))),
            );
            Field(ek, ev)
        }
    }
}

// Get val collection hint if present; if we a keyval hint, raise an error and return `None`
fn targ_from_collection_targs<Ex: Default>(
    cfg: &Config,
    ctarg_opt: &mut Option<CollectionTarg<Ex>>,
    pos: &Pos,
) -> Option<Targ<Ex>> {
    if let Some(ctarg) = ctarg_opt {
        match ctarg {
            CollectionTarg::CollectionTV(tv) => {
                let tv = std::mem::replace(
                    tv,
                    Targ(Ex::default(), Hint(Pos::NONE, Box::new(Hint_::Herr))),
                );
                Some(tv)
            }
            CollectionTarg::CollectionTKV(..) => {
                cfg.emit_error(NamingError::TooManyArguments(pos.clone()));
                None
            }
        }
    } else {
        None
    }
}

// Get keyval collection hint if present; if we a val hint, raise an error and return `None`
fn targs_from_collection_targs<Ex: Default>(
    cfg: &Config,
    ctarg_opt: &mut Option<CollectionTarg<Ex>>,
    pos: &Pos,
) -> Option<(Targ<Ex>, Targ<Ex>)> {
    if let Some(ctarg) = ctarg_opt {
        match ctarg {
            CollectionTarg::CollectionTKV(tk, tv) => {
                let tk = std::mem::replace(
                    tk,
                    Targ(Ex::default(), Hint(Pos::NONE, Box::new(Hint_::Herr))),
                );
                let tv = std::mem::replace(
                    tv,
                    Targ(Ex::default(), Hint(Pos::NONE, Box::new(Hint_::Herr))),
                );
                Some((tk, tv))
            }
            CollectionTarg::CollectionTV(..) => {
                cfg.emit_error(NamingError::TooFewArguments(pos.clone()));
                None
            }
        }
    } else {
        None
    }
}

enum CollectionKind {
    VcKind(VcKind),
    KvcKind(KvcKind),
    Pair,
    NotACollection,
}

// Determine the `CollectionKind` based on class name
fn collection_kind(name: &str) -> CollectionKind {
    if let Some(kind) = vc_kind_opt(name) {
        CollectionKind::VcKind(kind)
    } else if let Some(kind) = kvc_kind_opt(name) {
        CollectionKind::KvcKind(kind)
    } else if name == sn::collections::PAIR {
        CollectionKind::Pair
    } else {
        CollectionKind::NotACollection
    }
}

fn kvc_kind_opt(name: &str) -> Option<KvcKind> {
    match name {
        sn::collections::MAP => Some(KvcKind::Map),
        sn::collections::IMM_MAP => Some(KvcKind::ImmMap),
        sn::collections::DICT => Some(KvcKind::Dict),
        _ => None,
    }
}

fn vc_kind_opt(name: &str) -> Option<VcKind> {
    match name {
        sn::collections::VECTOR => Some(VcKind::Vector),
        sn::collections::IMM_VECTOR => Some(VcKind::ImmVector),
        sn::collections::SET => Some(VcKind::Set),
        sn::collections::IMM_SET => Some(VcKind::ImmSet),
        sn::collections::KEYSET => Some(VcKind::Keyset),
        sn::collections::VEC => Some(VcKind::Vec),
        _ => None,
    }
}

#[cfg(test)]
mod tests {

    use oxidized::naming_phase_error::NamingPhaseError;

    use super::*;
    use crate::Transform;

    // -- ValCollection --------------------------------------------------------

    #[test]
    fn test_val_collection_empty() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::VEC.to_string()),
                None,
                vec![],
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert_eq!(cfg.into_errors().len(), 0);
        assert!(matches!(expr_, Expr_::ValCollection(_)));
    }

    #[test]
    fn test_val_collection_afvalue() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::VEC.to_string()),
                None,
                vec![Afield::AFvalue(Expr(
                    (),
                    Pos::NONE,
                    Expr_::Int("42".to_string()),
                ))],
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert_eq!(cfg.into_errors().len(), 0);
        assert!(matches!(expr_, Expr_::ValCollection(_)));
    }

    #[test]
    fn test_val_collection_afkvalue() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::VEC.to_string()),
                None,
                vec![Afield::AFkvalue(
                    Expr((), Pos::NONE, Expr_::Int("42".to_string())),
                    Expr((), Pos::NONE, Expr_::True),
                )],
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert!(matches!(
            cfg.into_errors().pop(),
            Some(NamingPhaseError::Naming(
                NamingError::UnexpectedArrow { .. }
            ))
        ));
        assert!(matches!(expr_, Expr_::ValCollection(_)));
    }

    #[test]
    fn test_val_collection_val_arg() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::VEC.to_string()),
                Some(CollectionTarg::CollectionTV(Targ(
                    (),
                    Hint(Pos::NONE, Box::new(Hint_::Hnothing)),
                ))),
                vec![],
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert_eq!(cfg.into_errors().len(), 0);
        assert!(matches!(expr_, Expr_::ValCollection(_)));
    }

    #[test]
    fn test_val_collection_key_val_arg() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::VEC.to_string()),
                Some(CollectionTarg::CollectionTKV(
                    Targ((), Hint(Pos::NONE, Box::new(Hint_::Hnothing))),
                    Targ((), Hint(Pos::NONE, Box::new(Hint_::Hnothing))),
                )),
                vec![],
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert!(matches!(
            cfg.into_errors().pop(),
            Some(NamingPhaseError::Naming(NamingError::TooManyArguments(_)))
        ));
        assert!(match expr_ {
            Expr_::ValCollection(b) => {
                let (_, targ_opt, _) = *b;
                targ_opt.is_none()
            }
            _ => false,
        });
    }

    // -- KeyValCollection -----------------------------------------------------

    #[test]
    fn test_key_val_collection_empty() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::DICT.to_string()),
                None,
                vec![],
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert_eq!(cfg.into_errors().len(), 0);
        assert!(matches!(expr_, Expr_::KeyValCollection(_)));
    }

    #[test]
    fn test_key_val_collection_afvalue() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::MAP.to_string()),
                None,
                vec![Afield::AFvalue(Expr(
                    (),
                    Pos::NONE,
                    Expr_::Int("42".to_string()),
                ))],
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert!(matches!(
            cfg.into_errors().pop(),
            Some(NamingPhaseError::Naming(NamingError::MissingArrow { .. }))
        ));
        assert!(matches!(expr_, Expr_::KeyValCollection(_)));
    }

    #[test]
    fn test_key_val_collection_afkvalue() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::MAP.to_string()),
                None,
                vec![Afield::AFkvalue(
                    Expr((), Pos::NONE, Expr_::Int("42".to_string())),
                    Expr((), Pos::NONE, Expr_::True),
                )],
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert_eq!(cfg.into_errors().len(), 0);
        assert!(matches!(expr_, Expr_::KeyValCollection(_)));
    }

    #[test]
    fn test_key_val_collection_val_arg() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::MAP.to_string()),
                Some(CollectionTarg::CollectionTV(Targ(
                    (),
                    Hint(Pos::NONE, Box::new(Hint_::Hnothing)),
                ))),
                vec![],
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert!(matches!(
            cfg.into_errors().pop(),
            Some(NamingPhaseError::Naming(NamingError::TooFewArguments(_)))
        ));
        assert!(match expr_ {
            Expr_::KeyValCollection(b) => {
                let (_, targ_opt, _) = *b;
                targ_opt.is_none()
            }
            _ => false,
        });
    }

    #[test]
    fn test_key_val_collection_key_val_arg() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::MAP.to_string()),
                Some(CollectionTarg::CollectionTKV(
                    Targ((), Hint(Pos::NONE, Box::new(Hint_::Hnothing))),
                    Targ((), Hint(Pos::NONE, Box::new(Hint_::Hnothing))),
                )),
                vec![],
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert_eq!(cfg.into_errors().len(), 0);
        assert!(matches!(expr_, Expr_::KeyValCollection(_)));
    }
    // -- Pair -----------------------------------------------------------------
    #[test]
    fn test_pair() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::PAIR.to_string()),
                None,
                vec![
                    Afield::AFvalue(Expr((), Pos::NONE, Expr_::Int("42".to_string()))),
                    Afield::AFvalue(Expr((), Pos::NONE, Expr_::True)),
                ],
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert_eq!(cfg.into_errors().len(), 0);
        assert!(matches!(expr_, Expr_::Pair(_)));
    }

    #[test]
    fn test_pair_too_few_exprs() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::PAIR.to_string()),
                None,
                vec![Afield::AFvalue(Expr(
                    (),
                    Pos::NONE,
                    Expr_::Int("42".to_string()),
                ))],
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert!(matches!(
            cfg.into_errors().pop(),
            Some(NamingPhaseError::Naming(NamingError::TooFewArguments(_)))
        ));
        assert!(matches!(expr_, Expr_::Invalid(_)));
    }

    #[test]
    fn test_pair_too_many_exprs() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::PAIR.to_string()),
                None,
                vec![
                    Afield::AFvalue(Expr((), Pos::NONE, Expr_::Int("42".to_string()))),
                    Afield::AFvalue(Expr((), Pos::NONE, Expr_::True)),
                    Afield::AFvalue(Expr((), Pos::NONE, Expr_::Null)),
                ],
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert!(matches!(
            cfg.into_errors().pop(),
            Some(NamingPhaseError::Naming(NamingError::TooManyArguments(_)))
        ));
        assert!(matches!(expr_, Expr_::Invalid(_)));
    }

    #[test]
    fn test_pair_val_arg() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::PAIR.to_string()),
                Some(CollectionTarg::CollectionTV(Targ(
                    (),
                    Hint(Pos::NONE, Box::new(Hint_::Hnothing)),
                ))),
                vec![
                    Afield::AFvalue(Expr((), Pos::NONE, Expr_::Int("42".to_string()))),
                    Afield::AFvalue(Expr((), Pos::NONE, Expr_::True)),
                ],
            ))),
        );

        elem.transform(&cfg, &mut pass);

        let Expr(_, _, expr_) = elem;
        assert!(matches!(
            cfg.into_errors().pop(),
            Some(NamingPhaseError::Naming(NamingError::TooFewArguments(_)))
        ));
        assert!(match expr_ {
            Expr_::Pair(b) => {
                let (targ_opt, _, _) = *b;
                targ_opt.is_none()
            }
            _ => false,
        });
    }

    #[test]
    fn test_pair_key_val_arg() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::collections::PAIR.to_string()),
                Some(CollectionTarg::CollectionTKV(
                    Targ((), Hint(Pos::NONE, Box::new(Hint_::Hnothing))),
                    Targ((), Hint(Pos::NONE, Box::new(Hint_::Hnothing))),
                )),
                vec![
                    Afield::AFvalue(Expr((), Pos::NONE, Expr_::Int("42".to_string()))),
                    Afield::AFvalue(Expr((), Pos::NONE, Expr_::True)),
                ],
            ))),
        );
        elem.transform(&cfg, &mut pass);
        let Expr(_, _, expr_) = elem;
        assert_eq!(cfg.into_errors().len(), 0);
        assert!(matches!(expr_, Expr_::Pair(_)));
    }

    // -- Not a collection -----------------------------------------------------

    #[test]
    fn test_not_a_collection() {
        let cfg = Config::default();

        let mut pass = ElabExprCollectionPass;

        let mut elem: Expr<(), ()> = Expr(
            (),
            Pos::NONE,
            Expr_::Collection(Box::new((
                Id(Pos::NONE, sn::classes::DATE_TIME.to_string()),
                None,
                vec![],
            ))),
        );
        elem.transform(&cfg, &mut pass);
        let Expr(_, _, expr_) = elem;
        assert!(matches!(
            cfg.into_errors().pop(),
            Some(NamingPhaseError::Naming(
                NamingError::ExpectedCollection { .. }
            ))
        ));
        assert!(matches!(expr_, Expr_::Invalid(_)));
    }
}

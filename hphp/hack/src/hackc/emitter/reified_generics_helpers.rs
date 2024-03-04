// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use decl_provider::TypeDecl;
use env::emitter::Emitter;
use env::ClassExpr;
use env::Env;
use error::Result;
use hash::HashSet;
use instruction_sequence::instr;
use instruction_sequence::InstrSeq;
use naming_special_names_rust as sn;
use oxidized::aast;
use oxidized::ast_defs::Id;
use oxidized::pos::Pos;

use crate::emit_expression::emit_reified_arg;

#[derive(Debug, Clone)]
pub enum ReificationLevel {
    /// There is a reified generic
    Definitely,
    /// There is no function or class reified generics, but there may be an inferred one
    Maybe,
    Not,
    Unconstrained,
}
impl ReificationLevel {
    fn combine(v1: &Self, v2: &Self) -> Self {
        match (v1, v2) {
            (Self::Definitely, _) | (_, Self::Definitely) => Self::Definitely,
            (Self::Maybe, _) | (_, Self::Maybe) => Self::Maybe,
            _ => Self::Not,
        }
    }
}

pub(crate) fn get_erased_tparams<'a>(env: &'a Env<'a>) -> impl Iterator<Item = String> + 'a {
    env.scope.get_tparams().into_iter().filter_map(|tp| {
        if tp.reified != aast::ReifyKind::Reified {
            Some(tp.name.1.clone()) // TODO(hrust) figure out how to return &str
        } else {
            None
        }
    })
}

pub(crate) fn has_reified_type_constraint<'a>(env: &Env<'a>, h: &aast::Hint) -> ReificationLevel {
    use aast::Hint_;
    fn is_all_erased<'a>(
        env: &'a Env<'_>,
        mut h_iter: impl Iterator<Item = &'a aast::Hint>,
    ) -> bool {
        let erased_tparams: HashSet<String> = get_erased_tparams(env).collect();
        h_iter.all(|h| match &*h.1 {
            Hint_::Hwildcard => true,
            Hint_::Happly(Id(_, ref id), ref apply_hints) => {
                apply_hints.is_empty() && erased_tparams.contains(id)
            }
            _ => false,
        })
    }
    match &*h.1 {
        Hint_::Happly(Id(_, id), hs) => {
            if ClassExpr::is_reified_tparam(&env.scope, id) {
                ReificationLevel::Definitely
            } else if hs.is_empty() || is_all_erased(env, hs.iter()) {
                ReificationLevel::Not
            } else {
                hs.iter().rev().fold(ReificationLevel::Maybe, |v, h| {
                    ReificationLevel::combine(&v, &has_reified_type_constraint(env, h))
                })
            }
        }
        Hint_::Hsoft(h) | Hint_::Hlike(h) | Hint_::HclassArgs(h) | Hint_::Hoption(h) => {
            has_reified_type_constraint(env, h)
        }
        Hint_::Hprim(_)
        | Hint_::Hmixed
        | Hint_::Hwildcard
        | Hint_::Hnonnull
        | Hint_::HvecOrDict(_, _)
        | Hint_::Hthis
        | Hint_::Hnothing
        | Hint_::Hdynamic
        | Hint_::Htuple(_)
        | Hint_::Hunion(_)
        | Hint_::Hintersection(_)
        | Hint_::Hshape(_)
        | Hint_::Hfun(_)
        | Hint_::Haccess(_, _)
        | Hint_::Hrefinement(_, _)
        | Hint_::HfunContext(_)
        | Hint_::Hvar(_) => ReificationLevel::Not,
        // Not found in the original AST
        Hint_::Habstr(_, _) => panic!("TODO Unimplemented: Not in the original AST"),
    }
}

fn remove_awaitable(aast::Hint(pos, hint): aast::Hint) -> aast::Hint {
    use aast::Hint;
    use aast::Hint_;
    match *hint {
        Hint_::Happly(sid, mut hs)
            if hs.len() == 1 && sid.1.eq_ignore_ascii_case(sn::classes::AWAITABLE) =>
        {
            hs.pop().unwrap()
        }
        // For @Awaitable<T>, the soft type hint is moved to the inner type, i.e @T
        Hint_::Hsoft(h) => Hint(pos, Box::new(Hint_::Hsoft(remove_awaitable(h)))),
        // For ~Awaitable<T>, the like-type  hint is moved to the inner type, i.e ~T
        Hint_::Hlike(h) => Hint(pos, Box::new(Hint_::Hlike(remove_awaitable(h)))),
        // For ?Awaitable<T>, the optional is dropped
        Hint_::Hoption(h) => remove_awaitable(h),
        Hint_::Htuple(_)
        | Hint_::Hunion(_)
        | Hint_::Hintersection(_)
        | Hint_::Hshape(_)
        | Hint_::Hfun(_)
        | Hint_::Haccess(_, _)
        | Hint_::Hrefinement(_, _)
        | Hint_::HclassArgs(_)
        | Hint_::Happly(_, _)
        | Hint_::HfunContext(_)
        | Hint_::Hvar(_)
        | Hint_::Hwildcard => Hint(pos, hint),
        Hint_::Hmixed
        | Hint_::Hnonnull
        | Hint_::Habstr(_, _)
        | Hint_::HvecOrDict(_, _)
        | Hint_::Hprim(_)
        | Hint_::Hthis
        | Hint_::Hnothing
        | Hint_::Hdynamic => panic!("TODO Unimplemented Did not exist on legacy AST"),
    }
}

pub(crate) fn convert_awaitable<'a>(env: &Env<'a>, h: aast::Hint) -> aast::Hint {
    if env.scope.is_in_async() {
        remove_awaitable(h)
    } else {
        h
    }
}

pub(crate) fn simplify_verify_type<'a, 'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    env: &mut Env<'a>,
    pos: &Pos,
    check: InstrSeq,
    hint: &aast::Hint,
    verify_instr: InstrSeq,
) -> Result<InstrSeq> {
    let get_ts = |e, hint| Ok(emit_reified_arg(e, env, pos, false, hint)?.0);
    let aast::Hint(_, hint_) = hint;
    if let aast::Hint_::Hoption(ref hint) = **hint_ {
        let label_gen = e.label_gen_mut();
        let done_label = label_gen.next_regular();
        Ok(InstrSeq::gather(vec![
            check,
            instr::jmp_nz(done_label),
            get_ts(e, hint)?,
            verify_instr,
            instr::label(done_label),
        ]))
    } else {
        Ok(InstrSeq::gather(vec![get_ts(e, hint)?, verify_instr]))
    }
}

pub(crate) fn remove_erased_generics<'a>(env: &Env<'a>, h: aast::Hint) -> aast::Hint {
    use aast::Hint;
    use aast::Hint_;
    use aast::NastShapeInfo;
    use aast::ShapeFieldInfo;
    fn rec<'a>(env: &Env<'a>, Hint(pos, h_): Hint) -> Hint {
        let h_ = match *h_ {
            Hint_::Happly(Id(pos, id), hs) => {
                if get_erased_tparams(env).any(|p| p == id) {
                    Hint_::Hwildcard
                } else {
                    Hint_::Happly(Id(pos, id), hs.into_iter().map(|h| rec(env, h)).collect())
                }
            }
            Hint_::Hsoft(h) => Hint_::Hsoft(rec(env, h)),
            Hint_::Hlike(h) => Hint_::Hlike(rec(env, h)),
            Hint_::HclassArgs(h) => Hint_::HclassArgs(rec(env, h)),
            Hint_::Hoption(h) => Hint_::Hoption(rec(env, h)),
            Hint_::Htuple(hs) => Hint_::Htuple(hs.into_iter().map(|h| rec(env, h)).collect()),
            Hint_::Hunion(hs) => Hint_::Hunion(hs.into_iter().map(|h| rec(env, h)).collect()),
            Hint_::Hintersection(hs) => {
                Hint_::Hintersection(hs.into_iter().map(|h| rec(env, h)).collect())
            }
            Hint_::Hshape(NastShapeInfo {
                allows_unknown_fields,
                field_map,
            }) => {
                let field_map = field_map
                    .into_iter()
                    .map(|sfi: ShapeFieldInfo| ShapeFieldInfo {
                        hint: rec(env, sfi.hint),
                        ..sfi
                    })
                    .collect();
                Hint_::Hshape(NastShapeInfo {
                    allows_unknown_fields,
                    field_map,
                })
            }
            h_ @ Hint_::Hfun(_)
            | h_ @ Hint_::Haccess(_, _)
            | h_ @ Hint_::Hrefinement(_, _)
            | h_ @ Hint_::Hwildcard => h_,
            Hint_::Hmixed
            | Hint_::Hnonnull
            | Hint_::Habstr(_, _)
            | Hint_::HvecOrDict(_, _)
            | Hint_::Hprim(_)
            | Hint_::Hthis
            | Hint_::Hnothing
            | Hint_::Hdynamic => panic!("TODO Unimplemented Did not exist on legacy AST"),
            Hint_::HfunContext(_) | Hint_::Hvar(_) => {
                panic!("Coeffects are currently erased during compilation")
            }
        };
        Hint(pos, Box::new(h_))
    }
    rec(env, h)
}

/// Warning: Experimental usage of decl-directed bytecode compilation.
/// Given a hint, if the hint is an Happly(id, _), checks if the id is a class
/// that has reified generics.
pub(crate) fn happly_decl_has_reified_generics<'a, 'arena, 'decl>(
    env: &Env<'a>,
    emitter: &mut Emitter<'arena, 'decl>,
    aast::Hint(_, hint): &aast::Hint,
) -> bool {
    use aast::Hint_;
    use aast::ReifyKind;
    match hint.as_ref() {
        Hint_::Happly(Id(_, id), _) => {
            // If the parameter itself is a reified type parameter, then we want to do the
            // tparam check
            if ClassExpr::is_reified_tparam(&env.scope, id) {
                return true;
            }
            // If the parameter is an erased type parameter, then no check is necessary
            if get_erased_tparams(env).any(|tparam| &tparam == id) {
                return false;
            }
            // Otherwise, we have a class or typedef name that we want to look up
            let provider = match emitter.decl_provider.as_ref() {
                Some(p) if emitter.options().hhbc.optimize_reified_param_checks => p,
                Some(_) | None => {
                    // If we don't have a `DeclProvider` available, or this specific optimization
                    // has been turned off, assume that this may be a refied generic class.
                    return true;
                }
            };
            match provider.type_decl(id, 0) {
                Ok(TypeDecl::Class(class_decl)) => {
                    // Found a class with a matching name. Does it's shallow decl have
                    // any reified tparams?
                    class_decl
                        .tparams
                        .iter()
                        .any(|tparam| tparam.reified != ReifyKind::Erased)
                }
                Ok(TypeDecl::Typedef(_)) => {
                    // TODO: `id` could be an alias for something without reified generics,
                    // but conservatively assume it has at least one, for now.
                    true
                }
                Err(decl_provider::Error::NotFound) => {
                    // The DeclProvider has no idea what `id` is.
                    true
                }
                Err(decl_provider::Error::Bincode(_)) => {
                    // Infra error while handling serialized decls
                    true
                }
            }
        }
        Hint_::Hoption(_)
        | Hint_::Hlike(_)
        | Hint_::Hfun(_)
        | Hint_::Htuple(_)
        | Hint_::Hshape(_)
        | Hint_::Haccess(_, _)
        | Hint_::Hsoft(_) => {
            // Assume any of these types could have reified generics.
            true
        }
        x => {
            // Other AST nodes are impossible here, must be a bug.
            unreachable!("unexpected AST node: {:#?}", x)
        }
    }
}

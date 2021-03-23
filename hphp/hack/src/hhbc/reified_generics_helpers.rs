// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use emit_expression_rust::{emit_reified_arg, is_reified_tparam};
use env::{emitter::Emitter, Env};
use instruction_sequence::*;
use naming_special_names_rust as sn;
use oxidized::{aast, ast_defs::Id, pos::Pos};

use std::collections::HashSet;

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

pub(crate) fn has_reified_type_constraint(env: &Env, h: &aast::Hint) -> ReificationLevel {
    use aast::*;
    fn is_all_erased<'a>(env: &'a Env, mut h_iter: impl Iterator<Item = &'a aast::Hint>) -> bool {
        let erased_tparams: HashSet<String> = get_erased_tparams(env).collect();
        h_iter.all(|h| {
            if let &Hint_::Happly(Id(_, ref id), ref apply_hints) = &*h.1 {
                if apply_hints.is_empty() {
                    return erased_tparams.contains(id);
                }
            }
            false
        })
    }
    match &*h.1 {
        Hint_::Happly(Id(_, id), hs) => {
            if is_reified_tparam(env, true, &id).is_some()
                || is_reified_tparam(env, false, &id).is_some()
            {
                ReificationLevel::Definitely
            } else if hs.is_empty() || is_all_erased(env, hs.iter()) {
                ReificationLevel::Not
            } else {
                hs.iter().rev().fold(ReificationLevel::Maybe, |v, h| {
                    ReificationLevel::combine(&v, &has_reified_type_constraint(env, h))
                })
            }
        }
        Hint_::Hsoft(h) | Hint_::Hlike(h) | Hint_::Hoption(h) => {
            has_reified_type_constraint(env, h)
        }
        Hint_::Hprim(_)
        | Hint_::Hmixed
        | Hint_::Hnonnull
        | Hint_::Hdarray(_, _)
        | Hint_::Hvarray(_)
        | Hint_::HvarrayOrDarray(_, _)
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
        | Hint_::HfunContext(_)
        | Hint_::Hvar(_) => ReificationLevel::Not,
        // Not found in the original AST
        Hint_::Herr | Hint_::Hany => panic!("Should be a naming error"),
        Hint_::Habstr(_, _) => panic!("TODO Unimplemented: Not in the original AST"),
    }
}

fn remove_awaitable(h: aast::Hint) -> aast::Hint {
    use aast::*;
    let Hint(pos, h_) = h;
    match *h_ {
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
        | Hint_::Happly(_, _)
        | Hint_::HfunContext(_)
        | Hint_::Hvar(_) => Hint(pos, h_),
        Hint_::Herr
        | Hint_::Hany
        | Hint_::Hmixed
        | Hint_::Hnonnull
        | Hint_::Habstr(_, _)
        | Hint_::Hdarray(_, _)
        | Hint_::Hvarray(_)
        | Hint_::HvarrayOrDarray(_, _)
        | Hint_::HvecOrDict(_, _)
        | Hint_::Hprim(_)
        | Hint_::Hthis
        | Hint_::Hnothing
        | Hint_::Hdynamic => panic!("TODO Unimplemented Did not exist on legacy AST"),
    }
}

pub(crate) fn convert_awaitable(env: &Env, h: aast::Hint) -> aast::Hint {
    if env.scope.is_in_async() {
        remove_awaitable(h)
    } else {
        h
    }
}

pub(crate) fn simplify_verify_type(
    e: &mut Emitter,
    env: &mut Env,
    pos: &Pos,
    check: InstrSeq,
    hint: &aast::Hint,
    verify_instr: InstrSeq,
) -> Result {
    let get_ts = |e, hint| Ok(emit_reified_arg(e, env, pos, false, hint)?.0);
    let aast::Hint(_, hint_) = hint;
    if let aast::Hint_::Hoption(ref hint) = **hint_ {
        let label_gen = e.label_gen_mut();
        let done_label = label_gen.next_regular();
        Ok(InstrSeq::gather(vec![
            check,
            instr::jmpnz(done_label.clone()),
            get_ts(e, hint)?,
            verify_instr,
            instr::label(done_label),
        ]))
    } else {
        Ok(InstrSeq::gather(vec![get_ts(e, hint)?, verify_instr]))
    }
}

pub(crate) fn remove_erased_generics(env: &Env, h: aast::Hint) -> aast::Hint {
    use aast::*;
    fn rec(env: &Env, Hint(pos, h_): aast::Hint) -> aast::Hint {
        fn modify(env: &Env, id: String) -> String {
            if get_erased_tparams(env).any(|p| p == id) {
                "_".into()
            } else {
                id
            }
        }
        let h_ = match *h_ {
            Hint_::Happly(Id(pos, id), hs) => Hint_::Happly(
                Id(pos, modify(env, id)),
                hs.into_iter().map(|h| rec(env, h)).collect(),
            ),
            Hint_::Hsoft(h) => Hint_::Hsoft(rec(env, h)),
            Hint_::Hlike(h) => Hint_::Hlike(rec(env, h)),
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
            h_ @ Hint_::Hfun(_) | h_ @ Hint_::Haccess(_, _) => h_,
            Hint_::Herr
            | Hint_::Hany
            | Hint_::Hmixed
            | Hint_::Hnonnull
            | Hint_::Habstr(_, _)
            | Hint_::Hdarray(_, _)
            | Hint_::Hvarray(_)
            | Hint_::HvarrayOrDarray(_, _)
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

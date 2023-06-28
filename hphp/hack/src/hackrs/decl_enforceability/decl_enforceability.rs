// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use pos::Positioned;
use special_names as sn;
use ty::decl;
use ty::decl::Ty;
use ty::reason::Reason;

pub fn supportdyn_mixed<R: Reason>(pos: R::Pos, reason: R) -> Ty<R> {
    make_supportdyn_type(pos, reason.clone(), Ty::mixed(reason))
}

fn make_supportdyn_type<R: Reason>(pos: R::Pos, reason: R, ty: Ty<R>) -> Ty<R> {
    Ty::apply(
        reason,
        Positioned::new(pos, *sn::classes::cSupportDyn),
        [ty].into(),
    )
}

/// Add `as supportdyn<mixed>` constraints to the type parameters
pub fn add_supportdyn_constraints<R: Reason>(pos: &R::Pos, tparams: &mut [decl::Tparam<R, Ty<R>>]) {
    for tparam in tparams {
        if !sn::coeffects::is_generated_generic(tparam.name.id()) && !noautobound(tparam) {
            let mut constraints = Vec::with_capacity(1 + tparam.constraints.len());
            constraints.push((
                decl::ty::ConstraintKind::ConstraintAs,
                supportdyn_mixed(pos.clone(), R::witness_from_decl(pos.clone())),
            ));
            constraints.extend(tparam.constraints.iter().cloned());
            tparam.constraints = constraints.into_boxed_slice()
        }
    }
}

/// Add `as supportdyn<mixed>` constraints to the type parameters if in implicit
/// pessimisation mode.
pub fn maybe_add_supportdyn_constraints<R: Reason>(
    opts: &oxidized::typechecker_options::TypecheckerOptions,
    this_class: Option<&decl::ShallowClass<R>>,
    pos: &R::Pos,
    tparams: &mut [decl::Tparam<R, Ty<R>>],
) {
    if implicit_sdt_for_class(opts, this_class) {
        add_supportdyn_constraints(pos, tparams)
    }
}

fn noautodynamic<R: Reason>(this_class: Option<&decl::ShallowClass<R>>) -> bool {
    match this_class {
        None => false,
        Some(sc) => sc
            .user_attributes
            .iter()
            .any(|ua| ua.name.id() == *sn::user_attributes::uaNoAutoDynamic),
    }
}

fn noautobound<R: Reason>(tp: &decl::Tparam<R, Ty<R>>) -> bool {
    tp.user_attributes
        .iter()
        .any(|ua| ua.name.id() == *sn::user_attributes::uaNoAutoBound)
}

fn implicit_sdt_for_class<R: Reason>(
    opts: &oxidized::typechecker_options::TypecheckerOptions,
    this_class: Option<&decl::ShallowClass<R>>,
) -> bool {
    opts.tco_everything_sdt && !noautodynamic(this_class)
}

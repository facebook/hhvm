// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::typing_subtype;
use crate::Env;
use bumpalo::collections::vec::Vec as BVec;
use typing_defs_rust::*;

pub fn union_list<'a>(env: &mut Env<'a>, tys: impl Iterator<Item = Ty<'a>> + 'a) -> Ty<'a> {
    // TODO(hrust) normalize_union
    let tys = union_list_2_by_2(env, tys);
    make_union(env, tys)
}

pub fn union_list_2_by_2<'a>(
    env: &mut Env<'a>,
    tys: impl Iterator<Item = Ty<'a>> + 'a,
) -> &'a [Ty<'a>] {
    tys.fold(bumpalo::vec![in env.bld().bumpalo()], |res_tys, ty| {
        union_ty_w_tyl(env, ty, res_tys)
    })
    .into_bump_slice()
}

pub fn union_ty_w_tyl<'a>(
    env: &mut Env<'a>,
    ty: Ty<'a>,
    mut tys: BVec<'a, Ty<'a>>,
) -> BVec<'a, Ty<'a>> {
    let mut res_tys = bumpalo::vec![in env.bld().bumpalo()];
    let union_ty = tys
        .drain(..)
        .fold(ty, |ty_acc, ty| match simplify_union(env, ty_acc, ty) {
            None => {
                res_tys.push(ty);
                ty_acc
            }
            Some(union_ty) => union_ty,
        });
    res_tys.push(union_ty);
    res_tys
}

pub fn make_union<'a>(env: &mut Env<'a>, tys: &'a [Ty<'a>]) -> Ty<'a> {
    let bld = env.bld();
    // TODO(hrust) treat null and nonnull properly, build nullable type if necessary...
    // TODO(hrust) build proper like type if necessary
    bld.union(bld.mk_rnone(), tys) // TODO(hrust) proper reason
}

pub fn simplify_union<'a>(env: &mut Env<'a>, ty1: Ty<'a>, ty2: Ty<'a>) -> Option<Ty<'a>> {
    // TODO(hrust) log
    // TODO(hrust) check ty_equal
    if typing_subtype::is_sub_type_for_union(env, ty1, ty2) {
        Some(ty2)
    } else if typing_subtype::is_sub_type_for_union(env, ty2, ty1) {
        Some(ty1)
    } else {
        // TODO(hrust) simplify_union_
        None
    }
}

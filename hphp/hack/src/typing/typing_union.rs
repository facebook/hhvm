// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::Env;
use bumpalo::collections::vec::Vec as BVec;
use typing_defs_rust::*;

pub fn union_list<'a>(env: &mut Env<'a>, tys: impl Iterator<Item = Ty<'a>> + 'a) -> Ty<'a> {
    // TODO(hrust) normalize_union
    make_union(env, tys)
}

pub fn make_union<'a>(env: &mut Env<'a>, tys: impl Iterator<Item = Ty<'a>>) -> Ty<'a> {
    let bld = env.bld();
    // TODO(hrust) treat null and nonnull properly, build nullable type if necessary...
    // TODO(hrust) build proper like type if necessary
    bld.union(bld.mk_rnone(), BVec::from_iter_in(tys, bld.alloc)) // TODO(hrust) proper reason
}

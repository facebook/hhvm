// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use oxidized_by_ref::typing_defs_core::Tparam;
use typing_collections_rust::SMap;
use typing_defs_rust::typing_defs_core::Ty;
use typing_defs_rust::typing_make_type::TypeBuilder;

pub fn make_locl<'a>(
    bld: &'a TypeBuilder<'a>,
    tparams: impl IntoIterator<Item = &'a Tparam<'a>>,
    mut targs: impl Iterator<Item = Ty<'a>>,
) -> SMap<'a, Ty<'a>> {
    // TODO(hrust)
    let mut substs = SMap::empty();
    for tparam in tparams {
        let targ_ty: Ty = match targs.next() {
            Some(ty) => ty,
            None => bld.any(bld.mk_rnone()),
        };
        substs = substs.add(bld, &tparam.name.1, targ_ty)
    }
    substs
}

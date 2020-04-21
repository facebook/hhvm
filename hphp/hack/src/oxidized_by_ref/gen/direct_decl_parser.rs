// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<6bf0ed90a1b913094f7a9e3a3f5faf17>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub struct Decls<'a> {
    pub classes: s_map::SMap<'a, shallow_decl_defs::ShallowClass<'a>>,
    pub funs: s_map::SMap<'a, typing_defs::FunElt<'a>>,
    pub typedefs: s_map::SMap<'a, typing_defs::TypedefType<'a>>,
    pub consts: s_map::SMap<'a, typing_defs::Ty<'a>>,
}

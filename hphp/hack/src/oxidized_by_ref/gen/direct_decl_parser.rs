// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<9cef101d2ab724481e628a61047d499d>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Decls<'a> {
    pub classes: s_map::SMap<'a, shallow_decl_defs::ShallowClass<'a>>,
    pub funs: s_map::SMap<'a, typing_defs::FunElt<'a>>,
    pub typedefs: s_map::SMap<'a, typing_defs::TypedefType<'a>>,
    pub consts: s_map::SMap<'a, typing_defs::Ty<'a>>,
}
impl<'a> TrivialDrop for Decls<'a> {}

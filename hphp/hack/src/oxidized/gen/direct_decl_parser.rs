// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<aef10f6b05c49cc5d35ba4fd1411373f>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Decls {
    pub classes: s_map::SMap<shallow_decl_defs::ShallowClass>,
    pub funs: s_map::SMap<typing_defs::FunElt>,
    pub typedefs: s_map::SMap<typing_defs::TypedefType>,
    pub consts: s_map::SMap<typing_defs::Ty>,
}

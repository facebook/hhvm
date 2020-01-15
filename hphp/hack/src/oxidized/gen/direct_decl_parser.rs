// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ec911a340051c6594d7843478e6d4514>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

use crate::s_map;
use crate::shallow_decl_defs;
use crate::typing_defs;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub struct Decls {
    pub classes: s_map::SMap<shallow_decl_defs::ShallowClass>,
    pub funs: s_map::SMap<typing_defs::FunElt>,
    pub typedefs: s_map::SMap<typing_defs::TypedefType>,
    pub consts: s_map::SMap<typing_defs::DeclTy>,
}

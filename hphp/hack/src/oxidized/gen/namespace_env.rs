// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<494af5f2ba32670f1d10d029d750b5c0>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::IntoOcamlRep;
use ocamlvalue_macro::Ocamlvalue;

use crate::s_map;

#[derive(Clone, Debug, IntoOcamlRep, Ocamlvalue)]
pub struct Env {
    pub ns_uses: s_map::SMap<String>,
    pub class_uses: s_map::SMap<String>,
    pub fun_uses: s_map::SMap<String>,
    pub const_uses: s_map::SMap<String>,
    pub name: Option<String>,
    pub auto_ns_map: Vec<(String, String)>,
    pub is_codegen: bool,
}

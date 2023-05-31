// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<7eaa338ed3e31bf3b77b0e82f09c5c03>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
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
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, hash, show, ord)")]
#[rust_to_ocaml(prefix = "ns_")]
#[repr(C)]
pub struct Env {
    #[rust_to_ocaml(attr = "opaque")]
    pub ns_uses: s_map::SMap<String>,
    #[rust_to_ocaml(attr = "opaque")]
    pub class_uses: s_map::SMap<String>,
    #[rust_to_ocaml(attr = "opaque")]
    pub fun_uses: s_map::SMap<String>,
    #[rust_to_ocaml(attr = "opaque")]
    pub const_uses: s_map::SMap<String>,
    pub name: Option<String>,
    pub is_codegen: bool,
    pub disable_xhp_element_mangling: bool,
}

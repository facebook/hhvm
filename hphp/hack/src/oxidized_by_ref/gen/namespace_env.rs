// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<2aedab974fa2111998cf2e1933b4305f>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Env<'a> {
    pub ns_uses: s_map::SMap<'a, &'a str>,
    pub class_uses: s_map::SMap<'a, &'a str>,
    pub record_def_uses: s_map::SMap<'a, &'a str>,
    pub fun_uses: s_map::SMap<'a, &'a str>,
    pub const_uses: s_map::SMap<'a, &'a str>,
    pub name: Option<&'a str>,
    pub auto_ns_map: &'a [(&'a str, &'a str)],
    pub is_codegen: bool,
    pub disable_xhp_element_mangling: bool,
}
impl<'a> TrivialDrop for Env<'a> {}

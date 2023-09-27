// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<be69b3dada70a1c0a64f47ed641266bb>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub type Hash = isize;

#[derive(
    Clone,
    Debug,
    Default,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving yojson_of")]
#[repr(C)]
pub struct ByNames {
    #[serde(default)]
    #[rust_to_ocaml(attr = "yojson_drop_if SMap.is_empty")]
    pub fun_tast_hashes: s_map::SMap<Hash>,
    #[serde(default)]
    #[rust_to_ocaml(attr = "yojson_drop_if SMap.is_empty")]
    pub class_tast_hashes: s_map::SMap<Hash>,
    #[serde(default)]
    #[rust_to_ocaml(attr = "yojson_drop_if SMap.is_empty")]
    pub typedef_tast_hashes: s_map::SMap<Hash>,
    #[serde(default)]
    #[rust_to_ocaml(attr = "yojson_drop_if SMap.is_empty")]
    pub gconst_tast_hashes: s_map::SMap<Hash>,
    #[serde(default)]
    #[rust_to_ocaml(attr = "yojson_drop_if SMap.is_empty")]
    pub module_tast_hashes: s_map::SMap<Hash>,
}

#[rust_to_ocaml(attr = "deriving yojson_of")]
pub type TastHashes = relative_path::map::Map<ByNames>;

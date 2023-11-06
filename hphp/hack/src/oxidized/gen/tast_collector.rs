// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<57b0e2014de5a91485c556361c3f264e>>
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

#[rust_to_ocaml(attr = "deriving yojson_of")]
pub type Tast = String;

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
    pub fun_tasts: s_map::SMap<Tast>,
    pub class_tasts: s_map::SMap<Tast>,
    pub typedef_tasts: s_map::SMap<Tast>,
    pub gconst_tasts: s_map::SMap<Tast>,
    pub module_tasts: s_map::SMap<Tast>,
}

#[rust_to_ocaml(attr = "deriving yojson_of")]
pub type TastCollector = relative_path::map::Map<ByNames>;

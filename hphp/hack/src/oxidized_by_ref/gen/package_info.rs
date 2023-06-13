// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<bbff39aecb12a1efd0015747e28cacda>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRepIn;
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
    EqModuloPos,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C)]
pub struct PackageInfo<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub glob_to_package: s_map::SMap<'a, &'a package::Package<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub existing_packages: s_map::SMap<'a, &'a package::Package<'a>>,
}
impl<'a> TrivialDrop for PackageInfo<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PackageInfo<'arena>);

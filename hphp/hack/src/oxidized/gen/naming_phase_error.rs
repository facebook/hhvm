// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<68d240440800358c01cc8696a6cc0f3d>>
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

#[derive(
    Clone,
    Debug,
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
#[rust_to_ocaml(attr = r#"ocaml.warning "-37""#)]
#[repr(C, u8)]
pub enum ExperimentalFeature {
    #[rust_to_ocaml(name = "Like_type")]
    LikeType(pos::Pos),
    Supportdyn(pos::Pos),
    #[rust_to_ocaml(attr = r#"warning "-37""#)]
    #[rust_to_ocaml(name = "Const_attr")]
    ConstAttr(pos::Pos),
    #[rust_to_ocaml(attr = r#"warning "-37""#)]
    #[rust_to_ocaml(name = "Const_static_prop")]
    ConstStaticProp(pos::Pos),
    #[rust_to_ocaml(attr = r#"warning "-37""#)]
    #[rust_to_ocaml(name = "IFC_infer_flows")]
    IFCInferFlows(pos::Pos),
}

#[derive(
    Clone,
    Debug,
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
#[repr(C, u8)]
pub enum NamingPhaseError {
    Naming(naming_error::NamingError),
    #[rust_to_ocaml(name = "Nast_check")]
    NastCheck(nast_check_error::NastCheckError),
    #[rust_to_ocaml(name = "Unexpected_hint")]
    UnexpectedHint(pos::Pos),
    #[rust_to_ocaml(name = "Malformed_access")]
    MalformedAccess(pos::Pos),
    #[rust_to_ocaml(name = "Experimental_feature")]
    ExperimentalFeature(ExperimentalFeature),
}

#[derive(
    Clone,
    Debug,
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
#[repr(C)]
pub struct Agg {
    pub naming: Vec<naming_error::NamingError>,
    pub nast_check: Vec<nast_check_error::NastCheckError>,
    pub unexpected_hints: Vec<pos::Pos>,
    pub malformed_accesses: Vec<pos::Pos>,
    pub experimental_features: Vec<ExperimentalFeature>,
}

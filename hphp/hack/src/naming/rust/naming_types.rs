// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated <<SignedSource::*O*zOeWoEQle#+L!plEphiEmie@IsG>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh
use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep,
)]
#[rust_to_ocaml(attr = "deriving (show, eq)")]
#[repr(u8)]
pub enum KindOfType {
    TClass,
    TTypedef,
}
impl TrivialDrop for KindOfType {}
arena_deserializer::impl_deserialize_in_arena!(KindOfType);

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
    ToOcamlRep,
)]
#[rust_to_ocaml(attr = "deriving (show, eq)")]
#[repr(C, u8)]
pub enum NameKind {
    #[rust_to_ocaml(name = "Type_kind")]
    TypeKind(KindOfType),
    #[rust_to_ocaml(name = "Fun_kind")]
    FunKind,
    #[rust_to_ocaml(name = "Const_kind")]
    ConstKind,
    #[rust_to_ocaml(name = "Module_kind")]
    ModuleKind,
}

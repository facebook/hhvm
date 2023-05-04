// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<95cc519ba21d63979a4c56e80c11f935>>
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
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(u8)]
pub enum SiKind {
    #[rust_to_ocaml(name = "SI_Class")]
    SIClass,
    #[rust_to_ocaml(name = "SI_Interface")]
    SIInterface,
    #[rust_to_ocaml(name = "SI_Enum")]
    SIEnum,
    #[rust_to_ocaml(name = "SI_Trait")]
    SITrait,
    #[rust_to_ocaml(name = "SI_Unknown")]
    SIUnknown,
    #[rust_to_ocaml(name = "SI_Mixed")]
    SIMixed,
    #[rust_to_ocaml(name = "SI_Function")]
    SIFunction,
    #[rust_to_ocaml(name = "SI_Typedef")]
    SITypedef,
    #[rust_to_ocaml(name = "SI_GlobalConstant")]
    SIGlobalConstant,
    #[rust_to_ocaml(name = "SI_XHP")]
    SIXHP,
    #[rust_to_ocaml(name = "SI_Namespace")]
    SINamespace,
    #[rust_to_ocaml(name = "SI_ClassMethod")]
    SIClassMethod,
    #[rust_to_ocaml(name = "SI_Literal")]
    SILiteral,
    #[rust_to_ocaml(name = "SI_ClassConstant")]
    SIClassConstant,
    #[rust_to_ocaml(name = "SI_Property")]
    SIProperty,
    #[rust_to_ocaml(name = "SI_LocalVariable")]
    SILocalVariable,
    #[rust_to_ocaml(name = "SI_Keyword")]
    SIKeyword,
    #[rust_to_ocaml(name = "SI_Constructor")]
    SIConstructor,
}
impl TrivialDrop for SiKind {}
arena_deserializer::impl_deserialize_in_arena!(SiKind);

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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "sia_")]
#[repr(C)]
pub struct SiAddendum {
    pub name: String,
    pub kind: SiKind,
    pub is_abstract: bool,
    pub is_final: bool,
}

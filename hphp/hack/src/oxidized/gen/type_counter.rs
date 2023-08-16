// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<eb237696696712d692f5127927565d15>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
pub use pos::Pos;
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
#[rust_to_ocaml(attr = "deriving (ord, yojson_of)")]
#[repr(C, u8)]
pub enum Entity {
    Class(String),
    Function(String),
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
#[rust_to_ocaml(attr = "deriving (ord, yojson_of)")]
#[repr(C)]
pub struct EntityPos(pub Pos, pub Entity);

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
#[rust_to_ocaml(attr = "deriving (ord, yojson_of)")]
#[repr(u8)]
pub enum LoggedType {
    Like,
    NonLike,
    Mixed,
    SupportdynOfMixed,
    Dynamic,
}
impl TrivialDrop for LoggedType {}
arena_deserializer::impl_deserialize_in_arena!(LoggedType);

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
#[rust_to_ocaml(attr = "deriving (ord, yojson_of)")]
#[repr(u8)]
pub enum Category {
    Expression,
    #[rust_to_ocaml(name = "Obj_get_receiver")]
    ObjGetReceiver,
    #[rust_to_ocaml(name = "Class_get_receiver")]
    ClassGetReceiver,
    #[rust_to_ocaml(name = "Class_const_receiver")]
    ClassConstReceiver,
    Property,
    Parameter,
    Return,
}
impl TrivialDrop for Category {}
arena_deserializer::impl_deserialize_in_arena!(Category);

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
#[rust_to_ocaml(attr = "deriving yojson_of")]
#[repr(C)]
pub struct Count {
    /// The position of the entity for which this count holds
    pub entity_pos: EntityPos,
    /// The type that this count is for
    pub counted_type: LoggedType,
    /// Program construct that produces this type
    pub category: Category,
    /// The actual count
    pub value: isize,
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
#[rust_to_ocaml(attr = "deriving yojson_of")]
#[rust_to_ocaml(prefix = "num_")]
#[repr(C)]
pub struct Summary {
    pub like_types: isize,
    pub non_like_types: isize,
}

#[rust_to_ocaml(attr = "deriving yojson_of")]
pub type TypeCounter = relative_path::map::Map<Summary>;

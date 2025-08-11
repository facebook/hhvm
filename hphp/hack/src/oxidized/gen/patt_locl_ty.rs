// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<51d08a1c187d7a0e0812649920173709>>
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

/// Defines pattern matches over a subset of Hack types
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
pub enum PattLoclTy {
    /// Matches type-constructor like types e.g. class, vec_or_dict and newtypes
    #[rust_to_ocaml(prefix = "patt_")]
    Apply {
        name: patt_name::PattName,
        params: Box<Params>,
    },
    /// Matches Hack primitives
    Prim(Prim),
    /// Matches Hack shapes
    Shape(ShapeFields),
    /// Matches Hack optional types
    Option(Box<PattLoclTy>),
    /// Matches Hack tuples
    Tuple(Vec<PattLoclTy>),
    /// Matches dynamic
    Dynamic,
    /// Matches non-null
    Nonnull,
    /// Matches any Hack type
    Any,
    /// Matches either the first pattern or, if that does not match, the second
    #[rust_to_ocaml(prefix = "patt_")]
    Or {
        fst: Box<PattLoclTy>,
        snd: Box<PattLoclTy>,
    },
    /// Match the provided pattern and bind the result to the supplied variable name
    As {
        lbl: patt_var::PattVar,
        patt: Box<PattLoclTy>,
    },
    /// Mark invalid patterns
    Invalid(Vec<validation_err::ValidationErr>, Box<PattLoclTy>),
}

/// Defines pattern matches over list of Hack types appearing as type parameters
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Params {
    /// Matches the empty parameter list
    Nil,
    /// Matches any paramter list
    Wildcard,
    /// Mathes a parameter list where the first element matches [patt_hd] and
    /// the remaining parameters match [patt_tl]
    #[rust_to_ocaml(prefix = "patt_")]
    Cons { hd: PattLoclTy, tl: Box<Params> },
    /// Matches a parameter list where at least one Hint matches the supplied
    /// pattern
    Exists(PattLoclTy),
}

/// Match Hack primitive types
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
#[rust_to_ocaml(and)]
#[repr(u8)]
pub enum Prim {
    Null,
    Void,
    Int,
    Bool,
    Float,
    String,
    Resource,
    Num,
    Arraykey,
    Noreturn,
}
impl TrivialDrop for Prim {}
arena_deserializer::impl_deserialize_in_arena!(Prim);

/// Matches the fields of a Hack shape
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum ShapeFields {
    /// Matches a shape which contains the a field matching the given [shape_field]
    /// and matches the remaining [shape_fields]
    #[rust_to_ocaml(prefix = "patt_")]
    Fld {
        fld: ShapeField,
        rest: Box<ShapeFields>,
    },
    /// Matches any shape which contains fields which have not already been matched
    Open,
    /// Matches a shape in which all other fields have already been matched
    Closed,
}

/// Matches an individual Hack shape field
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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct ShapeField {
    pub lbl: ShapeLabel,
    pub optional: bool,
    pub patt: Box<PattLoclTy>,
}

/// Matches a Hack shape label
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "deriving (compare, eq, sexp, show)")]
#[repr(C, u8)]
pub enum ShapeLabel {
    RegGroupLabel(String),
    StrLbl(String),
    CConstLbl { cls_nm: String, cnst_nm: String },
}

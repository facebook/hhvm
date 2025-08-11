// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<26a9ced57daaa41b58e367ca8c8f6fab>>
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

/// Defines pattern matches over a subset of Hack types
#[derive(
    Clone,
    Copy,
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
#[repr(C, u8)]
pub enum PattLoclTy<'a> {
    /// Matches type-constructor like types e.g. class, vec_or_dict and newtypes
    #[rust_to_ocaml(prefix = "patt_")]
    Apply {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        name: patt_name::PattName<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        params: &'a Params<'a>,
    },
    /// Matches Hack primitives
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Prim(&'a oxidized::patt_locl_ty::Prim),
    /// Matches Hack shapes
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Shape(&'a ShapeFields<'a>),
    /// Matches Hack optional types
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Option(&'a PattLoclTy<'a>),
    /// Matches Hack tuples
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Tuple(&'a [PattLoclTy<'a>]),
    /// Matches dynamic
    Dynamic,
    /// Matches non-null
    Nonnull,
    /// Matches any Hack type
    Any,
    /// Matches either the first pattern or, if that does not match, the second
    #[rust_to_ocaml(prefix = "patt_")]
    Or {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        fst: &'a PattLoclTy<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        snd: &'a PattLoclTy<'a>,
    },
    /// Match the provided pattern and bind the result to the supplied variable name
    As {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        lbl: &'a patt_var::PattVar<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        patt: &'a PattLoclTy<'a>,
    },
    /// Mark invalid patterns
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Invalid(&'a (&'a [validation_err::ValidationErr<'a>], PattLoclTy<'a>)),
}
impl<'a> TrivialDrop for PattLoclTy<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PattLoclTy<'arena>);

/// Defines pattern matches over list of Hack types appearing as type parameters
#[derive(
    Clone,
    Copy,
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Params<'a> {
    /// Matches the empty parameter list
    Nil,
    /// Matches any paramter list
    Wildcard,
    /// Mathes a parameter list where the first element matches [patt_hd] and
    /// the remaining parameters match [patt_tl]
    #[rust_to_ocaml(prefix = "patt_")]
    Cons {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        hd: PattLoclTy<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        tl: &'a Params<'a>,
    },
    /// Matches a parameter list where at least one Hint matches the supplied
    /// pattern
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Exists(&'a PattLoclTy<'a>),
}
impl<'a> TrivialDrop for Params<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Params<'arena>);

pub use oxidized::patt_locl_ty::Prim;

/// Matches the fields of a Hack shape
#[derive(
    Clone,
    Copy,
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum ShapeFields<'a> {
    /// Matches a shape which contains the a field matching the given [shape_field]
    /// and matches the remaining [shape_fields]
    #[rust_to_ocaml(prefix = "patt_")]
    Fld {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        fld: ShapeField<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        rest: &'a ShapeFields<'a>,
    },
    /// Matches any shape which contains fields which have not already been matched
    Open,
    /// Matches a shape in which all other fields have already been matched
    Closed,
}
impl<'a> TrivialDrop for ShapeFields<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShapeFields<'arena>);

/// Matches an individual Hack shape field
#[derive(
    Clone,
    Copy,
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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct ShapeField<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub lbl: ShapeLabel<'a>,
    pub optional: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub patt: &'a PattLoclTy<'a>,
}
impl<'a> TrivialDrop for ShapeField<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShapeField<'arena>);

/// Matches a Hack shape label
#[derive(
    Clone,
    Copy,
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "deriving (compare, eq, sexp, show)")]
#[repr(C, u8)]
pub enum ShapeLabel<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RegGroupLabel(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    StrLbl(&'a str),
    CConstLbl {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        cls_nm: &'a str,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        cnst_nm: &'a str,
    },
}
impl<'a> TrivialDrop for ShapeLabel<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShapeLabel<'arena>);

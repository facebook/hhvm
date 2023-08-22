// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<aa45e2fb159a23c74735657cd6d33618>>
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
    #[rust_to_ocaml(prefix = "patt_")]
    Apply {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        name: patt_name::PattName<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        params: &'a Params<'a>,
    },
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Prim(&'a oxidized::patt_locl_ty::Prim),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Shape(&'a ShapeFields<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Option(&'a PattLoclTy<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Tuple(&'a [PattLoclTy<'a>]),
    Dynamic,
    Nonnull,
    Any,
    #[rust_to_ocaml(prefix = "patt_")]
    Or {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        fst: &'a PattLoclTy<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        snd: &'a PattLoclTy<'a>,
    },
    As {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        lbl: &'a patt_var::PattVar<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        patt: &'a PattLoclTy<'a>,
    },
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Invalid(&'a (&'a [validation_err::ValidationErr<'a>], PattLoclTy<'a>)),
}
impl<'a> TrivialDrop for PattLoclTy<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PattLoclTy<'arena>);

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
    Nil,
    Wildcard,
    #[rust_to_ocaml(prefix = "patt_")]
    Cons {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        hd: PattLoclTy<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        tl: &'a Params<'a>,
    },
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Exists(&'a PattLoclTy<'a>),
}
impl<'a> TrivialDrop for Params<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Params<'arena>);

pub use oxidized::patt_locl_ty::Prim;

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
    #[rust_to_ocaml(prefix = "patt_")]
    Fld {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        fld: ShapeField<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        rest: &'a ShapeFields<'a>,
    },
    Open,
    Closed,
}
impl<'a> TrivialDrop for ShapeFields<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShapeFields<'arena>);

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
    StrLbl(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    IntLbl(&'a str),
    CConstLbl {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        cls_nm: &'a str,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        cnst_nm: &'a str,
    },
}
impl<'a> TrivialDrop for ShapeLabel<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShapeLabel<'arena>);

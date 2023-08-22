// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<430a20a7591995518bbbfcf5021baab3>>
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
    #[rust_to_ocaml(prefix = "patt_")]
    Apply {
        name: patt_name::PattName,
        params: Box<Params>,
    },
    Prim(Prim),
    Shape(ShapeFields),
    Option(Box<PattLoclTy>),
    Tuple(Vec<PattLoclTy>),
    Dynamic,
    Nonnull,
    Any,
    #[rust_to_ocaml(prefix = "patt_")]
    Or {
        fst: Box<PattLoclTy>,
        snd: Box<PattLoclTy>,
    },
    As {
        lbl: patt_var::PattVar,
        patt: Box<PattLoclTy>,
    },
    Invalid(Vec<validation_err::ValidationErr>, Box<PattLoclTy>),
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Params {
    Nil,
    Wildcard,
    #[rust_to_ocaml(prefix = "patt_")]
    Cons {
        hd: PattLoclTy,
        tl: Box<Params>,
    },
    Exists(PattLoclTy),
}

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
    #[rust_to_ocaml(prefix = "patt_")]
    Fld {
        fld: ShapeField,
        rest: Box<ShapeFields>,
    },
    Open,
    Closed,
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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct ShapeField {
    pub lbl: ShapeLabel,
    pub optional: bool,
    pub patt: Box<PattLoclTy>,
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "deriving (compare, eq, sexp, show)")]
#[repr(C, u8)]
pub enum ShapeLabel {
    StrLbl(String),
    IntLbl(String),
    CConstLbl { cls_nm: String, cnst_nm: String },
}

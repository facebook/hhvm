// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<15dfa89d78c888889d5bba44f40df30b>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

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
pub enum PattError<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Primary(&'a Primary<'a>),
    #[rust_to_ocaml(prefix = "patt_")]
    Apply {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        cb: &'a oxidized::patt_error::Callback,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        err: &'a PattError<'a>,
    },
    #[rust_to_ocaml(prefix = "patt_")]
    #[rust_to_ocaml(name = "Apply_reasons")]
    ApplyReasons {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        rsns_cb: &'a oxidized::patt_error::ReasonsCallback,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        secondary: &'a Secondary<'a>,
    },
    #[rust_to_ocaml(prefix = "patt_")]
    Or {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        fst: &'a PattError<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        snd: &'a PattError<'a>,
    },
    Invalid {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        errs: &'a [validation_err::ValidationErr<'a>],
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        patt: &'a PattError<'a>,
    },
}
impl<'a> TrivialDrop for PattError<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PattError<'arena>);

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
pub enum Primary<'a> {
    #[rust_to_ocaml(name = "Any_prim")]
    AnyPrim,
    /// Member not found pattern
    #[rust_to_ocaml(prefix = "patt_")]
    #[rust_to_ocaml(name = "Member_not_found")]
    MemberNotFound {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        is_static: Option<&'a oxidized::patt_error::StaticPattern>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        kind: &'a oxidized::patt_error::MemberKindPattern,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        class_name: patt_string::PattString<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        member_name: patt_string::PattString<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        visibility: Option<&'a oxidized::patt_error::VisibilityPattern>,
    },
}
impl<'a> TrivialDrop for Primary<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Primary<'arena>);

pub use oxidized::patt_error::MemberKindPattern;
pub use oxidized::patt_error::StaticPattern;
pub use oxidized::patt_error::VisibilityPattern;

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
pub enum Secondary<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Of_error")]
    OfError(&'a PattError<'a>),
    #[rust_to_ocaml(prefix = "patt_")]
    #[rust_to_ocaml(name = "Violated_constraint")]
    ViolatedConstraint {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        cstr: patt_string::PattString<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        ty_sub: patt_locl_ty::PattLoclTy<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        ty_sup: patt_locl_ty::PattLoclTy<'a>,
    },
    #[rust_to_ocaml(prefix = "patt_ty_")]
    #[rust_to_ocaml(name = "Subtyping_error")]
    SubtypingError {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        sub: patt_locl_ty::PattLoclTy<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        sup: patt_locl_ty::PattLoclTy<'a>,
    },
    #[rust_to_ocaml(name = "Any_snd")]
    AnySnd,
}
impl<'a> TrivialDrop for Secondary<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Secondary<'arena>);

pub use oxidized::patt_error::Callback;
pub use oxidized::patt_error::ReasonsCallback;

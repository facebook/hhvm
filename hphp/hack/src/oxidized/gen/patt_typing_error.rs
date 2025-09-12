// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<0d92673cf3de1b89e899d120a0e7f251>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

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
pub enum PattTypingError {
    Primary(Primary),
    #[rust_to_ocaml(prefix = "patt_")]
    Apply {
        cb: Callback,
        err: Box<PattTypingError>,
    },
    #[rust_to_ocaml(prefix = "patt_")]
    #[rust_to_ocaml(name = "Apply_reasons")]
    ApplyReasons {
        rsns_cb: ReasonsCallback,
        secondary: Box<Secondary>,
    },
    #[rust_to_ocaml(prefix = "patt_")]
    Or {
        fst: Box<PattTypingError>,
        snd: Box<PattTypingError>,
    },
    #[rust_to_ocaml(name = "Invalid_typing")]
    InvalidTyping {
        errs: Vec<validation_err::ValidationErr>,
        patt: Box<PattTypingError>,
    },
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
pub enum Primary {
    #[rust_to_ocaml(name = "Any_prim")]
    AnyPrim,
    #[rust_to_ocaml(prefix = "patt_")]
    #[rust_to_ocaml(name = "Member_not_found")]
    MemberNotFound {
        is_static: Option<StaticPattern>,
        kind: MemberKindPattern,
        class_name: patt_string::PattString,
        member_name: patt_string::PattString,
        visibility: Option<VisibilityPattern>,
    },
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
pub enum StaticPattern {
    #[rust_to_ocaml(name = "Static_only")]
    StaticOnly,
    #[rust_to_ocaml(name = "Instance_only")]
    InstanceOnly,
}
impl TrivialDrop for StaticPattern {}
arena_deserializer::impl_deserialize_in_arena!(StaticPattern);

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
pub enum MemberKindPattern {
    #[rust_to_ocaml(name = "Any_member_kind")]
    AnyMemberKind,
    #[rust_to_ocaml(name = "Method_only")]
    MethodOnly,
    #[rust_to_ocaml(name = "Property_only")]
    PropertyOnly,
    #[rust_to_ocaml(name = "Class_constant_only")]
    ClassConstantOnly,
    #[rust_to_ocaml(name = "Class_typeconst_only")]
    ClassTypeconstOnly,
}
impl TrivialDrop for MemberKindPattern {}
arena_deserializer::impl_deserialize_in_arena!(MemberKindPattern);

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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(u8)]
pub enum VisibilityPattern {
    #[rust_to_ocaml(name = "Any_visibility")]
    AnyVisibility,
    #[rust_to_ocaml(name = "Public_only")]
    PublicOnly,
    #[rust_to_ocaml(name = "Private_only")]
    PrivateOnly,
    #[rust_to_ocaml(name = "Protected_only")]
    ProtectedOnly,
    #[rust_to_ocaml(name = "Internal_only")]
    InternalOnly,
}
impl TrivialDrop for VisibilityPattern {}
arena_deserializer::impl_deserialize_in_arena!(VisibilityPattern);

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
pub enum Secondary {
    #[rust_to_ocaml(name = "Of_error")]
    OfError(PattTypingError),
    #[rust_to_ocaml(prefix = "patt_")]
    #[rust_to_ocaml(name = "Violated_constraint")]
    ViolatedConstraint {
        cstr: patt_string::PattString,
        ty_sub: patt_locl_ty::PattLoclTy,
        ty_sup: patt_locl_ty::PattLoclTy,
    },
    #[rust_to_ocaml(prefix = "patt_ty_")]
    #[rust_to_ocaml(name = "Subtyping_error")]
    SubtypingError {
        sub: patt_locl_ty::PattLoclTy,
        sup: patt_locl_ty::PattLoclTy,
    },
    #[rust_to_ocaml(name = "Any_snd")]
    AnySnd,
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
pub enum Callback {
    #[rust_to_ocaml(name = "Any_callback")]
    AnyCallback,
}
impl TrivialDrop for Callback {}
arena_deserializer::impl_deserialize_in_arena!(Callback);

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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(u8)]
pub enum ReasonsCallback {
    #[rust_to_ocaml(name = "Any_reasons_callback")]
    AnyReasonsCallback,
}
impl TrivialDrop for ReasonsCallback {}
arena_deserializer::impl_deserialize_in_arena!(ReasonsCallback);

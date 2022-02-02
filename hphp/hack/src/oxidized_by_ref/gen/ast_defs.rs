// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<7514d3f805bfd11d2606b66d279ee74a>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::shape_map;

pub use pos::Pos;

pub type Id_<'a> = str;

#[derive(
    Clone,
    Copy,
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
#[repr(C)]
pub struct Id<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Id_<'a>,
);
impl<'a> TrivialDrop for Id<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Id<'arena>);

pub type Pstring<'a> = (&'a Pos<'a>, &'a str);

pub type ByteString<'a> = str;

pub type PositionedByteString<'a> = (&'a Pos<'a>, &'a bstr::BStr);

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
#[repr(C)]
pub enum ShapeFieldName<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    SFlitInt(&'a Pstring<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    SFlitStr(&'a PositionedByteString<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    SFclassConst(&'a (Id<'a>, &'a Pstring<'a>)),
}
impl<'a> TrivialDrop for ShapeFieldName<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShapeFieldName<'arena>);

pub use oxidized::ast_defs::Variance;

pub use oxidized::ast_defs::ConstraintKind;

pub use oxidized::ast_defs::Reified;

pub use oxidized::ast_defs::Abstraction;

pub use oxidized::ast_defs::ClassishKind;

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
#[repr(C)]
pub enum ParamKind<'a> {
    /// Contains the position for an entire `inout` annotated expression, e.g.:
    ///
    ///   foo(inout $bar);
    ///       ^^^^^^^^^^
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Pinout(&'a Pos<'a>),
    Pnormal,
}
impl<'a> TrivialDrop for ParamKind<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ParamKind<'arena>);

pub use oxidized::ast_defs::ReadonlyKind;

pub use oxidized::ast_defs::OgNullFlavor;

pub use oxidized::ast_defs::PropOrMethod;

pub use oxidized::ast_defs::FunKind;

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
#[repr(C)]
pub enum Bop<'a> {
    Plus,
    Minus,
    Star,
    Slash,
    Eqeq,
    Eqeqeq,
    Starstar,
    Diff,
    Diff2,
    Ampamp,
    Barbar,
    Lt,
    Lte,
    Gt,
    Gte,
    Dot,
    Amp,
    Bar,
    Ltlt,
    Gtgt,
    Percent,
    Xor,
    Cmp,
    QuestionQuestion,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Eq(Option<&'a Bop<'a>>),
}
impl<'a> TrivialDrop for Bop<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Bop<'arena>);

pub use oxidized::ast_defs::Uop;

pub use oxidized::ast_defs::Visibility;

/// Literal values that can occur in XHP enum properties.
///
/// class :my-xhp-class {
///   attribute enum {'big', 'small'} my-prop;
/// }
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
#[repr(C)]
pub enum XhpEnumValue<'a> {
    XEVInt(isize),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    XEVString(&'a str),
}
impl<'a> TrivialDrop for XhpEnumValue<'a> {}
arena_deserializer::impl_deserialize_in_arena!(XhpEnumValue<'arena>);

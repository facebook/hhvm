// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ff95698ba96a73e1f3ceb1b637774e01>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
pub use pos::Pos;
use serde::Deserialize;
use serde::Serialize;

pub use crate::shape_map;
#[allow(unused_imports)]
use crate::*;

#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct Id<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Id_<'a>,
);
impl<'a> TrivialDrop for Id<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Id<'arena>);

#[rust_to_ocaml(and)]
pub type Pstring<'a> = (&'a Pos<'a>, &'a str);

#[rust_to_ocaml(and)]
pub type ByteString<'a> = str;

#[rust_to_ocaml(and)]
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(C, u8)]
pub enum ShapeFieldName<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "SFlit_int")]
    SFlitInt(&'a Pstring<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "SFlit_str")]
    SFlitStr(&'a PositionedByteString<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "SFclass_const")]
    #[rust_to_ocaml(inline_tuple)]
    SFclassConst(&'a (Id<'a>, &'a Pstring<'a>)),
}
impl<'a> TrivialDrop for ShapeFieldName<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShapeFieldName<'arena>);

pub use oxidized::ast_defs::Abstraction;
pub use oxidized::ast_defs::ClassishKind;
pub use oxidized::ast_defs::ConstraintKind;
pub use oxidized::ast_defs::Variance;

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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(C, u8)]
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

pub use oxidized::ast_defs::FunKind;
pub use oxidized::ast_defs::OgNullFlavor;
pub use oxidized::ast_defs::OptionalKind;
pub use oxidized::ast_defs::PropOrMethod;
pub use oxidized::ast_defs::ReadonlyKind;

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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(C, u8)]
pub enum Bop<'a> {
    /// Addition: x + y
    Plus,
    /// Subtraction: x - y
    Minus,
    /// Multiplication: x * y
    Star,
    /// Division: x / y
    Slash,
    /// Value/coercing equality: x == y
    Eqeq,
    /// Same-type-and-value equality: x === y
    Eqeqeq,
    /// Exponent: x ** y
    Starstar,
    /// Value inquality: x != y
    Diff,
    /// Not-same-type-and-value-equality: x !== y
    Diff2,
    /// Logical AND: x && y
    Ampamp,
    /// Logical OR: x || y
    Barbar,
    /// Less than: x < y
    Lt,
    /// Less than or equal to: x <= y
    Lte,
    /// Greater than: x > y
    Gt,
    /// Greater than or equal to: x >= y
    Gte,
    /// String concatenation: x . y
    Dot,
    /// Bitwise AND: x & y
    Amp,
    /// Bitwise OR: x | y
    Bar,
    /// Bitwise left shift: x << y
    Ltlt,
    /// Bitwise right shift: x >> y
    Gtgt,
    /// Modulo: x % y
    Percent,
    /// Bitwise XOR: x ^ y
    Xor,
    /// Spaceship operator: x <=> y
    Cmp,
    /// Coalesce: x ?? y
    QuestionQuestion,
    /// =, +=, -=, ...
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(C, u8)]
pub enum XhpEnumValue<'a> {
    #[rust_to_ocaml(name = "XEV_Int")]
    XEVInt(isize),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "XEV_String")]
    XEVString(&'a str),
}
impl<'a> TrivialDrop for XhpEnumValue<'a> {}
arena_deserializer::impl_deserialize_in_arena!(XhpEnumValue<'arena>);

pub use oxidized::ast_defs::ReifyKind;
pub use oxidized::ast_defs::Tprim;
pub use oxidized::ast_defs::TypedefVisibility;

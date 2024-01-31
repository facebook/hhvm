// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<2a1b77b1c5d4d8f2f953e1d2b768d67d>>
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

pub use crate::shape_map;
#[allow(unused_imports)]
use crate::*;

#[rust_to_ocaml(and)]
pub type Id_ = String;

#[derive(
    Clone,
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
pub struct Id(pub Pos, pub Id_);

#[rust_to_ocaml(and)]
pub type Pstring = (Pos, String);

#[rust_to_ocaml(and)]
pub type ByteString = String;

#[rust_to_ocaml(and)]
pub type PositionedByteString = (Pos, bstr::BString);

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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(C, u8)]
pub enum ShapeFieldName {
    #[rust_to_ocaml(name = "SFlit_int")]
    SFlitInt(Pstring),
    #[rust_to_ocaml(name = "SFlit_str")]
    SFlitStr(PositionedByteString),
    #[rust_to_ocaml(name = "SFclass_const")]
    SFclassConst(Id, Pstring),
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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum Variance {
    Covariant,
    Contravariant,
    Invariant,
}
impl TrivialDrop for Variance {}
arena_deserializer::impl_deserialize_in_arena!(Variance);

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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum ConstraintKind {
    #[rust_to_ocaml(name = "Constraint_as")]
    ConstraintAs,
    #[rust_to_ocaml(name = "Constraint_eq")]
    ConstraintEq,
    #[rust_to_ocaml(name = "Constraint_super")]
    ConstraintSuper,
}
impl TrivialDrop for ConstraintKind {}
arena_deserializer::impl_deserialize_in_arena!(ConstraintKind);

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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum Abstraction {
    Concrete,
    Abstract,
}
impl TrivialDrop for Abstraction {}
arena_deserializer::impl_deserialize_in_arena!(Abstraction);

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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(C, u8)]
pub enum ClassishKind {
    /// Kind for `class` and `abstract class`
    Cclass(Abstraction),
    /// Kind for `interface`
    Cinterface,
    /// Kind for `trait`
    Ctrait,
    /// Kind for `enum`
    Cenum,
    /// Kind for `enum class` and `abstract enum class`.
    /// See https://docs.hhvm.com/hack/built-in-types/enum-class
    #[rust_to_ocaml(name = "Cenum_class")]
    CenumClass(Abstraction),
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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(C, u8)]
pub enum ParamKind {
    /// Contains the position for an entire `inout` annotated expression, e.g.:
    ///
    ///   foo(inout $bar);
    ///       ^^^^^^^^^^
    Pinout(Pos),
    Pnormal,
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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum ReadonlyKind {
    Readonly,
}
impl TrivialDrop for ReadonlyKind {}
arena_deserializer::impl_deserialize_in_arena!(ReadonlyKind);

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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum OptionalKind {
    Optional,
}
impl TrivialDrop for OptionalKind {}
arena_deserializer::impl_deserialize_in_arena!(OptionalKind);

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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum OgNullFlavor {
    #[rust_to_ocaml(name = "OG_nullthrows")]
    OGNullthrows,
    #[rust_to_ocaml(name = "OG_nullsafe")]
    OGNullsafe,
}
impl TrivialDrop for OgNullFlavor {}
arena_deserializer::impl_deserialize_in_arena!(OgNullFlavor);

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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum PropOrMethod {
    #[rust_to_ocaml(name = "Is_prop")]
    IsProp,
    #[rust_to_ocaml(name = "Is_method")]
    IsMethod,
}
impl TrivialDrop for PropOrMethod {}
arena_deserializer::impl_deserialize_in_arena!(PropOrMethod);

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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum FunKind {
    FSync,
    FAsync,
    FGenerator,
    FAsyncGenerator,
}
impl TrivialDrop for FunKind {}
arena_deserializer::impl_deserialize_in_arena!(FunKind);

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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(C, u8)]
pub enum Bop {
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
    Eq(Option<Box<Bop>>),
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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum Uop {
    /// Bitwise negation: ~x
    Utild,
    /// Logical not: !b
    Unot,
    /// Unary plus: +x
    Uplus,
    /// Unary minus: -x
    Uminus,
    /// Unary increment: ++i
    Uincr,
    /// Unary decrement: --i
    Udecr,
    /// Unary postfix increment: i++
    Upincr,
    /// Unary postfix decrement: i--
    Updecr,
    /// Error control/Silence (ignore) expections: @e
    Usilence,
}
impl TrivialDrop for Uop {}
arena_deserializer::impl_deserialize_in_arena!(Uop);

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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum Visibility {
    #[rust_to_ocaml(attr = r#"visitors.name "visibility_Private""#)]
    Private,
    #[rust_to_ocaml(attr = r#"visitors.name "visibility_Public""#)]
    Public,
    #[rust_to_ocaml(attr = r#"visitors.name "visibility_Protected""#)]
    Protected,
    #[rust_to_ocaml(attr = r#"visitors.name "visibility_Internal""#)]
    Internal,
}
impl TrivialDrop for Visibility {}
arena_deserializer::impl_deserialize_in_arena!(Visibility);

/// Literal values that can occur in XHP enum properties.
///
/// class :my-xhp-class {
///   attribute enum {'big', 'small'} my-prop;
/// }
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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(C, u8)]
pub enum XhpEnumValue {
    #[rust_to_ocaml(name = "XEV_Int")]
    XEVInt(isize),
    #[rust_to_ocaml(name = "XEV_String")]
    XEVString(String),
}

/// Hack's primitive types (as the typechecker understands them).
///
/// Used in the AST of typehints (Aast_defs.Hprim) and in the representation of
/// types (Typing_defs.Tprim).
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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum Tprim {
    Tnull,
    Tvoid,
    Tint,
    Tbool,
    Tfloat,
    Tstring,
    Tresource,
    Tnum,
    Tarraykey,
    Tnoreturn,
}
impl TrivialDrop for Tprim {}
arena_deserializer::impl_deserialize_in_arena!(Tprim);

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
#[rust_to_ocaml(attr = "transform.opaque")]
#[repr(u8)]
pub enum TypedefVisibility {
    Transparent,
    Opaque,
    OpaqueModule,
    CaseType,
}
impl TrivialDrop for TypedefVisibility {}
arena_deserializer::impl_deserialize_in_arena!(TypedefVisibility);

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
#[rust_to_ocaml(attr = "transform.opaque")]
#[rust_to_ocaml(attr = r#"deriving ((show { with_path = false }), eq, hash, ord,
    (transform ~restart:(`Disallow `Encode_as_result)), yojson_of,
    (visitors
       {
         variety = "iter";
         nude = true;
         visit_prefix = "on_";
         ancestors = ["Visitors_runtime.iter_base"]
       }),
    (visitors
       {
         variety = "endo";
         nude = true;
         visit_prefix = "on_";
         ancestors = ["Visitors_runtime.endo_base"]
       }),
    (visitors
       {
         variety = "reduce";
         nude = true;
         visit_prefix = "on_";
         ancestors = ["Visitors_runtime.reduce_base"]
       }),
    (visitors
       {
         variety = "map";
         nude = true;
         visit_prefix = "on_";
         ancestors = ["Visitors_runtime.map_base"]
       }))"#)]
#[repr(u8)]
pub enum ReifyKind {
    Erased,
    SoftReified,
    Reified,
}
impl TrivialDrop for ReifyKind {}
arena_deserializer::impl_deserialize_in_arena!(ReifyKind);

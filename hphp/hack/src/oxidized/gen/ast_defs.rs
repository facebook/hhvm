// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<bd3ee5f9d50be7cafce640d591eb7a2a>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::shape_map;

pub use pos::Pos;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Id(pub Pos, pub String);

pub type Pstring = (Pos, String);

pub type ByteString = String;

pub type PositionedByteString = (Pos, bstr::BString);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum ShapeFieldName {
    SFlitInt(Pstring),
    SFlitStr(PositionedByteString),
    SFclassConst(Id, Pstring),
}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
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
pub enum Variance {
    Covariant,
    Contravariant,
    Invariant,
}
impl TrivialDrop for Variance {}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
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
pub enum ConstraintKind {
    ConstraintAs,
    ConstraintEq,
    ConstraintSuper,
}
impl TrivialDrop for ConstraintKind {}

pub type Reified = bool;

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
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
pub enum ClassKind {
    Cabstract,
    Cnormal,
    Cinterface,
    Ctrait,
    Cenum,
}
impl TrivialDrop for ClassKind {}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
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
pub enum ParamKind {
    Pinout,
}
impl TrivialDrop for ParamKind {}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
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
pub enum OgNullFlavor {
    OGNullthrows,
    OGNullsafe,
}
impl TrivialDrop for OgNullFlavor {}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
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
pub enum FunKind {
    FSync,
    FAsync,
    FGenerator,
    FAsyncGenerator,
    FCoroutine,
}
impl TrivialDrop for FunKind {}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum Bop {
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
    LogXor,
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
    Eq(Option<Box<Bop>>),
}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
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
pub enum Uop {
    Utild,
    Unot,
    Uplus,
    Uminus,
    Uincr,
    Udecr,
    Upincr,
    Updecr,
    Usilence,
}
impl TrivialDrop for Uop {}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
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
pub enum Visibility {
    Private,
    Public,
    Protected,
}
impl TrivialDrop for Visibility {}

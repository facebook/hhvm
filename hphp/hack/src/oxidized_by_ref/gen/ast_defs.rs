// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<6c90ac99438860f2b72ebce610347981>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::shape_map;

pub use pos::Pos;

pub type Id_<'a> = str;

#[derive(
    Clone,
    Copy,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Id<'a>(pub &'a Pos<'a>, pub &'a Id_<'a>);
impl<'a> TrivialDrop for Id<'a> {}

pub type Pstring<'a> = (&'a Pos<'a>, &'a str);

pub type ByteString<'a> = str;

pub type PositionedByteString<'a> = (&'a Pos<'a>, &'a bstr::BStr);

#[derive(
    Clone,
    Copy,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum ShapeFieldName<'a> {
    SFlitInt(&'a Pstring<'a>),
    SFlitStr(&'a PositionedByteString<'a>),
    SFclassConst(&'a (Id<'a>, &'a Pstring<'a>)),
}
impl<'a> TrivialDrop for ShapeFieldName<'a> {}

pub use oxidized::ast_defs::Variance;

pub use oxidized::ast_defs::ConstraintKind;

pub use oxidized::ast_defs::Reified;

pub use oxidized::ast_defs::ClassKind;

pub use oxidized::ast_defs::ParamKind;

pub use oxidized::ast_defs::ReadonlyKind;

pub use oxidized::ast_defs::OgNullFlavor;

pub use oxidized::ast_defs::FunKind;

#[derive(
    Clone,
    Copy,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
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
    Eq(Option<&'a Bop<'a>>),
}
impl<'a> TrivialDrop for Bop<'a> {}

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
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum XhpEnumValue<'a> {
    XEVInt(isize),
    XEVString(&'a str),
}
impl<'a> TrivialDrop for XhpEnumValue<'a> {}

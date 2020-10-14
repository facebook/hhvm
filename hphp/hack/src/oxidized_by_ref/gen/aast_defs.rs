// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<955cbe59363c1ab8fdb1b9f731aec3d3>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::ast_defs::shape_map;

pub use ast_defs::OgNullFlavor;
pub use ast_defs::Pos;
pub use ast_defs::PositionedByteString;
pub use ast_defs::Pstring;
pub use local_id::LocalId;
pub use shape_map::ShapeMap;

pub use oxidized::aast_defs::Visibility;

#[derive(
    Clone,
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
pub struct Lid<'a>(pub &'a Pos<'a>, pub &'a LocalId<'a>);
impl<'a> TrivialDrop for Lid<'a> {}

pub type Sid<'a> = ast_defs::Id<'a>;

pub use oxidized::aast_defs::IsReified;

pub use oxidized::aast_defs::FuncReactive;

pub use oxidized::aast_defs::ParamMutability;

pub use oxidized::aast_defs::ImportFlavor;

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
pub enum XhpChild<'a> {
    ChildName(&'a Sid<'a>),
    ChildList(&'a [XhpChild<'a>]),
    ChildUnary(&'a (XhpChild<'a>, oxidized::aast_defs::XhpChildOp)),
    ChildBinary(&'a (XhpChild<'a>, XhpChild<'a>)),
}
impl<'a> TrivialDrop for XhpChild<'a> {}

pub use oxidized::aast_defs::XhpChildOp;

#[derive(
    Clone,
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
pub struct Hint<'a>(pub &'a Pos<'a>, pub &'a Hint_<'a>);
impl<'a> TrivialDrop for Hint<'a> {}

pub use oxidized::aast_defs::MutableReturn;

pub type VariadicHint<'a> = Option<&'a Hint<'a>>;

#[derive(
    Clone,
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
pub struct HintFun<'a> {
    pub reactive_kind: oxidized::aast_defs::FuncReactive,
    pub param_tys: &'a [&'a Hint<'a>],
    pub param_kinds: &'a [Option<oxidized::ast_defs::ParamKind>],
    pub param_mutability: &'a [Option<oxidized::aast_defs::ParamMutability>],
    pub variadic_ty: &'a VariadicHint<'a>,
    pub cap: Option<&'a Hint<'a>>,
    pub return_ty: &'a Hint<'a>,
    pub is_mutable_return: &'a oxidized::aast_defs::MutableReturn,
}
impl<'a> TrivialDrop for HintFun<'a> {}

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
pub enum Hint_<'a> {
    Hoption(&'a Hint<'a>),
    Hlike(&'a Hint<'a>),
    Hfun(&'a HintFun<'a>),
    Htuple(&'a [&'a Hint<'a>]),
    Happly(&'a (Sid<'a>, &'a [&'a Hint<'a>])),
    Hshape(&'a NastShapeInfo<'a>),
    /// This represents the use of a type const. Type consts are accessed like
    /// regular consts in Hack, i.e.
    ///
    /// [self | static | Class]::TypeConst
    ///
    /// Class  => Happly "Class"
    /// self   => Happly of the class of definition
    /// static => Habstr ("static",
    ///           Habstr ("this", (Constraint_as, Happly of class of definition)))
    /// Type const access can be chained such as
    ///
    /// Class::TC1::TC2::TC3
    ///
    /// We resolve the root of the type access chain as a type as follows.
    ///
    /// This will result in the following representation
    ///
    /// Haccess (Happly "Class", ["TC1", "TC2", "TC3"])
    Haccess(&'a (&'a Hint<'a>, &'a [Sid<'a>])),
    Hsoft(&'a Hint<'a>),
    Hany,
    Herr,
    Hmixed,
    Hnonnull,
    Habstr(&'a (&'a str, &'a [&'a Hint<'a>])),
    Hdarray(&'a (&'a Hint<'a>, &'a Hint<'a>)),
    Hvarray(&'a Hint<'a>),
    HvarrayOrDarray(&'a (Option<&'a Hint<'a>>, &'a Hint<'a>)),
    Hprim(&'a Tprim<'a>),
    Hthis,
    Hdynamic,
    Hnothing,
    HpuAccess(&'a (&'a Hint<'a>, Sid<'a>)),
    Hunion(&'a [&'a Hint<'a>]),
    Hintersection(&'a [&'a Hint<'a>]),
}
impl<'a> TrivialDrop for Hint_<'a> {}

/// AST types such as Happly("int", []) are resolved to Hprim values
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
pub enum Tprim<'a> {
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
    /// plain Pocket Universe atom when we don't know which enum it is in.
    /// E.g. `:@MyAtom`
    Tatom(&'a str),
}
impl<'a> TrivialDrop for Tprim<'a> {}

#[derive(
    Clone,
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
pub struct ShapeFieldInfo<'a> {
    pub optional: bool,
    pub hint: &'a Hint<'a>,
    pub name: ast_defs::ShapeFieldName<'a>,
}
impl<'a> TrivialDrop for ShapeFieldInfo<'a> {}

#[derive(
    Clone,
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
pub struct NastShapeInfo<'a> {
    pub allows_unknown_fields: bool,
    pub field_map: &'a [&'a ShapeFieldInfo<'a>],
}
impl<'a> TrivialDrop for NastShapeInfo<'a> {}

pub use oxidized::aast_defs::KvcKind;

pub use oxidized::aast_defs::VcKind;

pub use oxidized::aast_defs::UseAsVisibility;

pub use oxidized::aast_defs::TypedefVisibility;

#[derive(
    Clone,
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
pub struct Enum_<'a> {
    pub base: &'a Hint<'a>,
    pub constraint: Option<&'a Hint<'a>>,
    pub includes: &'a [&'a Hint<'a>],
    pub enum_class: bool,
}
impl<'a> TrivialDrop for Enum_<'a> {}

#[derive(
    Clone,
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
pub struct WhereConstraintHint<'a>(
    pub &'a Hint<'a>,
    pub oxidized::ast_defs::ConstraintKind,
    pub &'a Hint<'a>,
);
impl<'a> TrivialDrop for WhereConstraintHint<'a> {}

pub use oxidized::aast_defs::ReifyKind;

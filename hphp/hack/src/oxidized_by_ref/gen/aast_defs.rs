// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<3644cc5243988d0cb72759b51c06fb49>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::ast_defs::shape_map;

pub use ast_defs::OgNullFlavor;
pub use ast_defs::Pos;
pub use ast_defs::Pstring;
pub use local_id::LocalId;
pub use shape_map::ShapeMap;

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Lid<'a>(pub &'a Pos<'a>, pub LocalId<'a>);
impl<'a> TrivialDrop for Lid<'a> {}

pub type Sid<'a> = ast_defs::Id<'a>;

pub use oxidized::aast_defs::IsReified;

pub use oxidized::aast_defs::CallType;

pub use oxidized::aast_defs::IsCoroutine;

pub use oxidized::aast_defs::FuncReactive;

pub use oxidized::aast_defs::ParamMutability;

pub use oxidized::aast_defs::ImportFlavor;

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum XhpChild<'a> {
    ChildName(Sid<'a>),
    ChildList(&'a [XhpChild<'a>]),
    ChildUnary(&'a XhpChild<'a>, oxidized::aast_defs::XhpChildOp),
    ChildBinary(&'a XhpChild<'a>, &'a XhpChild<'a>),
}
impl<'a> TrivialDrop for XhpChild<'a> {}

pub use oxidized::aast_defs::XhpChildOp;

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Hint<'a>(pub &'a Pos<'a>, pub &'a Hint_<'a>);
impl<'a> TrivialDrop for Hint<'a> {}

pub use oxidized::aast_defs::MutableReturn;

pub type VariadicHint<'a> = Option<Hint<'a>>;

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct HintFun<'a> {
    pub reactive_kind: oxidized::aast_defs::FuncReactive,
    pub is_coroutine: oxidized::aast_defs::IsCoroutine,
    pub param_tys: &'a [Hint<'a>],
    pub param_kinds: &'a [Option<oxidized::ast_defs::ParamKind>],
    pub param_mutability: &'a [Option<oxidized::aast_defs::ParamMutability>],
    pub variadic_ty: VariadicHint<'a>,
    pub return_ty: Hint<'a>,
    pub is_mutable_return: oxidized::aast_defs::MutableReturn,
}
impl<'a> TrivialDrop for HintFun<'a> {}

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum Hint_<'a> {
    Hoption(Hint<'a>),
    Hlike(Hint<'a>),
    Hfun(HintFun<'a>),
    Htuple(&'a [Hint<'a>]),
    Happly(Sid<'a>, &'a [Hint<'a>]),
    Hshape(NastShapeInfo<'a>),
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
    Haccess(Hint<'a>, &'a [Sid<'a>]),
    Hsoft(Hint<'a>),
    Hany,
    Herr,
    Hmixed,
    Hnonnull,
    Habstr(&'a str, &'a [Hint<'a>]),
    Harray(Option<Hint<'a>>, Option<Hint<'a>>),
    Hdarray(Hint<'a>, Hint<'a>),
    Hvarray(Hint<'a>),
    HvarrayOrDarray(Option<Hint<'a>>, Hint<'a>),
    Hprim(Tprim<'a>),
    Hthis,
    Hdynamic,
    Hnothing,
    HpuAccess(Hint<'a>, Sid<'a>),
    Hunion(&'a [Hint<'a>]),
    Hintersection(&'a [Hint<'a>]),
}
impl<'a> TrivialDrop for Hint_<'a> {}

/// AST types such as Happly("int", []) are resolved to Hprim values
#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
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
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct ShapeFieldInfo<'a> {
    pub optional: bool,
    pub hint: Hint<'a>,
    pub name: ast_defs::ShapeFieldName<'a>,
}
impl<'a> TrivialDrop for ShapeFieldInfo<'a> {}

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct NastShapeInfo<'a> {
    pub allows_unknown_fields: bool,
    pub field_map: &'a [ShapeFieldInfo<'a>],
}
impl<'a> TrivialDrop for NastShapeInfo<'a> {}

pub use oxidized::aast_defs::KvcKind;

pub use oxidized::aast_defs::VcKind;

pub use oxidized::aast_defs::Visibility;

pub use oxidized::aast_defs::UseAsVisibility;

pub use oxidized::aast_defs::TypedefVisibility;

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Enum_<'a> {
    pub base: Hint<'a>,
    pub constraint: Option<Hint<'a>>,
}
impl<'a> TrivialDrop for Enum_<'a> {}

#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct WhereConstraint<'a>(
    pub Hint<'a>,
    pub oxidized::ast_defs::ConstraintKind,
    pub Hint<'a>,
);
impl<'a> TrivialDrop for WhereConstraint<'a> {}

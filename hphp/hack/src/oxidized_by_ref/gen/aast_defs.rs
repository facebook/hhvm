// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<0fd93c85f2de4655fec24e41d1a4c781>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
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
pub struct Lid<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a LocalId<'a>,
);
impl<'a> TrivialDrop for Lid<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Lid<'arena>);

pub type Sid<'a> = ast_defs::Id<'a>;

pub use oxidized::aast_defs::IsReified;

pub use oxidized::aast_defs::ImportFlavor;

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
pub enum XhpChild<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ChildName(&'a Sid<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ChildList(&'a [XhpChild<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ChildUnary(&'a (XhpChild<'a>, oxidized::aast_defs::XhpChildOp)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ChildBinary(&'a (XhpChild<'a>, XhpChild<'a>)),
}
impl<'a> TrivialDrop for XhpChild<'a> {}
arena_deserializer::impl_deserialize_in_arena!(XhpChild<'arena>);

pub use oxidized::aast_defs::XhpChildOp;

#[derive(
    Clone,
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
pub struct Hint<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Hint_<'a>,
);
impl<'a> TrivialDrop for Hint<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Hint<'arena>);

pub type VariadicHint<'a> = Option<&'a Hint<'a>>;

#[derive(
    Clone,
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
pub struct Contexts<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a [&'a Hint<'a>],
);
impl<'a> TrivialDrop for Contexts<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Contexts<'arena>);

#[derive(
    Clone,
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
pub struct HfParamInfo {
    pub kind: Option<oxidized::ast_defs::ParamKind>,
    pub readonlyness: Option<oxidized::ast_defs::ReadonlyKind>,
}
impl TrivialDrop for HfParamInfo {}
arena_deserializer::impl_deserialize_in_arena!(HfParamInfo);

#[derive(
    Clone,
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
pub struct HintFun<'a> {
    pub is_readonly: Option<oxidized::ast_defs::ReadonlyKind>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub param_tys: &'a [&'a Hint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub param_info: &'a [Option<&'a HfParamInfo>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub variadic_ty: &'a VariadicHint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ctxs: Option<&'a Contexts<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub return_ty: &'a Hint<'a>,
    pub is_readonly_return: Option<oxidized::ast_defs::ReadonlyKind>,
}
impl<'a> TrivialDrop for HintFun<'a> {}
arena_deserializer::impl_deserialize_in_arena!(HintFun<'arena>);

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
pub enum Hint_<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hoption(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hlike(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hfun(&'a HintFun<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Htuple(&'a [&'a Hint<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Happly(&'a (Sid<'a>, &'a [&'a Hint<'a>])),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hshape(&'a NastShapeInfo<'a>),
    /// This represents the use of a type const. Type consts are accessed like
    /// regular consts in Hack, i.e.
    ///
    /// [$x | self | static | Class]::TypeConst
    ///
    /// Class  => Happly "Class"
    /// self   => Happly of the class of definition
    /// static => Habstr ("static",
    ///           Habstr ("this", (Constraint_as, Happly of class of definition)))
    /// $x     => Hvar "$x"
    ///
    /// Type const access can be chained such as
    ///
    /// Class::TC1::TC2::TC3
    ///
    /// We resolve the root of the type access chain as a type as follows.
    ///
    /// This will result in the following representation
    ///
    /// Haccess (Happly "Class", ["TC1", "TC2", "TC3"])
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Haccess(&'a (&'a Hint<'a>, &'a [Sid<'a>])),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hsoft(&'a Hint<'a>),
    Hany,
    Herr,
    Hmixed,
    Hnonnull,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Habstr(&'a (&'a str, &'a [&'a Hint<'a>])),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hdarray(&'a (&'a Hint<'a>, &'a Hint<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hvarray(&'a Hint<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    HvarrayOrDarray(&'a (Option<&'a Hint<'a>>, &'a Hint<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    HvecOrDict(&'a (Option<&'a Hint<'a>>, &'a Hint<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hprim(&'a Tprim),
    Hthis,
    Hdynamic,
    Hnothing,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hunion(&'a [&'a Hint<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hintersection(&'a [&'a Hint<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    HfunContext(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hvar(&'a str),
}
impl<'a> TrivialDrop for Hint_<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Hint_<'arena>);

/// AST types such as Happly("int", []) are resolved to Hprim values
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
pub struct ShapeFieldInfo<'a> {
    pub optional: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub hint: &'a Hint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: ast_defs::ShapeFieldName<'a>,
}
impl<'a> TrivialDrop for ShapeFieldInfo<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShapeFieldInfo<'arena>);

#[derive(
    Clone,
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
pub struct NastShapeInfo<'a> {
    pub allows_unknown_fields: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub field_map: &'a [&'a ShapeFieldInfo<'a>],
}
impl<'a> TrivialDrop for NastShapeInfo<'a> {}
arena_deserializer::impl_deserialize_in_arena!(NastShapeInfo<'arena>);

pub use oxidized::aast_defs::KvcKind;

pub use oxidized::aast_defs::VcKind;

pub use oxidized::aast_defs::UseAsVisibility;

pub use oxidized::aast_defs::TypedefVisibility;

#[derive(
    Clone,
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
pub struct Enum_<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub base: &'a Hint<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub constraint: Option<&'a Hint<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub includes: &'a [&'a Hint<'a>],
    pub enum_class: bool,
}
impl<'a> TrivialDrop for Enum_<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Enum_<'arena>);

#[derive(
    Clone,
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
pub struct WhereConstraintHint<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Hint<'a>,
    pub oxidized::ast_defs::ConstraintKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Hint<'a>,
);
impl<'a> TrivialDrop for WhereConstraintHint<'a> {}
arena_deserializer::impl_deserialize_in_arena!(WhereConstraintHint<'arena>);

pub use oxidized::aast_defs::ReifyKind;

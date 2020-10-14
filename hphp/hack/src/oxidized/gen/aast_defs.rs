// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<5397b8d95654b0da0c9a3369fd05b240>>
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

pub use crate::ast_defs::shape_map;

pub use ast_defs::OgNullFlavor;
pub use ast_defs::Pos;
pub use ast_defs::PositionedByteString;
pub use ast_defs::Pstring;
pub use ast_defs::Visibility;
pub use local_id::LocalId;
pub use shape_map::ShapeMap;

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
pub struct Lid(pub Pos, pub LocalId);

pub type Sid = ast_defs::Id;

pub type IsReified = bool;

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
pub enum FuncReactive {
    FPure,
    FReactive,
    FLocal,
    FShallow,
    FNonreactive,
}
impl TrivialDrop for FuncReactive {}

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
pub enum ParamMutability {
    PMutable,
    POwnedMutable,
    PMaybeMutable,
}
impl TrivialDrop for ParamMutability {}

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
pub enum ImportFlavor {
    Include,
    Require,
    IncludeOnce,
    RequireOnce,
}
impl TrivialDrop for ImportFlavor {}

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
pub enum XhpChild {
    ChildName(Sid),
    ChildList(Vec<XhpChild>),
    ChildUnary(Box<XhpChild>, XhpChildOp),
    ChildBinary(Box<XhpChild>, Box<XhpChild>),
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
pub enum XhpChildOp {
    ChildStar,
    ChildPlus,
    ChildQuestion,
}
impl TrivialDrop for XhpChildOp {}

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
pub struct Hint(pub Pos, pub Box<Hint_>);

pub type MutableReturn = bool;

pub type VariadicHint = Option<Hint>;

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
pub struct HintFun {
    pub reactive_kind: FuncReactive,
    pub param_tys: Vec<Hint>,
    pub param_kinds: Vec<Option<ast_defs::ParamKind>>,
    pub param_mutability: Vec<Option<ParamMutability>>,
    pub variadic_ty: VariadicHint,
    pub cap: Option<Hint>,
    pub return_ty: Hint,
    pub is_mutable_return: MutableReturn,
}

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
pub enum Hint_ {
    Hoption(Hint),
    Hlike(Hint),
    Hfun(HintFun),
    Htuple(Vec<Hint>),
    Happly(Sid, Vec<Hint>),
    Hshape(NastShapeInfo),
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
    Haccess(Hint, Vec<Sid>),
    Hsoft(Hint),
    Hany,
    Herr,
    Hmixed,
    Hnonnull,
    Habstr(String, Vec<Hint>),
    Hdarray(Hint, Hint),
    Hvarray(Hint),
    HvarrayOrDarray(Option<Hint>, Hint),
    Hprim(Tprim),
    Hthis,
    Hdynamic,
    Hnothing,
    HpuAccess(Hint, Sid),
    Hunion(Vec<Hint>),
    Hintersection(Vec<Hint>),
}

/// AST types such as Happly("int", []) are resolved to Hprim values
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
    /// plain Pocket Universe atom when we don't know which enum it is in.
    /// E.g. `:@MyAtom`
    Tatom(String),
}

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
pub struct ShapeFieldInfo {
    pub optional: bool,
    pub hint: Hint,
    pub name: ast_defs::ShapeFieldName,
}

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
pub struct NastShapeInfo {
    pub allows_unknown_fields: bool,
    pub field_map: Vec<ShapeFieldInfo>,
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
pub enum KvcKind {
    Map,
    ImmMap,
    Dict,
}
impl TrivialDrop for KvcKind {}

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
pub enum VcKind {
    Vector,
    ImmVector,
    Vec,
    Set,
    ImmSet,
    Pair_,
    Keyset,
}
impl TrivialDrop for VcKind {}

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
pub enum UseAsVisibility {
    UseAsPublic,
    UseAsPrivate,
    UseAsProtected,
    UseAsFinal,
}
impl TrivialDrop for UseAsVisibility {}

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
pub enum TypedefVisibility {
    Transparent,
    Opaque,
}
impl TrivialDrop for TypedefVisibility {}

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
pub struct Enum_ {
    pub base: Hint,
    pub constraint: Option<Hint>,
    pub includes: Vec<Hint>,
    pub enum_class: bool,
}

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
pub struct WhereConstraintHint(pub Hint, pub ast_defs::ConstraintKind, pub Hint);

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
pub enum ReifyKind {
    Erased,
    SoftReified,
    Reified,
}
impl TrivialDrop for ReifyKind {}

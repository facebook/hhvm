// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<0e26c26a8b2d5618b695ebcfef44c4f0>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_rc/regen.sh

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

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Lid(pub std::rc::Rc<Pos>, pub LocalId);

pub type Sid = ast_defs::Id;

pub use oxidized::aast_defs::IsReified;

pub use oxidized::aast_defs::CallType;

pub use oxidized::aast_defs::FuncReactive;

pub use oxidized::aast_defs::ParamMutability;

pub use oxidized::aast_defs::ImportFlavor;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum XhpChild {
    ChildName(Sid),
    ChildList(Vec<XhpChild>),
    ChildUnary(std::rc::Rc<XhpChild>, oxidized::aast_defs::XhpChildOp),
    ChildBinary(std::rc::Rc<XhpChild>, std::rc::Rc<XhpChild>),
}

pub use oxidized::aast_defs::XhpChildOp;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Hint(pub std::rc::Rc<Pos>, pub std::rc::Rc<Hint_>);

pub use oxidized::aast_defs::MutableReturn;

pub type VariadicHint = Option<Hint>;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct HintFun {
    pub reactive_kind: oxidized::aast_defs::FuncReactive,
    pub param_tys: Vec<Hint>,
    pub param_kinds: Vec<Option<oxidized::ast_defs::ParamKind>>,
    pub param_mutability: Vec<Option<oxidized::aast_defs::ParamMutability>>,
    pub variadic_ty: VariadicHint,
    pub return_ty: Hint,
    pub is_mutable_return: oxidized::aast_defs::MutableReturn,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
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
    Habstr(std::rc::Rc<String>, Vec<Hint>),
    Harray(Option<Hint>, Option<Hint>),
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
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
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
    Tatom(std::rc::Rc<String>),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ShapeFieldInfo {
    pub optional: bool,
    pub hint: Hint,
    pub name: ast_defs::ShapeFieldName,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct NastShapeInfo {
    pub allows_unknown_fields: bool,
    pub field_map: Vec<ShapeFieldInfo>,
}

pub use oxidized::aast_defs::KvcKind;

pub use oxidized::aast_defs::VcKind;

pub use oxidized::aast_defs::Visibility;

pub use oxidized::aast_defs::UseAsVisibility;

pub use oxidized::aast_defs::TypedefVisibility;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Enum_ {
    pub base: Hint,
    pub constraint: Option<Hint>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct WhereConstraint(pub Hint, pub oxidized::ast_defs::ConstraintKind, pub Hint);

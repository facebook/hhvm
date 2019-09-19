// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<799befa728f17b6d95b38f72b995d659>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use ocamlvalue_macro::Ocamlvalue;

use crate::ast_defs;
use crate::local_id;

pub use crate::ast_defs::shape_map;

pub use ast_defs::OgNullFlavor;
pub use ast_defs::Pos;
pub use ast_defs::Pstring;
pub use local_id::LocalId;
pub use shape_map::ShapeMap;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct Lid(pub Pos, pub LocalId);

pub type Sid = ast_defs::Id;

pub type IsTerminal = bool;

pub type IsReified = bool;

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum CallType {
    Cnormal,
    CuserFunc,
}

pub type IsCoroutine = bool;

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum FuncReactive {
    FReactive,
    FLocal,
    FShallow,
    FNonreactive,
}

pub type Targ = Hint;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub enum CollectionTarg {
    CollectionTV(Targ),
    CollectionTKV(Targ, Targ),
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum ParamMutability {
    PMutable,
    POwnedMutable,
    PMaybeMutable,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum ImportFlavor {
    Include,
    Require,
    IncludeOnce,
    RequireOnce,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub enum XhpChild {
    ChildName(Sid),
    ChildList(Vec<XhpChild>),
    ChildUnary(Box<XhpChild>, XhpChildOp),
    ChildBinary(Box<XhpChild>, Box<XhpChild>),
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum XhpChildOp {
    ChildStar,
    ChildPlus,
    ChildQuestion,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct Hint(pub Pos, pub Box<Hint_>);

pub type MutableReturn = bool;

pub type VariadicHint = Option<Hint>;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub enum Hint_ {
    Hoption(Hint),
    Hlike(Hint),
    Hfun(
        FuncReactive,
        IsCoroutine,
        Vec<Hint>,
        Vec<Option<ast_defs::ParamKind>>,
        Vec<Option<ParamMutability>>,
        VariadicHint,
        Hint,
        MutableReturn,
    ),
    Htuple(Vec<Hint>),
    Happly(Sid, Vec<Hint>),
    Hshape(NastShapeInfo),
    Haccess(Hint, Vec<Sid>),
    Hsoft(Hint),
    Hany,
    Herr,
    Hmixed,
    Hnonnull,
    Habstr(String),
    Harray(Option<Hint>, Option<Hint>),
    Hdarray(Hint, Hint),
    Hvarray(Hint),
    HvarrayOrDarray(Hint),
    Hprim(Tprim),
    Hthis,
    Hdynamic,
    Hnothing,
    HpuAccess(Hint, Sid),
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
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
    Tatom(String),
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct ShapeFieldInfo {
    pub optional: bool,
    pub hint: Hint,
    pub name: ast_defs::ShapeFieldName,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct NastShapeInfo {
    pub allows_unknown_fields: bool,
    pub field_map: Vec<ShapeFieldInfo>,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum KvcKind {
    Map,
    ImmMap,
    Dict,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum VcKind {
    Vector,
    ImmVector,
    Vec,
    Set,
    ImmSet,
    Pair_,
    Keyset,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum Visibility {
    Private,
    Public,
    Protected,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum UseAsVisibility {
    UseAsPublic,
    UseAsPrivate,
    UseAsProtected,
    UseAsFinal,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, Ocamlvalue, PartialEq)]
pub enum TypedefVisibility {
    Transparent,
    Opaque,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct Enum_ {
    pub base: Hint,
    pub constraint: Option<Hint>,
}

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct WhereConstraint(pub Hint, pub ast_defs::ConstraintKind, pub Hint);

pub type Id = Lid;

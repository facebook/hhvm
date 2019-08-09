// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<63b2199a8c44bef182139f93fef73168>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use crate::ast_defs;
use crate::local_id;

pub use crate::ast_defs::shape_map;

pub use ast_defs::OgNullFlavor;
pub use ast_defs::Pos;
pub use ast_defs::Pstring;
pub use local_id::LocalId;
pub use shape_map::ShapeMap;

#[derive(Clone, Debug)]
pub struct Lid(pub Pos, pub LocalId);

pub type Sid = ast_defs::Id;

pub type IsTerminal = bool;

pub type IsReified = bool;

#[derive(Clone, Debug)]
pub enum CallType {
    Cnormal,
    CuserFunc,
}

pub type IsCoroutine = bool;

#[derive(Clone, Debug)]
pub enum FuncReactive {
    FReactive,
    FLocal,
    FShallow,
    FNonreactive,
}

pub type Targ = Hint;

#[derive(Clone, Debug)]
pub enum CollectionTarg {
    CollectionTV(Targ),
    CollectionTKV(Targ, Targ),
}

#[derive(Clone, Debug)]
pub enum ParamMutability {
    PMutable,
    POwnedMutable,
    PMaybeMutable,
}

#[derive(Clone, Debug)]
pub enum ImportFlavor {
    Include,
    Require,
    IncludeOnce,
    RequireOnce,
}

#[derive(Clone, Debug)]
pub enum XhpChild {
    ChildName(Sid),
    ChildList(Vec<XhpChild>),
    ChildUnary(Box<XhpChild>, XhpChildOp),
    ChildBinary(Box<XhpChild>, Box<XhpChild>),
}

#[derive(Clone, Debug)]
pub enum XhpChildOp {
    ChildStar,
    ChildPlus,
    ChildQuestion,
}

#[derive(Clone, Debug)]
pub struct Hint(pub Pos, pub Box<Hint_>);

pub type MutableReturn = bool;

pub type VariadicHint = Option<Hint>;

#[derive(Clone, Debug)]
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
}

#[derive(Clone, Debug)]
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

#[derive(Clone, Debug)]
pub struct ShapeFieldInfo {
    pub optional: bool,
    pub hint: Hint,
    pub name: ast_defs::ShapeFieldName,
}

#[derive(Clone, Debug)]
pub struct NastShapeInfo {
    pub allows_unknown_fields: bool,
    pub field_map: Vec<ShapeFieldInfo>,
}

#[derive(Clone, Debug)]
pub enum KvcKind {
    Map,
    ImmMap,
    Dict,
}

#[derive(Clone, Debug)]
pub enum VcKind {
    Vector,
    ImmVector,
    Vec,
    Set,
    ImmSet,
    Pair,
    Keyset,
}

#[derive(Clone, Debug)]
pub enum Visibility {
    Private,
    Public,
    Protected,
}

#[derive(Clone, Debug)]
pub enum UseAsVisibility {
    UseAsPublic,
    UseAsPrivate,
    UseAsProtected,
    UseAsFinal,
}

#[derive(Clone, Debug)]
pub enum TypedefVisibility {
    Transparent,
    Opaque,
}

#[derive(Clone, Debug)]
pub struct Enum_ {
    pub base: Hint,
    pub constraint: Option<Hint>,
}

#[derive(Clone, Debug)]
pub struct WhereConstraint(pub Hint, pub ast_defs::ConstraintKind, pub Hint);

pub type Id = Lid;

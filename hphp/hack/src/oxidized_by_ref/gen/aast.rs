// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<7cfaf3a38d82c49f41cd1840dcff5620>>
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

pub use aast_defs::*;
pub use doc_comment::DocComment;

/// Aast.program represents the top-level definitions in a Hack program.
/// ex: Expression annotation type (when typechecking, the inferred dtype)
/// fb: Function body tag (e.g. has naming occurred)
/// en: Environment (tracking state inside functions and classes)
/// hi: Hint annotation (when typechecking it will be the localized type hint or the
/// inferred missing type if the hint is missing)
pub type Program<'a, Ex, Fb, En, Hi> = [Def<'a, Ex, Fb, En, Hi>];

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
pub struct Stmt<'a, Ex, Fb, En, Hi>(pub &'a Pos<'a>, pub Stmt_<'a, Ex, Fb, En, Hi>);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Stmt<'a, Ex, Fb, En, Hi>
{
}

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
pub enum Stmt_<'a, Ex, Fb, En, Hi> {
    Fallthrough,
    Expr(&'a Expr<'a, Ex, Fb, En, Hi>),
    Break,
    Continue,
    Throw(&'a Expr<'a, Ex, Fb, En, Hi>),
    Return(Option<&'a Expr<'a, Ex, Fb, En, Hi>>),
    GotoLabel(&'a Pstring<'a>),
    Goto(&'a Pstring<'a>),
    Awaitall(
        &'a (
            &'a [(Option<&'a Lid<'a>>, &'a Expr<'a, Ex, Fb, En, Hi>)],
            &'a Block<'a, Ex, Fb, En, Hi>,
        ),
    ),
    If(
        &'a (
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Block<'a, Ex, Fb, En, Hi>,
            &'a Block<'a, Ex, Fb, En, Hi>,
        ),
    ),
    Do(&'a (&'a Block<'a, Ex, Fb, En, Hi>, &'a Expr<'a, Ex, Fb, En, Hi>)),
    While(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Block<'a, Ex, Fb, En, Hi>)),
    Using(&'a UsingStmt<'a, Ex, Fb, En, Hi>),
    For(
        &'a (
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Block<'a, Ex, Fb, En, Hi>,
        ),
    ),
    Switch(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a [Case<'a, Ex, Fb, En, Hi>])),
    Foreach(
        &'a (
            &'a Expr<'a, Ex, Fb, En, Hi>,
            AsExpr<'a, Ex, Fb, En, Hi>,
            &'a Block<'a, Ex, Fb, En, Hi>,
        ),
    ),
    Try(
        &'a (
            &'a Block<'a, Ex, Fb, En, Hi>,
            &'a [&'a Catch<'a, Ex, Fb, En, Hi>],
            &'a Block<'a, Ex, Fb, En, Hi>,
        ),
    ),
    Noop,
    Block(&'a Block<'a, Ex, Fb, En, Hi>),
    Markup(&'a Pstring<'a>),
    AssertEnv(&'a (oxidized::aast::EnvAnnot, &'a LocalIdMap<'a, Ex>)),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Stmt_<'a, Ex, Fb, En, Hi>
{
}

pub use oxidized::aast::EnvAnnot;

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
pub struct UsingStmt<'a, Ex, Fb, En, Hi> {
    pub is_block_scoped: bool,
    pub has_await: bool,
    pub expr: &'a Expr<'a, Ex, Fb, En, Hi>,
    pub block: &'a Block<'a, Ex, Fb, En, Hi>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for UsingStmt<'a, Ex, Fb, En, Hi>
{
}

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
pub enum AsExpr<'a, Ex, Fb, En, Hi> {
    AsV(&'a Expr<'a, Ex, Fb, En, Hi>),
    AsKv(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Expr<'a, Ex, Fb, En, Hi>)),
    AwaitAsV(&'a (&'a Pos<'a>, &'a Expr<'a, Ex, Fb, En, Hi>)),
    AwaitAsKv(
        &'a (
            &'a Pos<'a>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
        ),
    ),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for AsExpr<'a, Ex, Fb, En, Hi>
{
}

pub type Block<'a, Ex, Fb, En, Hi> = [&'a Stmt<'a, Ex, Fb, En, Hi>];

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
pub struct ClassId<'a, Ex, Fb, En, Hi>(pub Ex, pub ClassId_<'a, Ex, Fb, En, Hi>);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassId<'a, Ex, Fb, En, Hi>
{
}

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
pub enum ClassId_<'a, Ex, Fb, En, Hi> {
    CIparent,
    CIself,
    CIstatic,
    CIexpr(&'a Expr<'a, Ex, Fb, En, Hi>),
    CI(&'a Sid<'a>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassId_<'a, Ex, Fb, En, Hi>
{
}

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
pub struct Expr<'a, Ex, Fb, En, Hi>(pub Ex, pub Expr_<'a, Ex, Fb, En, Hi>);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Expr<'a, Ex, Fb, En, Hi>
{
}

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
pub enum CollectionTarg<'a, Hi> {
    CollectionTV(&'a Targ<'a, Hi>),
    CollectionTKV(&'a (&'a Targ<'a, Hi>, &'a Targ<'a, Hi>)),
}
impl<'a, Hi: TrivialDrop> TrivialDrop for CollectionTarg<'a, Hi> {}

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
pub enum FunctionPtrId<'a, Ex, Fb, En, Hi> {
    FPId(&'a Sid<'a>),
    FPClassConst(&'a (&'a ClassId<'a, Ex, Fb, En, Hi>, &'a Pstring<'a>)),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FunctionPtrId<'a, Ex, Fb, En, Hi>
{
}

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
pub enum Expr_<'a, Ex, Fb, En, Hi> {
    Darray(
        &'a (
            Option<&'a (&'a Targ<'a, Hi>, &'a Targ<'a, Hi>)>,
            &'a [(&'a Expr<'a, Ex, Fb, En, Hi>, &'a Expr<'a, Ex, Fb, En, Hi>)],
        ),
    ),
    Varray(&'a (Option<&'a Targ<'a, Hi>>, &'a [&'a Expr<'a, Ex, Fb, En, Hi>])),
    Shape(&'a [(ast_defs::ShapeFieldName<'a>, &'a Expr<'a, Ex, Fb, En, Hi>)]),
    /// TODO: T38184446 Consolidate collections in AAST
    ValCollection(
        &'a (
            oxidized::aast::VcKind,
            Option<&'a Targ<'a, Hi>>,
            &'a [&'a Expr<'a, Ex, Fb, En, Hi>],
        ),
    ),
    /// TODO: T38184446 Consolidate collections in AAST
    KeyValCollection(
        &'a (
            oxidized::aast::KvcKind,
            Option<&'a (&'a Targ<'a, Hi>, &'a Targ<'a, Hi>)>,
            &'a [&'a Field<'a, Ex, Fb, En, Hi>],
        ),
    ),
    Null,
    This,
    True,
    False,
    Omitted,
    Id(&'a Sid<'a>),
    Lvar(&'a Lid<'a>),
    Dollardollar(&'a Lid<'a>),
    Clone(&'a Expr<'a, Ex, Fb, En, Hi>),
    ObjGet(
        &'a (
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
            oxidized::aast::OgNullFlavor,
        ),
    ),
    ArrayGet(
        &'a (
            &'a Expr<'a, Ex, Fb, En, Hi>,
            Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
        ),
    ),
    ClassGet(
        &'a (
            &'a ClassId<'a, Ex, Fb, En, Hi>,
            ClassGetExpr<'a, Ex, Fb, En, Hi>,
        ),
    ),
    ClassConst(&'a (&'a ClassId<'a, Ex, Fb, En, Hi>, &'a Pstring<'a>)),
    Call(
        &'a (
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a [&'a Targ<'a, Hi>],
            &'a [&'a Expr<'a, Ex, Fb, En, Hi>],
            Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
        ),
    ),
    FunctionPointer(&'a (FunctionPtrId<'a, Ex, Fb, En, Hi>, &'a [&'a Targ<'a, Hi>])),
    Int(&'a str),
    Float(&'a str),
    String(&'a bstr::BStr),
    String2(&'a [&'a Expr<'a, Ex, Fb, En, Hi>]),
    PrefixedString(&'a (&'a str, &'a Expr<'a, Ex, Fb, En, Hi>)),
    Yield(&'a Afield<'a, Ex, Fb, En, Hi>),
    YieldBreak,
    Await(&'a Expr<'a, Ex, Fb, En, Hi>),
    Suspend(&'a Expr<'a, Ex, Fb, En, Hi>),
    List(&'a [&'a Expr<'a, Ex, Fb, En, Hi>]),
    ExprList(&'a [&'a Expr<'a, Ex, Fb, En, Hi>]),
    Cast(&'a (&'a Hint<'a>, &'a Expr<'a, Ex, Fb, En, Hi>)),
    Unop(&'a (oxidized::ast_defs::Uop, &'a Expr<'a, Ex, Fb, En, Hi>)),
    Binop(
        &'a (
            ast_defs::Bop<'a>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
        ),
    ),
    /// The lid is the ID of the $$ that is implicitly declared by this pipe.
    Pipe(
        &'a (
            &'a Lid<'a>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
        ),
    ),
    Eif(
        &'a (
            &'a Expr<'a, Ex, Fb, En, Hi>,
            Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
        ),
    ),
    Is(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Hint<'a>)),
    As(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Hint<'a>, bool)),
    New(
        &'a (
            &'a ClassId<'a, Ex, Fb, En, Hi>,
            &'a [&'a Targ<'a, Hi>],
            &'a [&'a Expr<'a, Ex, Fb, En, Hi>],
            Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
            Ex,
        ),
    ),
    Record(
        &'a (
            Sid<'a>,
            &'a [(&'a Expr<'a, Ex, Fb, En, Hi>, &'a Expr<'a, Ex, Fb, En, Hi>)],
        ),
    ),
    Efun(&'a (&'a Fun_<'a, Ex, Fb, En, Hi>, &'a [&'a Lid<'a>])),
    Lfun(&'a (&'a Fun_<'a, Ex, Fb, En, Hi>, &'a [&'a Lid<'a>])),
    Xml(
        &'a (
            Sid<'a>,
            &'a [XhpAttribute<'a, Ex, Fb, En, Hi>],
            &'a [&'a Expr<'a, Ex, Fb, En, Hi>],
        ),
    ),
    Callconv(&'a (oxidized::ast_defs::ParamKind, &'a Expr<'a, Ex, Fb, En, Hi>)),
    Import(&'a (oxidized::aast::ImportFlavor, &'a Expr<'a, Ex, Fb, En, Hi>)),
    /// TODO: T38184446 Consolidate collections in AAST
    Collection(
        &'a (
            Sid<'a>,
            Option<CollectionTarg<'a, Hi>>,
            &'a [Afield<'a, Ex, Fb, En, Hi>],
        ),
    ),
    BracedExpr(&'a Expr<'a, Ex, Fb, En, Hi>),
    ParenthesizedExpr(&'a Expr<'a, Ex, Fb, En, Hi>),
    ExpressionTree(
        &'a (
            &'a Hint<'a>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
            Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
        ),
    ),
    Lplaceholder(&'a Pos<'a>),
    FunId(&'a Sid<'a>),
    MethodId(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Pstring<'a>)),
    /// meth_caller('Class name', 'method name')
    MethodCaller(&'a (Sid<'a>, &'a Pstring<'a>)),
    SmethodId(&'a (&'a ClassId<'a, Ex, Fb, En, Hi>, &'a Pstring<'a>)),
    Pair(
        &'a (
            Option<&'a (&'a Targ<'a, Hi>, &'a Targ<'a, Hi>)>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
            &'a Expr<'a, Ex, Fb, En, Hi>,
        ),
    ),
    Assert(&'a AssertExpr<'a, Ex, Fb, En, Hi>),
    PUAtom(&'a str),
    PUIdentifier(
        &'a (
            &'a ClassId<'a, Ex, Fb, En, Hi>,
            &'a Pstring<'a>,
            &'a Pstring<'a>,
        ),
    ),
    ETSplice(&'a Expr<'a, Ex, Fb, En, Hi>),
    Any,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Expr_<'a, Ex, Fb, En, Hi>
{
}

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
pub enum ClassGetExpr<'a, Ex, Fb, En, Hi> {
    CGstring(&'a Pstring<'a>),
    CGexpr(&'a Expr<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassGetExpr<'a, Ex, Fb, En, Hi>
{
}

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
pub enum AssertExpr<'a, Ex, Fb, En, Hi> {
    AEAssert(&'a Expr<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for AssertExpr<'a, Ex, Fb, En, Hi>
{
}

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
pub enum Case<'a, Ex, Fb, En, Hi> {
    Default(&'a (&'a Pos<'a>, &'a Block<'a, Ex, Fb, En, Hi>)),
    Case(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Block<'a, Ex, Fb, En, Hi>)),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Case<'a, Ex, Fb, En, Hi>
{
}

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
pub struct Catch<'a, Ex, Fb, En, Hi>(
    pub Sid<'a>,
    pub &'a Lid<'a>,
    pub &'a Block<'a, Ex, Fb, En, Hi>,
);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Catch<'a, Ex, Fb, En, Hi>
{
}

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
pub struct Field<'a, Ex, Fb, En, Hi>(
    pub &'a Expr<'a, Ex, Fb, En, Hi>,
    pub &'a Expr<'a, Ex, Fb, En, Hi>,
);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Field<'a, Ex, Fb, En, Hi>
{
}

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
pub enum Afield<'a, Ex, Fb, En, Hi> {
    AFvalue(&'a Expr<'a, Ex, Fb, En, Hi>),
    AFkvalue(&'a (&'a Expr<'a, Ex, Fb, En, Hi>, &'a Expr<'a, Ex, Fb, En, Hi>)),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Afield<'a, Ex, Fb, En, Hi>
{
}

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
pub enum XhpAttribute<'a, Ex, Fb, En, Hi> {
    XhpSimple(&'a (&'a Pstring<'a>, &'a Expr<'a, Ex, Fb, En, Hi>)),
    XhpSpread(&'a Expr<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for XhpAttribute<'a, Ex, Fb, En, Hi>
{
}

pub use oxidized::aast::IsVariadic;

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
pub struct FunParam<'a, Ex, Fb, En, Hi> {
    pub annotation: Ex,
    pub type_hint: &'a TypeHint<'a, Hi>,
    pub is_variadic: &'a oxidized::aast::IsVariadic,
    pub pos: &'a Pos<'a>,
    pub name: &'a str,
    pub expr: Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
    pub callconv: Option<oxidized::ast_defs::ParamKind>,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub visibility: Option<oxidized::aast::Visibility>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FunParam<'a, Ex, Fb, En, Hi>
{
}

/// does function take varying number of args?
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
pub enum FunVariadicity<'a, Ex, Fb, En, Hi> {
    /// PHP5.6 ...$args finishes the func declaration
    FVvariadicArg(&'a FunParam<'a, Ex, Fb, En, Hi>),
    /// HH ... finishes the declaration; deprecate for ...$args?
    FVellipsis(&'a Pos<'a>),
    /// standard non variadic function
    FVnonVariadic,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FunVariadicity<'a, Ex, Fb, En, Hi>
{
}

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
pub struct Fun_<'a, Ex, Fb, En, Hi> {
    pub span: &'a Pos<'a>,
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    pub ret: &'a TypeHint<'a, Hi>,
    pub name: Sid<'a>,
    pub tparams: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    pub where_constraints: &'a [&'a WhereConstraintHint<'a>],
    pub variadic: FunVariadicity<'a, Ex, Fb, En, Hi>,
    pub params: &'a [&'a FunParam<'a, Ex, Fb, En, Hi>],
    pub cap: &'a TypeHint<'a, Hi>,
    pub unsafe_cap: &'a TypeHint<'a, Hi>,
    pub body: &'a FuncBody<'a, Ex, Fb, En, Hi>,
    pub fun_kind: oxidized::ast_defs::FunKind,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub file_attributes: &'a [&'a FileAttribute<'a, Ex, Fb, En, Hi>],
    /// true if this declaration has no body because it is an
    /// external function declaration (e.g. from an HHI file)
    pub external: bool,
    pub namespace: &'a Nsenv<'a>,
    pub doc_comment: Option<&'a DocComment<'a>>,
    pub static_: bool,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Fun_<'a, Ex, Fb, En, Hi>
{
}

/// Naming has two phases and the annotation helps to indicate the phase.
/// In the first pass, it will perform naming on everything except for function
/// and method bodies and collect information needed. Then, another round of
/// naming is performed where function bodies are named. Thus, naming will
/// have named and unnamed variants of the annotation.
/// See BodyNamingAnnotation in nast.ml and the comment in naming.ml
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
pub struct FuncBody<'a, Ex, Fb, En, Hi> {
    pub ast: &'a Block<'a, Ex, Fb, En, Hi>,
    pub annotation: Fb,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FuncBody<'a, Ex, Fb, En, Hi>
{
}

/// A type annotation is two things:
/// - the localized hint, or if the hint is missing, the inferred type
/// - The typehint associated to this expression if it exists
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
pub struct TypeHint<'a, Hi>(pub Hi, pub &'a TypeHint_<'a>);
impl<'a, Hi: TrivialDrop> TrivialDrop for TypeHint<'a, Hi> {}

/// Explicit type argument to function, constructor, or collection literal.
/// 'hi = unit in NAST
/// 'hi = Typing_defs.(locl ty) in TAST,
/// and is used to record inferred type arguments, with wildcard hint.
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
pub struct Targ<'a, Hi>(pub Hi, pub &'a Hint<'a>);
impl<'a, Hi: TrivialDrop> TrivialDrop for Targ<'a, Hi> {}

pub type TypeHint_<'a> = Option<&'a Hint<'a>>;

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
pub struct UserAttribute<'a, Ex, Fb, En, Hi> {
    pub name: Sid<'a>,
    /// user attributes are restricted to scalar values
    pub params: &'a [&'a Expr<'a, Ex, Fb, En, Hi>],
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for UserAttribute<'a, Ex, Fb, En, Hi>
{
}

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
pub struct FileAttribute<'a, Ex, Fb, En, Hi> {
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub namespace: &'a Nsenv<'a>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for FileAttribute<'a, Ex, Fb, En, Hi>
{
}

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
pub struct Tparam<'a, Ex, Fb, En, Hi> {
    pub variance: oxidized::ast_defs::Variance,
    pub name: Sid<'a>,
    pub parameters: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    pub constraints: &'a [(oxidized::ast_defs::ConstraintKind, &'a Hint<'a>)],
    pub reified: oxidized::aast::ReifyKind,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Tparam<'a, Ex, Fb, En, Hi>
{
}

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
pub struct UseAsAlias<'a>(
    pub Option<Sid<'a>>,
    pub &'a Pstring<'a>,
    pub Option<Sid<'a>>,
    pub &'a [oxidized::aast::UseAsVisibility],
);
impl<'a> TrivialDrop for UseAsAlias<'a> {}

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
pub struct InsteadofAlias<'a>(pub Sid<'a>, pub &'a Pstring<'a>, pub &'a [Sid<'a>]);
impl<'a> TrivialDrop for InsteadofAlias<'a> {}

pub use oxidized::aast::IsExtends;

pub use oxidized::aast::EmitId;

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
pub struct Class_<'a, Ex, Fb, En, Hi> {
    pub span: &'a Pos<'a>,
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    pub final_: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub kind: oxidized::ast_defs::ClassKind,
    pub name: Sid<'a>,
    /// The type parameters of a class A<T> (T is the parameter)
    pub tparams: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    pub extends: &'a [&'a ClassHint<'a>],
    pub uses: &'a [&'a TraitHint<'a>],
    pub use_as_alias: &'a [&'a UseAsAlias<'a>],
    pub insteadof_alias: &'a [&'a InsteadofAlias<'a>],
    pub xhp_attr_uses: &'a [&'a XhpAttrHint<'a>],
    pub xhp_category: Option<&'a (&'a Pos<'a>, &'a [&'a Pstring<'a>])>,
    pub reqs: &'a [(&'a ClassHint<'a>, &'a oxidized::aast::IsExtends)],
    pub implements: &'a [&'a ClassHint<'a>],
    pub where_constraints: &'a [&'a WhereConstraintHint<'a>],
    pub consts: &'a [&'a ClassConst<'a, Ex, Fb, En, Hi>],
    pub typeconsts: &'a [&'a ClassTypeconst<'a, Ex, Fb, En, Hi>],
    pub vars: &'a [&'a ClassVar<'a, Ex, Fb, En, Hi>],
    pub methods: &'a [&'a Method_<'a, Ex, Fb, En, Hi>],
    pub attributes: &'a [ClassAttr<'a, Ex, Fb, En, Hi>],
    pub xhp_children: &'a [(&'a Pos<'a>, &'a XhpChild<'a>)],
    pub xhp_attrs: &'a [&'a XhpAttr<'a, Ex, Fb, En, Hi>],
    pub namespace: &'a Nsenv<'a>,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub file_attributes: &'a [&'a FileAttribute<'a, Ex, Fb, En, Hi>],
    pub enum_: Option<&'a Enum_<'a>>,
    pub pu_enums: &'a [&'a PuEnum<'a, Ex, Fb, En, Hi>],
    pub doc_comment: Option<&'a DocComment<'a>>,
    pub emit_id: Option<&'a oxidized::aast::EmitId>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Class_<'a, Ex, Fb, En, Hi>
{
}

pub type ClassHint<'a> = Hint<'a>;

pub type TraitHint<'a> = Hint<'a>;

pub type XhpAttrHint<'a> = Hint<'a>;

pub use oxidized::aast::XhpAttrTag;

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
pub struct XhpAttr<'a, Ex, Fb, En, Hi>(
    pub &'a TypeHint<'a, Hi>,
    pub &'a ClassVar<'a, Ex, Fb, En, Hi>,
    pub Option<oxidized::aast::XhpAttrTag>,
    pub Option<&'a (&'a Pos<'a>, &'a [&'a Expr<'a, Ex, Fb, En, Hi>])>,
);
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for XhpAttr<'a, Ex, Fb, En, Hi>
{
}

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
pub enum ClassAttr<'a, Ex, Fb, En, Hi> {
    CAName(&'a Sid<'a>),
    CAField(&'a CaField<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassAttr<'a, Ex, Fb, En, Hi>
{
}

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
pub struct CaField<'a, Ex, Fb, En, Hi> {
    pub type_: CaType<'a>,
    pub id: Sid<'a>,
    pub value: Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
    pub required: bool,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for CaField<'a, Ex, Fb, En, Hi>
{
}

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
pub enum CaType<'a> {
    CAHint(&'a Hint<'a>),
    CAEnum(&'a [&'a str]),
}
impl<'a> TrivialDrop for CaType<'a> {}

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
pub struct ClassConst<'a, Ex, Fb, En, Hi> {
    pub type_: Option<&'a Hint<'a>>,
    pub id: Sid<'a>,
    /// expr = None indicates an abstract const
    pub expr: Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassConst<'a, Ex, Fb, En, Hi>
{
}

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
pub enum TypeconstAbstractKind<'a> {
    TCAbstract(Option<&'a Hint<'a>>),
    TCPartiallyAbstract,
    TCConcrete,
}
impl<'a> TrivialDrop for TypeconstAbstractKind<'a> {}

/// This represents a type const definition. If a type const is abstract then
/// then the type hint acts as a constraint. Any concrete definition of the
/// type const must satisfy the constraint.
///
/// If the type const is not abstract then a type must be specified.
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
pub struct ClassTypeconst<'a, Ex, Fb, En, Hi> {
    pub abstract_: TypeconstAbstractKind<'a>,
    pub name: Sid<'a>,
    pub constraint: Option<&'a Hint<'a>>,
    pub type_: Option<&'a Hint<'a>>,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub span: &'a Pos<'a>,
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassTypeconst<'a, Ex, Fb, En, Hi>
{
}

pub use oxidized::aast::XhpAttrInfo;

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
pub struct ClassVar<'a, Ex, Fb, En, Hi> {
    pub final_: bool,
    pub xhp_attr: Option<&'a oxidized::aast::XhpAttrInfo>,
    pub abstract_: bool,
    pub visibility: oxidized::aast::Visibility,
    pub type_: &'a TypeHint<'a, Hi>,
    pub id: Sid<'a>,
    pub expr: Option<&'a Expr<'a, Ex, Fb, En, Hi>>,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub doc_comment: Option<&'a DocComment<'a>>,
    pub is_promoted_variadic: bool,
    pub is_static: bool,
    pub span: &'a Pos<'a>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for ClassVar<'a, Ex, Fb, En, Hi>
{
}

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
pub struct Method_<'a, Ex, Fb, En, Hi> {
    pub span: &'a Pos<'a>,
    pub annotation: En,
    pub final_: bool,
    pub abstract_: bool,
    pub static_: bool,
    pub visibility: oxidized::aast::Visibility,
    pub name: Sid<'a>,
    pub tparams: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    pub where_constraints: &'a [&'a WhereConstraintHint<'a>],
    pub variadic: FunVariadicity<'a, Ex, Fb, En, Hi>,
    pub params: &'a [&'a FunParam<'a, Ex, Fb, En, Hi>],
    pub cap: &'a TypeHint<'a, Hi>,
    pub unsafe_cap: &'a TypeHint<'a, Hi>,
    pub body: &'a FuncBody<'a, Ex, Fb, En, Hi>,
    pub fun_kind: oxidized::ast_defs::FunKind,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub ret: &'a TypeHint<'a, Hi>,
    /// true if this declaration has no body because it is an external method
    /// declaration (e.g. from an HHI file)
    pub external: bool,
    pub doc_comment: Option<&'a DocComment<'a>>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Method_<'a, Ex, Fb, En, Hi>
{
}

pub type Nsenv<'a> = namespace_env::Env<'a>;

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
pub struct Typedef<'a, Ex, Fb, En, Hi> {
    pub annotation: En,
    pub name: Sid<'a>,
    pub tparams: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    pub constraint: Option<&'a Hint<'a>>,
    pub kind: &'a Hint<'a>,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub mode: oxidized::file_info::Mode,
    pub vis: oxidized::aast::TypedefVisibility,
    pub namespace: &'a Nsenv<'a>,
    pub span: &'a Pos<'a>,
    pub emit_id: Option<&'a oxidized::aast::EmitId>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Typedef<'a, Ex, Fb, En, Hi>
{
}

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
pub struct Gconst<'a, Ex, Fb, En, Hi> {
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    pub name: Sid<'a>,
    pub type_: Option<&'a Hint<'a>>,
    pub value: &'a Expr<'a, Ex, Fb, En, Hi>,
    pub namespace: &'a Nsenv<'a>,
    pub span: &'a Pos<'a>,
    pub emit_id: Option<&'a oxidized::aast::EmitId>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Gconst<'a, Ex, Fb, En, Hi>
{
}

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
pub struct RecordDef<'a, Ex, Fb, En, Hi> {
    pub annotation: En,
    pub name: Sid<'a>,
    pub extends: Option<&'a RecordHint<'a>>,
    pub abstract_: bool,
    pub fields: &'a [(Sid<'a>, &'a Hint<'a>, Option<&'a Expr<'a, Ex, Fb, En, Hi>>)],
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub namespace: &'a Nsenv<'a>,
    pub span: &'a Pos<'a>,
    pub doc_comment: Option<&'a DocComment<'a>>,
    pub emit_id: Option<&'a oxidized::aast::EmitId>,
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for RecordDef<'a, Ex, Fb, En, Hi>
{
}

pub type RecordHint<'a> = Hint<'a>;

/// Pocket Universe Enumeration, e.g.
///
/// ```
///   enum Foo { // pu_name
///     // pu_case_types
///     case type T0;
///     case type T1;
///
///     // pu_case_values
///     case ?T0 default_value;
///     case T1 foo;
///
///     // pu_members
///     :@A( // pum_atom
///       // pum_types
///       type T0 = string,
///       type T1 = int,
///
///       // pum_exprs
///       default_value = null,
///       foo = 42,
///     );
///     :@B( ... )
///     ...
///   }
/// ```
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
pub struct PuEnum<'a, Ex, Fb, En, Hi> {
    pub annotation: En,
    pub name: Sid<'a>,
    pub user_attributes: &'a [&'a UserAttribute<'a, Ex, Fb, En, Hi>],
    pub is_final: bool,
    pub case_types: &'a [&'a Tparam<'a, Ex, Fb, En, Hi>],
    pub case_values: &'a [&'a PuCaseValue<'a>],
    pub members: &'a [&'a PuMember<'a, Ex, Fb, En, Hi>],
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for PuEnum<'a, Ex, Fb, En, Hi>
{
}

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
pub struct PuCaseValue<'a>(pub Sid<'a>, pub &'a Hint<'a>);
impl<'a> TrivialDrop for PuCaseValue<'a> {}

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
pub struct PuMember<'a, Ex, Fb, En, Hi> {
    pub atom: Sid<'a>,
    pub types: &'a [(Sid<'a>, &'a Hint<'a>)],
    pub exprs: &'a [(Sid<'a>, &'a Expr<'a, Ex, Fb, En, Hi>)],
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for PuMember<'a, Ex, Fb, En, Hi>
{
}

pub type FunDef<'a, Ex, Fb, En, Hi> = Fun_<'a, Ex, Fb, En, Hi>;

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
pub enum Def<'a, Ex, Fb, En, Hi> {
    Fun(&'a FunDef<'a, Ex, Fb, En, Hi>),
    Class(&'a Class_<'a, Ex, Fb, En, Hi>),
    RecordDef(&'a RecordDef<'a, Ex, Fb, En, Hi>),
    Stmt(&'a Stmt<'a, Ex, Fb, En, Hi>),
    Typedef(&'a Typedef<'a, Ex, Fb, En, Hi>),
    Constant(&'a Gconst<'a, Ex, Fb, En, Hi>),
    Namespace(&'a (Sid<'a>, &'a Program<'a, Ex, Fb, En, Hi>)),
    NamespaceUse(&'a [(oxidized::aast::NsKind, Sid<'a>, Sid<'a>)]),
    SetNamespaceEnv(&'a Nsenv<'a>),
    FileAttributes(&'a FileAttribute<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex: TrivialDrop, Fb: TrivialDrop, En: TrivialDrop, Hi: TrivialDrop> TrivialDrop
    for Def<'a, Ex, Fb, En, Hi>
{
}

pub use oxidized::aast::NsKind;

pub use oxidized::aast::BreakContinueLevel;

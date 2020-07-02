// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<cdb1bcddbc08e4a0e794520b7bf22a8e>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
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
pub type Program<'a, Ex, Fb, En, Hi> = &'a [Def<'a, Ex, Fb, En, Hi>];

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Stmt<'a, Ex, Fb, En, Hi>(pub &'a Pos<'a>, pub Stmt_<'a, Ex, Fb, En, Hi>);
impl<'a, Ex, Fb, En, Hi> TrivialDrop for Stmt<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Stmt_<'a, Ex, Fb, En, Hi> {
    Fallthrough,
    Expr(&'a Expr<'a, Ex, Fb, En, Hi>),
    Break,
    Continue,
    Throw(&'a Expr<'a, Ex, Fb, En, Hi>),
    Return(&'a Option<Expr<'a, Ex, Fb, En, Hi>>),
    GotoLabel(&'a Pstring<'a>),
    Goto(&'a Pstring<'a>),
    Awaitall(
        &'a (
            &'a [(Option<Lid<'a>>, Expr<'a, Ex, Fb, En, Hi>)],
            Block<'a, Ex, Fb, En, Hi>,
        ),
    ),
    If(
        &'a (
            Expr<'a, Ex, Fb, En, Hi>,
            Block<'a, Ex, Fb, En, Hi>,
            Block<'a, Ex, Fb, En, Hi>,
        ),
    ),
    Do(&'a (Block<'a, Ex, Fb, En, Hi>, Expr<'a, Ex, Fb, En, Hi>)),
    While(&'a (Expr<'a, Ex, Fb, En, Hi>, Block<'a, Ex, Fb, En, Hi>)),
    Using(&'a UsingStmt<'a, Ex, Fb, En, Hi>),
    For(
        &'a (
            Expr<'a, Ex, Fb, En, Hi>,
            Expr<'a, Ex, Fb, En, Hi>,
            Expr<'a, Ex, Fb, En, Hi>,
            Block<'a, Ex, Fb, En, Hi>,
        ),
    ),
    Switch(&'a (Expr<'a, Ex, Fb, En, Hi>, &'a [Case<'a, Ex, Fb, En, Hi>])),
    Foreach(
        &'a (
            Expr<'a, Ex, Fb, En, Hi>,
            AsExpr<'a, Ex, Fb, En, Hi>,
            Block<'a, Ex, Fb, En, Hi>,
        ),
    ),
    Try(
        &'a (
            Block<'a, Ex, Fb, En, Hi>,
            &'a [Catch<'a, Ex, Fb, En, Hi>],
            Block<'a, Ex, Fb, En, Hi>,
        ),
    ),
    Noop,
    Block(Block<'a, Ex, Fb, En, Hi>),
    Markup(&'a Pstring<'a>),
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for Stmt_<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct UsingStmt<'a, Ex, Fb, En, Hi> {
    pub is_block_scoped: bool,
    pub has_await: bool,
    pub expr: Expr<'a, Ex, Fb, En, Hi>,
    pub block: Block<'a, Ex, Fb, En, Hi>,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for UsingStmt<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum AsExpr<'a, Ex, Fb, En, Hi> {
    AsV(Expr<'a, Ex, Fb, En, Hi>),
    AsKv(Expr<'a, Ex, Fb, En, Hi>, Expr<'a, Ex, Fb, En, Hi>),
    AwaitAsV(&'a Pos<'a>, Expr<'a, Ex, Fb, En, Hi>),
    AwaitAsKv(
        &'a Pos<'a>,
        Expr<'a, Ex, Fb, En, Hi>,
        Expr<'a, Ex, Fb, En, Hi>,
    ),
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for AsExpr<'a, Ex, Fb, En, Hi> {}

pub type Block<'a, Ex, Fb, En, Hi> = &'a [Stmt<'a, Ex, Fb, En, Hi>];

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ClassId<'a, Ex, Fb, En, Hi>(pub Ex, pub ClassId_<'a, Ex, Fb, En, Hi>);
impl<'a, Ex, Fb, En, Hi> TrivialDrop for ClassId<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum ClassId_<'a, Ex, Fb, En, Hi> {
    CIparent,
    CIself,
    CIstatic,
    CIexpr(Expr<'a, Ex, Fb, En, Hi>),
    CI(Sid<'a>),
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for ClassId_<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Copy, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Expr<'a, Ex, Fb, En, Hi>(pub Ex, pub Expr_<'a, Ex, Fb, En, Hi>);

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum CollectionTarg<'a, Hi> {
    CollectionTV(Targ<'a, Hi>),
    CollectionTKV(Targ<'a, Hi>, Targ<'a, Hi>),
}
impl<'a, Hi> TrivialDrop for CollectionTarg<'a, Hi> {}

#[derive(
    Clone, Copy, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Expr_<'a, Ex, Fb, En, Hi> {
    Array(&'a [Afield<'a, Ex, Fb, En, Hi>]),
    Darray(
        &'a (
            Option<(Targ<'a, Hi>, Targ<'a, Hi>)>,
            &'a [(Expr<'a, Ex, Fb, En, Hi>, Expr<'a, Ex, Fb, En, Hi>)],
        ),
    ),
    Varray(&'a (Option<Targ<'a, Hi>>, &'a [Expr<'a, Ex, Fb, En, Hi>])),
    Shape(&'a [(ast_defs::ShapeFieldName<'a>, Expr<'a, Ex, Fb, En, Hi>)]),
    /// TODO: T38184446 Consolidate collections in AAST
    ValCollection(
        &'a (
            oxidized::aast::VcKind,
            Option<Targ<'a, Hi>>,
            &'a [Expr<'a, Ex, Fb, En, Hi>],
        ),
    ),
    /// TODO: T38184446 Consolidate collections in AAST
    KeyValCollection(
        &'a (
            oxidized::aast::KvcKind,
            Option<(Targ<'a, Hi>, Targ<'a, Hi>)>,
            &'a [Field<'a, Ex, Fb, En, Hi>],
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
            Expr<'a, Ex, Fb, En, Hi>,
            Expr<'a, Ex, Fb, En, Hi>,
            oxidized::aast::OgNullFlavor,
        ),
    ),
    ArrayGet(&'a (Expr<'a, Ex, Fb, En, Hi>, Option<Expr<'a, Ex, Fb, En, Hi>>)),
    ClassGet(
        &'a (
            ClassId<'a, Ex, Fb, En, Hi>,
            ClassGetExpr<'a, Ex, Fb, En, Hi>,
        ),
    ),
    ClassConst(&'a (ClassId<'a, Ex, Fb, En, Hi>, Pstring<'a>)),
    Call(
        &'a (
            oxidized::aast::CallType,
            Expr<'a, Ex, Fb, En, Hi>,
            &'a [Targ<'a, Hi>],
            &'a [Expr<'a, Ex, Fb, En, Hi>],
            Option<Expr<'a, Ex, Fb, En, Hi>>,
        ),
    ),
    FunctionPointer(&'a (Expr<'a, Ex, Fb, En, Hi>, &'a [Targ<'a, Hi>])),
    Int(&'a str),
    Float(&'a str),
    String(&'a str),
    String2(&'a [Expr<'a, Ex, Fb, En, Hi>]),
    PrefixedString(&'a (&'a str, Expr<'a, Ex, Fb, En, Hi>)),
    Yield(&'a Afield<'a, Ex, Fb, En, Hi>),
    YieldBreak,
    Await(&'a Expr<'a, Ex, Fb, En, Hi>),
    Suspend(&'a Expr<'a, Ex, Fb, En, Hi>),
    List(&'a [Expr<'a, Ex, Fb, En, Hi>]),
    ExprList(&'a [Expr<'a, Ex, Fb, En, Hi>]),
    Cast(&'a (Hint<'a>, Expr<'a, Ex, Fb, En, Hi>)),
    Unop(&'a (oxidized::ast_defs::Uop, Expr<'a, Ex, Fb, En, Hi>)),
    Binop(
        &'a (
            ast_defs::Bop<'a>,
            Expr<'a, Ex, Fb, En, Hi>,
            Expr<'a, Ex, Fb, En, Hi>,
        ),
    ),
    /// The lid is the ID of the $$ that is implicitly declared by this pipe.
    Pipe(&'a (Lid<'a>, Expr<'a, Ex, Fb, En, Hi>, Expr<'a, Ex, Fb, En, Hi>)),
    Eif(
        &'a (
            Expr<'a, Ex, Fb, En, Hi>,
            Option<Expr<'a, Ex, Fb, En, Hi>>,
            Expr<'a, Ex, Fb, En, Hi>,
        ),
    ),
    Is(&'a (Expr<'a, Ex, Fb, En, Hi>, Hint<'a>)),
    As(&'a (Expr<'a, Ex, Fb, En, Hi>, Hint<'a>, bool)),
    New(
        &'a (
            ClassId<'a, Ex, Fb, En, Hi>,
            &'a [Targ<'a, Hi>],
            &'a [Expr<'a, Ex, Fb, En, Hi>],
            Option<Expr<'a, Ex, Fb, En, Hi>>,
            Ex,
        ),
    ),
    Record(
        &'a (
            Sid<'a>,
            &'a [(Expr<'a, Ex, Fb, En, Hi>, Expr<'a, Ex, Fb, En, Hi>)],
        ),
    ),
    Efun(&'a (Fun_<'a, Ex, Fb, En, Hi>, &'a [Lid<'a>])),
    Lfun(&'a (Fun_<'a, Ex, Fb, En, Hi>, &'a [Lid<'a>])),
    Xml(
        &'a (
            Sid<'a>,
            &'a [XhpAttribute<'a, Ex, Fb, En, Hi>],
            &'a [Expr<'a, Ex, Fb, En, Hi>],
        ),
    ),
    Callconv(&'a (oxidized::ast_defs::ParamKind, Expr<'a, Ex, Fb, En, Hi>)),
    Import(&'a (oxidized::aast::ImportFlavor, Expr<'a, Ex, Fb, En, Hi>)),
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
    Lplaceholder(&'a Pos<'a>),
    FunId(&'a Sid<'a>),
    MethodId(&'a (Expr<'a, Ex, Fb, En, Hi>, Pstring<'a>)),
    /// meth_caller('Class name', 'method name')
    MethodCaller(&'a (Sid<'a>, Pstring<'a>)),
    SmethodId(&'a (Sid<'a>, Pstring<'a>)),
    Pair(
        &'a (
            Option<(Targ<'a, Hi>, Targ<'a, Hi>)>,
            Expr<'a, Ex, Fb, En, Hi>,
            Expr<'a, Ex, Fb, En, Hi>,
        ),
    ),
    Assert(&'a AssertExpr<'a, Ex, Fb, En, Hi>),
    PUAtom(&'a str),
    PUIdentifier(&'a (ClassId<'a, Ex, Fb, En, Hi>, Pstring<'a>, Pstring<'a>)),
    Any,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum ClassGetExpr<'a, Ex, Fb, En, Hi> {
    CGstring(Pstring<'a>),
    CGexpr(Expr<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for ClassGetExpr<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum AssertExpr<'a, Ex, Fb, En, Hi> {
    AEAssert(Expr<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for AssertExpr<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Case<'a, Ex, Fb, En, Hi> {
    Default(&'a Pos<'a>, Block<'a, Ex, Fb, En, Hi>),
    Case(Expr<'a, Ex, Fb, En, Hi>, Block<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for Case<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Catch<'a, Ex, Fb, En, Hi>(pub Sid<'a>, pub Lid<'a>, pub Block<'a, Ex, Fb, En, Hi>);
impl<'a, Ex, Fb, En, Hi> TrivialDrop for Catch<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Field<'a, Ex, Fb, En, Hi>(pub Expr<'a, Ex, Fb, En, Hi>, pub Expr<'a, Ex, Fb, En, Hi>);
impl<'a, Ex, Fb, En, Hi> TrivialDrop for Field<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Afield<'a, Ex, Fb, En, Hi> {
    AFvalue(Expr<'a, Ex, Fb, En, Hi>),
    AFkvalue(Expr<'a, Ex, Fb, En, Hi>, Expr<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for Afield<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum XhpAttribute<'a, Ex, Fb, En, Hi> {
    XhpSimple(Pstring<'a>, Expr<'a, Ex, Fb, En, Hi>),
    XhpSpread(Expr<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for XhpAttribute<'a, Ex, Fb, En, Hi> {}

pub use oxidized::aast::IsVariadic;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct FunParam<'a, Ex, Fb, En, Hi> {
    pub annotation: Ex,
    pub type_hint: TypeHint<'a, Hi>,
    pub is_variadic: oxidized::aast::IsVariadic,
    pub pos: &'a Pos<'a>,
    pub name: &'a str,
    pub expr: Option<Expr<'a, Ex, Fb, En, Hi>>,
    pub callconv: Option<oxidized::ast_defs::ParamKind>,
    pub user_attributes: &'a [UserAttribute<'a, Ex, Fb, En, Hi>],
    pub visibility: Option<oxidized::aast::Visibility>,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for FunParam<'a, Ex, Fb, En, Hi> {}

/// does function take varying number of args?
#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum FunVariadicity<'a, Ex, Fb, En, Hi> {
    /// PHP5.6 ...$args finishes the func declaration
    FVvariadicArg(&'a FunParam<'a, Ex, Fb, En, Hi>),
    /// HH ... finishes the declaration; deprecate for ...$args?
    FVellipsis(&'a Pos<'a>),
    /// standard non variadic function
    FVnonVariadic,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for FunVariadicity<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Fun_<'a, Ex, Fb, En, Hi> {
    pub span: &'a Pos<'a>,
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    pub ret: TypeHint<'a, Hi>,
    pub name: Sid<'a>,
    pub tparams: &'a [Tparam<'a, Ex, Fb, En, Hi>],
    pub where_constraints: &'a [WhereConstraint<'a>],
    pub variadic: FunVariadicity<'a, Ex, Fb, En, Hi>,
    pub params: &'a [&'a FunParam<'a, Ex, Fb, En, Hi>],
    pub body: FuncBody<'a, Ex, Fb, En, Hi>,
    pub fun_kind: oxidized::ast_defs::FunKind,
    pub user_attributes: &'a [UserAttribute<'a, Ex, Fb, En, Hi>],
    pub file_attributes: &'a [FileAttribute<'a, Ex, Fb, En, Hi>],
    /// true if this declaration has no body because it is an
    /// external function declaration (e.g. from an HHI file)
    pub external: bool,
    pub namespace: Nsenv<'a>,
    pub doc_comment: Option<DocComment<'a>>,
    pub static_: bool,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for Fun_<'a, Ex, Fb, En, Hi> {}

/// Naming has two phases and the annotation helps to indicate the phase.
/// In the first pass, it will perform naming on everything except for function
/// and method bodies and collect information needed. Then, another round of
/// naming is performed where function bodies are named. Thus, naming will
/// have named and unnamed variants of the annotation.
/// See BodyNamingAnnotation in nast.ml and the comment in naming.ml
#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct FuncBody<'a, Ex, Fb, En, Hi> {
    pub ast: Block<'a, Ex, Fb, En, Hi>,
    pub annotation: Fb,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for FuncBody<'a, Ex, Fb, En, Hi> {}

/// A type annotation is two things:
/// - the localized hint, or if the hint is missing, the inferred type
/// - The typehint associated to this expression if it exists
#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct TypeHint<'a, Hi>(pub Hi, pub TypeHint_<'a>);
impl<'a, Hi> TrivialDrop for TypeHint<'a, Hi> {}

/// Explicit type argument to function, constructor, or collection literal.
/// 'hi = unit in NAST
/// 'hi = Typing_defs.(locl ty) in TAST,
/// and is used to record inferred type arguments, with wildcard hint.
#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Targ<'a, Hi>(pub Hi, pub Hint<'a>);
impl<'a, Hi> TrivialDrop for Targ<'a, Hi> {}

pub type TypeHint_<'a> = Option<Hint<'a>>;

#[derive(
    Clone, Copy, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct UserAttribute<'a, Ex, Fb, En, Hi> {
    pub name: Sid<'a>,
    /// user attributes are restricted to scalar values
    pub params: &'a [Expr<'a, Ex, Fb, En, Hi>],
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct FileAttribute<'a, Ex, Fb, En, Hi> {
    pub user_attributes: &'a [UserAttribute<'a, Ex, Fb, En, Hi>],
    pub namespace: Nsenv<'a>,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for FileAttribute<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Tparam<'a, Ex, Fb, En, Hi> {
    pub variance: oxidized::ast_defs::Variance,
    pub name: Sid<'a>,
    pub constraints: &'a [(oxidized::ast_defs::ConstraintKind, Hint<'a>)],
    pub reified: oxidized::aast::ReifyKind,
    pub user_attributes: &'a [UserAttribute<'a, Ex, Fb, En, Hi>],
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for Tparam<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ClassTparams<'a, Ex, Fb, En, Hi> {
    pub list: &'a [Tparam<'a, Ex, Fb, En, Hi>],
    /// keeping around the ast version of the constraint only
    /// for the purposes of Naming.class_meth_bodies
    /// TODO: remove this and use tp_constraints
    pub constraints: s_map::SMap<
        'a,
        (
            oxidized::aast::ReifyKind,
            &'a [(oxidized::ast_defs::ConstraintKind, Hint<'a>)],
        ),
    >,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for ClassTparams<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct UseAsAlias<'a>(
    pub Option<Sid<'a>>,
    pub Pstring<'a>,
    pub Option<Sid<'a>>,
    pub &'a [oxidized::aast::UseAsVisibility],
);
impl<'a> TrivialDrop for UseAsAlias<'a> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct InsteadofAlias<'a>(pub Sid<'a>, pub Pstring<'a>, pub &'a [Sid<'a>]);
impl<'a> TrivialDrop for InsteadofAlias<'a> {}

pub use oxidized::aast::IsExtends;

pub use oxidized::aast::EmitId;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
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
    pub tparams: ClassTparams<'a, Ex, Fb, En, Hi>,
    pub extends: &'a [ClassHint<'a>],
    pub uses: &'a [TraitHint<'a>],
    pub use_as_alias: &'a [UseAsAlias<'a>],
    pub insteadof_alias: &'a [InsteadofAlias<'a>],
    pub method_redeclarations: &'a [MethodRedeclaration<'a, Ex, Fb, En, Hi>],
    pub xhp_attr_uses: &'a [XhpAttrHint<'a>],
    pub xhp_category: Option<(&'a Pos<'a>, &'a [Pstring<'a>])>,
    pub reqs: &'a [(ClassHint<'a>, oxidized::aast::IsExtends)],
    pub implements: &'a [ClassHint<'a>],
    pub where_constraints: &'a [WhereConstraint<'a>],
    pub consts: &'a [ClassConst<'a, Ex, Fb, En, Hi>],
    pub typeconsts: &'a [ClassTypeconst<'a, Ex, Fb, En, Hi>],
    pub vars: &'a [ClassVar<'a, Ex, Fb, En, Hi>],
    pub methods: &'a [Method_<'a, Ex, Fb, En, Hi>],
    pub attributes: &'a [ClassAttr<'a, Ex, Fb, En, Hi>],
    pub xhp_children: &'a [(&'a Pos<'a>, XhpChild<'a>)],
    pub xhp_attrs: &'a [XhpAttr<'a, Ex, Fb, En, Hi>],
    pub namespace: Nsenv<'a>,
    pub user_attributes: &'a [UserAttribute<'a, Ex, Fb, En, Hi>],
    pub file_attributes: &'a [FileAttribute<'a, Ex, Fb, En, Hi>],
    pub enum_: Option<Enum_<'a>>,
    pub pu_enums: &'a [PuEnum<'a, Ex, Fb, En, Hi>],
    pub doc_comment: Option<DocComment<'a>>,
    pub emit_id: Option<oxidized::aast::EmitId>,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for Class_<'a, Ex, Fb, En, Hi> {}

pub type ClassHint<'a> = Hint<'a>;

pub type TraitHint<'a> = Hint<'a>;

pub type XhpAttrHint<'a> = Hint<'a>;

pub use oxidized::aast::XhpAttrTag;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct XhpAttr<'a, Ex, Fb, En, Hi>(
    pub TypeHint<'a, Hi>,
    pub ClassVar<'a, Ex, Fb, En, Hi>,
    pub Option<oxidized::aast::XhpAttrTag>,
    pub Option<(&'a Pos<'a>, bool, &'a [Expr<'a, Ex, Fb, En, Hi>])>,
);
impl<'a, Ex, Fb, En, Hi> TrivialDrop for XhpAttr<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum ClassAttr<'a, Ex, Fb, En, Hi> {
    CAName(Sid<'a>),
    CAField(CaField<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for ClassAttr<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct CaField<'a, Ex, Fb, En, Hi> {
    pub type_: CaType<'a>,
    pub id: Sid<'a>,
    pub value: Option<Expr<'a, Ex, Fb, En, Hi>>,
    pub required: bool,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for CaField<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum CaType<'a> {
    CAHint(Hint<'a>),
    CAEnum(&'a [&'a str]),
}
impl<'a> TrivialDrop for CaType<'a> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ClassConst<'a, Ex, Fb, En, Hi> {
    pub type_: Option<Hint<'a>>,
    pub id: Sid<'a>,
    /// expr = None indicates an abstract const
    pub expr: Option<Expr<'a, Ex, Fb, En, Hi>>,
    pub doc_comment: Option<DocComment<'a>>,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for ClassConst<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum TypeconstAbstractKind<'a> {
    TCAbstract(Option<Hint<'a>>),
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
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ClassTypeconst<'a, Ex, Fb, En, Hi> {
    pub abstract_: TypeconstAbstractKind<'a>,
    pub name: Sid<'a>,
    pub constraint: Option<Hint<'a>>,
    pub type_: Option<Hint<'a>>,
    pub user_attributes: &'a [UserAttribute<'a, Ex, Fb, En, Hi>],
    pub span: &'a Pos<'a>,
    pub doc_comment: Option<DocComment<'a>>,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for ClassTypeconst<'a, Ex, Fb, En, Hi> {}

pub use oxidized::aast::XhpAttrInfo;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ClassVar<'a, Ex, Fb, En, Hi> {
    pub final_: bool,
    pub xhp_attr: Option<oxidized::aast::XhpAttrInfo>,
    pub abstract_: bool,
    pub visibility: oxidized::aast::Visibility,
    pub type_: TypeHint<'a, Hi>,
    pub id: Sid<'a>,
    pub expr: Option<Expr<'a, Ex, Fb, En, Hi>>,
    pub user_attributes: &'a [UserAttribute<'a, Ex, Fb, En, Hi>],
    pub doc_comment: Option<DocComment<'a>>,
    pub is_promoted_variadic: bool,
    pub is_static: bool,
    pub span: &'a Pos<'a>,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for ClassVar<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Method_<'a, Ex, Fb, En, Hi> {
    pub span: &'a Pos<'a>,
    pub annotation: En,
    pub final_: bool,
    pub abstract_: bool,
    pub static_: bool,
    pub visibility: oxidized::aast::Visibility,
    pub name: Sid<'a>,
    pub tparams: &'a [Tparam<'a, Ex, Fb, En, Hi>],
    pub where_constraints: &'a [WhereConstraint<'a>],
    pub variadic: FunVariadicity<'a, Ex, Fb, En, Hi>,
    pub params: &'a [&'a FunParam<'a, Ex, Fb, En, Hi>],
    pub body: FuncBody<'a, Ex, Fb, En, Hi>,
    pub fun_kind: oxidized::ast_defs::FunKind,
    pub user_attributes: &'a [UserAttribute<'a, Ex, Fb, En, Hi>],
    pub ret: TypeHint<'a, Hi>,
    /// true if this declaration has no body because it is an external method
    /// declaration (e.g. from an HHI file)
    pub external: bool,
    pub doc_comment: Option<DocComment<'a>>,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for Method_<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct MethodRedeclaration<'a, Ex, Fb, En, Hi> {
    pub final_: bool,
    pub abstract_: bool,
    pub static_: bool,
    pub visibility: oxidized::aast::Visibility,
    pub name: Sid<'a>,
    pub tparams: &'a [Tparam<'a, Ex, Fb, En, Hi>],
    pub where_constraints: &'a [WhereConstraint<'a>],
    pub variadic: FunVariadicity<'a, Ex, Fb, En, Hi>,
    pub params: &'a [&'a FunParam<'a, Ex, Fb, En, Hi>],
    pub fun_kind: oxidized::ast_defs::FunKind,
    pub ret: TypeHint<'a, Hi>,
    pub trait_: TraitHint<'a>,
    pub method: Pstring<'a>,
    pub user_attributes: &'a [UserAttribute<'a, Ex, Fb, En, Hi>],
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for MethodRedeclaration<'a, Ex, Fb, En, Hi> {}

pub type Nsenv<'a> = namespace_env::Env<'a>;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Typedef<'a, Ex, Fb, En, Hi> {
    pub annotation: En,
    pub name: Sid<'a>,
    pub tparams: &'a [Tparam<'a, Ex, Fb, En, Hi>],
    pub constraint: Option<Hint<'a>>,
    pub kind: Hint<'a>,
    pub user_attributes: &'a [UserAttribute<'a, Ex, Fb, En, Hi>],
    pub mode: oxidized::file_info::Mode,
    pub vis: oxidized::aast::TypedefVisibility,
    pub namespace: Nsenv<'a>,
    pub emit_id: Option<oxidized::aast::EmitId>,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for Typedef<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Gconst<'a, Ex, Fb, En, Hi> {
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    pub name: Sid<'a>,
    pub type_: Option<Hint<'a>>,
    pub value: Expr<'a, Ex, Fb, En, Hi>,
    pub namespace: Nsenv<'a>,
    pub span: &'a Pos<'a>,
    pub emit_id: Option<oxidized::aast::EmitId>,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for Gconst<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct RecordDef<'a, Ex, Fb, En, Hi> {
    pub annotation: En,
    pub name: Sid<'a>,
    pub extends: Option<RecordHint<'a>>,
    pub abstract_: bool,
    pub fields: &'a [(Sid<'a>, Hint<'a>, Option<Expr<'a, Ex, Fb, En, Hi>>)],
    pub user_attributes: &'a [UserAttribute<'a, Ex, Fb, En, Hi>],
    pub namespace: Nsenv<'a>,
    pub span: &'a Pos<'a>,
    pub doc_comment: Option<DocComment<'a>>,
    pub emit_id: Option<oxidized::aast::EmitId>,
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for RecordDef<'a, Ex, Fb, En, Hi> {}

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
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct PuEnum<'a, Ex, Fb, En, Hi> {
    pub annotation: En,
    pub name: Sid<'a>,
    pub user_attributes: &'a [UserAttribute<'a, Ex, Fb, En, Hi>],
    pub is_final: bool,
    pub case_types: &'a [Tparam<'a, Ex, Fb, En, Hi>],
    pub case_values: &'a [PuCaseValue<'a>],
    pub members: &'a [PuMember<'a, Ex, Fb, En, Hi>],
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for PuEnum<'a, Ex, Fb, En, Hi> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct PuCaseValue<'a>(pub Sid<'a>, pub Hint<'a>);
impl<'a> TrivialDrop for PuCaseValue<'a> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct PuMember<'a, Ex, Fb, En, Hi> {
    pub atom: Sid<'a>,
    pub types: &'a [(Sid<'a>, Hint<'a>)],
    pub exprs: &'a [(Sid<'a>, Expr<'a, Ex, Fb, En, Hi>)],
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for PuMember<'a, Ex, Fb, En, Hi> {}

pub type FunDef<'a, Ex, Fb, En, Hi> = Fun_<'a, Ex, Fb, En, Hi>;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Def<'a, Ex, Fb, En, Hi> {
    Fun(&'a FunDef<'a, Ex, Fb, En, Hi>),
    Class(&'a Class_<'a, Ex, Fb, En, Hi>),
    RecordDef(&'a RecordDef<'a, Ex, Fb, En, Hi>),
    Stmt(&'a Stmt<'a, Ex, Fb, En, Hi>),
    Typedef(&'a Typedef<'a, Ex, Fb, En, Hi>),
    Constant(&'a Gconst<'a, Ex, Fb, En, Hi>),
    Namespace(&'a (Sid<'a>, Program<'a, Ex, Fb, En, Hi>)),
    NamespaceUse(&'a [(oxidized::aast::NsKind, Sid<'a>, Sid<'a>)]),
    SetNamespaceEnv(&'a Nsenv<'a>),
    FileAttributes(&'a FileAttribute<'a, Ex, Fb, En, Hi>),
}
impl<'a, Ex, Fb, En, Hi> TrivialDrop for Def<'a, Ex, Fb, En, Hi> {}

pub use oxidized::aast::NsKind;

pub use oxidized::aast::ReifyKind;

pub use oxidized::aast::BreakContinueLevel;

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<c3132d8952bf645a2aa75a3f1b4efaf9>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use aast_defs::*;

/// Aast.program represents the top-level definitions in a Hack program.
/// ex: Expression annotation type (when typechecking, the inferred dtype)
/// fb: Function body tag (e.g. has naming occurred)
/// en: Environment (tracking state inside functions and classes)
/// hi: Hint annotation (when typechecking it will be the localized type hint or the
/// inferred missing type if the hint is missing)
pub type Program<Ex, Fb, En, Hi> = Vec<Def<Ex, Fb, En, Hi>>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct Stmt<Ex, Fb, En, Hi>(pub Pos, pub Stmt_<Ex, Fb, En, Hi>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum Stmt_<Ex, Fb, En, Hi> {
    Fallthrough,
    Expr(Box<Expr<Ex, Fb, En, Hi>>),
    Break,
    Continue,
    Throw(Box<Expr<Ex, Fb, En, Hi>>),
    Return(Box<Option<Expr<Ex, Fb, En, Hi>>>),
    GotoLabel(Box<Pstring>),
    Goto(Box<Pstring>),
    Awaitall(
        Box<(
            Vec<(Option<Lid>, Expr<Ex, Fb, En, Hi>)>,
            Block<Ex, Fb, En, Hi>,
        )>,
    ),
    If(
        Box<(
            Expr<Ex, Fb, En, Hi>,
            Block<Ex, Fb, En, Hi>,
            Block<Ex, Fb, En, Hi>,
        )>,
    ),
    Do(Box<(Block<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>),
    While(Box<(Expr<Ex, Fb, En, Hi>, Block<Ex, Fb, En, Hi>)>),
    Using(Box<UsingStmt<Ex, Fb, En, Hi>>),
    For(
        Box<(
            Expr<Ex, Fb, En, Hi>,
            Expr<Ex, Fb, En, Hi>,
            Expr<Ex, Fb, En, Hi>,
            Block<Ex, Fb, En, Hi>,
        )>,
    ),
    Switch(Box<(Expr<Ex, Fb, En, Hi>, Vec<Case<Ex, Fb, En, Hi>>)>),
    Foreach(
        Box<(
            Expr<Ex, Fb, En, Hi>,
            AsExpr<Ex, Fb, En, Hi>,
            Block<Ex, Fb, En, Hi>,
        )>,
    ),
    Try(
        Box<(
            Block<Ex, Fb, En, Hi>,
            Vec<Catch<Ex, Fb, En, Hi>>,
            Block<Ex, Fb, En, Hi>,
        )>,
    ),
    Noop,
    Block(Block<Ex, Fb, En, Hi>),
    Markup(Box<Pstring>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct UsingStmt<Ex, Fb, En, Hi> {
    pub is_block_scoped: bool,
    pub has_await: bool,
    pub expr: Expr<Ex, Fb, En, Hi>,
    pub block: Block<Ex, Fb, En, Hi>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum AsExpr<Ex, Fb, En, Hi> {
    AsV(Expr<Ex, Fb, En, Hi>),
    AsKv(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>),
    AwaitAsV(Pos, Expr<Ex, Fb, En, Hi>),
    AwaitAsKv(Pos, Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>),
}

pub type Block<Ex, Fb, En, Hi> = Vec<Stmt<Ex, Fb, En, Hi>>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct ClassId<Ex, Fb, En, Hi>(pub Ex, pub ClassId_<Ex, Fb, En, Hi>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum ClassId_<Ex, Fb, En, Hi> {
    CIparent,
    CIself,
    CIstatic,
    CIexpr(Expr<Ex, Fb, En, Hi>),
    CI(Sid),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct Expr<Ex, Fb, En, Hi>(pub Ex, pub Expr_<Ex, Fb, En, Hi>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum CollectionTarg<Hi> {
    CollectionTV(Targ<Hi>),
    CollectionTKV(Targ<Hi>, Targ<Hi>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum Expr_<Ex, Fb, En, Hi> {
    Array(Vec<Afield<Ex, Fb, En, Hi>>),
    Darray(
        Box<(
            Option<(Targ<Hi>, Targ<Hi>)>,
            Vec<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>,
        )>,
    ),
    Varray(Box<(Option<Targ<Hi>>, Vec<Expr<Ex, Fb, En, Hi>>)>),
    Shape(Vec<(ast_defs::ShapeFieldName, Expr<Ex, Fb, En, Hi>)>),
    /// TODO: T38184446 Consolidate collections in AAST
    ValCollection(Box<(VcKind, Option<Targ<Hi>>, Vec<Expr<Ex, Fb, En, Hi>>)>),
    /// TODO: T38184446 Consolidate collections in AAST
    KeyValCollection(
        Box<(
            KvcKind,
            Option<(Targ<Hi>, Targ<Hi>)>,
            Vec<Field<Ex, Fb, En, Hi>>,
        )>,
    ),
    Null,
    This,
    True,
    False,
    Omitted,
    Id(Box<Sid>),
    Lvar(Box<Lid>),
    Dollardollar(Box<Lid>),
    Clone(Box<Expr<Ex, Fb, En, Hi>>),
    ObjGet(Box<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>, OgNullFlavor)>),
    ArrayGet(Box<(Expr<Ex, Fb, En, Hi>, Option<Expr<Ex, Fb, En, Hi>>)>),
    ClassGet(Box<(ClassId<Ex, Fb, En, Hi>, ClassGetExpr<Ex, Fb, En, Hi>)>),
    ClassConst(Box<(ClassId<Ex, Fb, En, Hi>, Pstring)>),
    Call(
        Box<(
            CallType,
            Expr<Ex, Fb, En, Hi>,
            Vec<Targ<Hi>>,
            Vec<Expr<Ex, Fb, En, Hi>>,
            Option<Expr<Ex, Fb, En, Hi>>,
        )>,
    ),
    FunctionPointer(Box<(Expr<Ex, Fb, En, Hi>, Vec<Targ<Hi>>)>),
    Int(String),
    Float(String),
    String(String),
    String2(Vec<Expr<Ex, Fb, En, Hi>>),
    PrefixedString(Box<(String, Expr<Ex, Fb, En, Hi>)>),
    Yield(Box<Afield<Ex, Fb, En, Hi>>),
    YieldBreak,
    YieldFrom(Box<Expr<Ex, Fb, En, Hi>>),
    Await(Box<Expr<Ex, Fb, En, Hi>>),
    Suspend(Box<Expr<Ex, Fb, En, Hi>>),
    List(Vec<Expr<Ex, Fb, En, Hi>>),
    ExprList(Vec<Expr<Ex, Fb, En, Hi>>),
    Cast(Box<(Hint, Expr<Ex, Fb, En, Hi>)>),
    Unop(Box<(ast_defs::Uop, Expr<Ex, Fb, En, Hi>)>),
    Binop(Box<(ast_defs::Bop, Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>),
    /// The lid is the ID of the $$ that is implicitly declared by this pipe.
    Pipe(Box<(Lid, Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>),
    Eif(
        Box<(
            Expr<Ex, Fb, En, Hi>,
            Option<Expr<Ex, Fb, En, Hi>>,
            Expr<Ex, Fb, En, Hi>,
        )>,
    ),
    Is(Box<(Expr<Ex, Fb, En, Hi>, Hint)>),
    As(Box<(Expr<Ex, Fb, En, Hi>, Hint, bool)>),
    New(
        Box<(
            ClassId<Ex, Fb, En, Hi>,
            Vec<Targ<Hi>>,
            Vec<Expr<Ex, Fb, En, Hi>>,
            Option<Expr<Ex, Fb, En, Hi>>,
            Ex,
        )>,
    ),
    Record(Box<(Sid, bool, Vec<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>)>),
    Efun(Box<(Fun_<Ex, Fb, En, Hi>, Vec<Lid>)>),
    Lfun(Box<(Fun_<Ex, Fb, En, Hi>, Vec<Lid>)>),
    Xml(
        Box<(
            Sid,
            Vec<XhpAttribute<Ex, Fb, En, Hi>>,
            Vec<Expr<Ex, Fb, En, Hi>>,
        )>,
    ),
    Callconv(Box<(ast_defs::ParamKind, Expr<Ex, Fb, En, Hi>)>),
    Import(Box<(ImportFlavor, Expr<Ex, Fb, En, Hi>)>),
    /// TODO: T38184446 Consolidate collections in AAST
    Collection(Box<(Sid, Option<CollectionTarg<Hi>>, Vec<Afield<Ex, Fb, En, Hi>>)>),
    BracedExpr(Box<Expr<Ex, Fb, En, Hi>>),
    ParenthesizedExpr(Box<Expr<Ex, Fb, En, Hi>>),
    Lplaceholder(Box<Pos>),
    FunId(Box<Sid>),
    MethodId(Box<(Expr<Ex, Fb, En, Hi>, Pstring)>),
    /// meth_caller('Class name', 'method name')
    MethodCaller(Box<(Sid, Pstring)>),
    SmethodId(Box<(Sid, Pstring)>),
    Pair(Box<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>),
    Assert(Box<AssertExpr<Ex, Fb, En, Hi>>),
    Typename(Box<Sid>),
    PUAtom(String),
    PUIdentifier(Box<(ClassId<Ex, Fb, En, Hi>, Pstring, Pstring)>),
    Any,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum ClassGetExpr<Ex, Fb, En, Hi> {
    CGstring(Pstring),
    CGexpr(Expr<Ex, Fb, En, Hi>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum AssertExpr<Ex, Fb, En, Hi> {
    AEAssert(Expr<Ex, Fb, En, Hi>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum Case<Ex, Fb, En, Hi> {
    Default(Pos, Block<Ex, Fb, En, Hi>),
    Case(Expr<Ex, Fb, En, Hi>, Block<Ex, Fb, En, Hi>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct Catch<Ex, Fb, En, Hi>(pub Sid, pub Lid, pub Block<Ex, Fb, En, Hi>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct Field<Ex, Fb, En, Hi>(pub Expr<Ex, Fb, En, Hi>, pub Expr<Ex, Fb, En, Hi>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum Afield<Ex, Fb, En, Hi> {
    AFvalue(Expr<Ex, Fb, En, Hi>),
    AFkvalue(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum XhpAttribute<Ex, Fb, En, Hi> {
    XhpSimple(Pstring, Expr<Ex, Fb, En, Hi>),
    XhpSpread(Expr<Ex, Fb, En, Hi>),
}

pub type IsVariadic = bool;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct FunParam<Ex, Fb, En, Hi> {
    pub annotation: Ex,
    pub type_hint: TypeHint<Hi>,
    pub is_variadic: IsVariadic,
    pub pos: Pos,
    pub name: String,
    pub expr: Option<Expr<Ex, Fb, En, Hi>>,
    pub callconv: Option<ast_defs::ParamKind>,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub visibility: Option<Visibility>,
}

/// does function take varying number of args?
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum FunVariadicity<Ex, Fb, En, Hi> {
    /// PHP5.6 ...$args finishes the func declaration
    FVvariadicArg(FunParam<Ex, Fb, En, Hi>),
    /// HH ... finishes the declaration; deprecate for ...$args?
    FVellipsis(Pos),
    /// standard non variadic function
    FVnonVariadic,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct Fun_<Ex, Fb, En, Hi> {
    pub span: Pos,
    pub annotation: En,
    pub mode: file_info::Mode,
    pub ret: TypeHint<Hi>,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, Fb, En, Hi>>,
    pub where_constraints: Vec<WhereConstraint>,
    pub variadic: FunVariadicity<Ex, Fb, En, Hi>,
    pub params: Vec<FunParam<Ex, Fb, En, Hi>>,
    pub body: FuncBody<Ex, Fb, En, Hi>,
    pub fun_kind: ast_defs::FunKind,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub file_attributes: Vec<FileAttribute<Ex, Fb, En, Hi>>,
    /// true if this declaration has no body because it is an
    /// external function declaration (e.g. from an HHI file)
    pub external: bool,
    pub namespace: Nsenv,
    pub doc_comment: Option<doc_comment::DocComment>,
    pub static_: bool,
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
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct FuncBody<Ex, Fb, En, Hi> {
    pub ast: Block<Ex, Fb, En, Hi>,
    pub annotation: Fb,
}

/// A type annotation is two things:
/// - the localized hint, or if the hint is missing, the inferred type
/// - The typehint associated to this expression if it exists
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct TypeHint<Hi>(pub Hi, pub TypeHint_);

/// Explicit type argument to function, constructor, or collection literal.
/// 'hi = unit in NAST
/// 'hi = Typing_defs.(locl ty) in TAST,
/// and is used to record inferred type arguments, with wildcard hint.
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct Targ<Hi>(pub Hi, pub Hint);

pub type TypeHint_ = Option<Hint>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct UserAttribute<Ex, Fb, En, Hi> {
    pub name: Sid,
    /// user attributes are restricted to scalar values
    pub params: Vec<Expr<Ex, Fb, En, Hi>>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct FileAttribute<Ex, Fb, En, Hi> {
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub namespace: Nsenv,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct Tparam<Ex, Fb, En, Hi> {
    pub variance: ast_defs::Variance,
    pub name: Sid,
    pub constraints: Vec<(ast_defs::ConstraintKind, Hint)>,
    pub reified: ReifyKind,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct ClassTparams<Ex, Fb, En, Hi> {
    pub list: Vec<Tparam<Ex, Fb, En, Hi>>,
    /// keeping around the ast version of the constraint only
    /// for the purposes of Naming.class_meth_bodies
    /// TODO: remove this and use tp_constraints
    pub constraints: s_map::SMap<(ReifyKind, Vec<(ast_defs::ConstraintKind, Hint)>)>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct UseAsAlias(
    pub Option<Sid>,
    pub Pstring,
    pub Option<Sid>,
    pub Vec<UseAsVisibility>,
);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct InsteadofAlias(pub Sid, pub Pstring, pub Vec<Sid>);

pub type IsExtends = bool;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum EmitId {
    EmitId(isize),
    Anonymous,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct Class_<Ex, Fb, En, Hi> {
    pub span: Pos,
    pub annotation: En,
    pub mode: file_info::Mode,
    pub final_: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub kind: ast_defs::ClassKind,
    pub name: Sid,
    /// The type parameters of a class A<T> (T is the parameter)
    pub tparams: ClassTparams<Ex, Fb, En, Hi>,
    pub extends: Vec<Hint>,
    pub uses: Vec<Hint>,
    pub use_as_alias: Vec<UseAsAlias>,
    pub insteadof_alias: Vec<InsteadofAlias>,
    pub method_redeclarations: Vec<MethodRedeclaration<Ex, Fb, En, Hi>>,
    pub xhp_attr_uses: Vec<Hint>,
    pub xhp_category: Option<(Pos, Vec<Pstring>)>,
    pub reqs: Vec<(Hint, IsExtends)>,
    pub implements: Vec<Hint>,
    pub where_constraints: Vec<WhereConstraint>,
    pub consts: Vec<ClassConst<Ex, Fb, En, Hi>>,
    pub typeconsts: Vec<ClassTypeconst<Ex, Fb, En, Hi>>,
    pub vars: Vec<ClassVar<Ex, Fb, En, Hi>>,
    pub methods: Vec<Method_<Ex, Fb, En, Hi>>,
    pub attributes: Vec<ClassAttr<Ex, Fb, En, Hi>>,
    pub xhp_children: Vec<(Pos, XhpChild)>,
    pub xhp_attrs: Vec<XhpAttr<Ex, Fb, En, Hi>>,
    pub namespace: Nsenv,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub file_attributes: Vec<FileAttribute<Ex, Fb, En, Hi>>,
    pub enum_: Option<Enum_>,
    pub pu_enums: Vec<PuEnum<Ex, Fb, En, Hi>>,
    pub doc_comment: Option<doc_comment::DocComment>,
    pub emit_id: Option<EmitId>,
}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum XhpAttrTag {
    Required,
    LateInit,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct XhpAttr<Ex, Fb, En, Hi>(
    pub TypeHint<Hi>,
    pub ClassVar<Ex, Fb, En, Hi>,
    pub Option<XhpAttrTag>,
    pub Option<(Pos, bool, Vec<Expr<Ex, Fb, En, Hi>>)>,
);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum ClassAttr<Ex, Fb, En, Hi> {
    CAName(Sid),
    CAField(CaField<Ex, Fb, En, Hi>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct CaField<Ex, Fb, En, Hi> {
    pub type_: CaType,
    pub id: Sid,
    pub value: Option<Expr<Ex, Fb, En, Hi>>,
    pub required: bool,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum CaType {
    CAHint(Hint),
    CAEnum(Vec<String>),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct ClassConst<Ex, Fb, En, Hi> {
    pub type_: Option<Hint>,
    pub id: Sid,
    /// expr = None indicates an abstract const
    pub expr: Option<Expr<Ex, Fb, En, Hi>>,
    pub doc_comment: Option<doc_comment::DocComment>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum TypeconstAbstractKind {
    TCAbstract(Option<Hint>),
    TCPartiallyAbstract,
    TCConcrete,
}

/// This represents a type const definition. If a type const is abstract then
/// then the type hint acts as a constraint. Any concrete definition of the
/// type const must satisfy the constraint.
///
/// If the type const is not abstract then a type must be specified.
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct ClassTypeconst<Ex, Fb, En, Hi> {
    pub abstract_: TypeconstAbstractKind,
    pub name: Sid,
    pub constraint: Option<Hint>,
    pub type_: Option<Hint>,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub span: Pos,
    pub doc_comment: Option<doc_comment::DocComment>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct XhpAttrInfo {
    pub xai_tag: Option<XhpAttrTag>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct ClassVar<Ex, Fb, En, Hi> {
    pub final_: bool,
    pub xhp_attr: Option<XhpAttrInfo>,
    pub abstract_: bool,
    pub visibility: Visibility,
    pub type_: TypeHint<Hi>,
    pub id: Sid,
    pub expr: Option<Expr<Ex, Fb, En, Hi>>,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub doc_comment: Option<doc_comment::DocComment>,
    pub is_promoted_variadic: bool,
    pub is_static: bool,
    pub span: Pos,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct Method_<Ex, Fb, En, Hi> {
    pub span: Pos,
    pub annotation: En,
    pub final_: bool,
    pub abstract_: bool,
    pub static_: bool,
    pub visibility: Visibility,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, Fb, En, Hi>>,
    pub where_constraints: Vec<WhereConstraint>,
    pub variadic: FunVariadicity<Ex, Fb, En, Hi>,
    pub params: Vec<FunParam<Ex, Fb, En, Hi>>,
    pub body: FuncBody<Ex, Fb, En, Hi>,
    pub fun_kind: ast_defs::FunKind,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub ret: TypeHint<Hi>,
    /// true if this declaration has no body because it is an external method
    /// declaration (e.g. from an HHI file)
    pub external: bool,
    pub doc_comment: Option<doc_comment::DocComment>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct MethodRedeclaration<Ex, Fb, En, Hi> {
    pub final_: bool,
    pub abstract_: bool,
    pub static_: bool,
    pub visibility: Visibility,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, Fb, En, Hi>>,
    pub where_constraints: Vec<WhereConstraint>,
    pub variadic: FunVariadicity<Ex, Fb, En, Hi>,
    pub params: Vec<FunParam<Ex, Fb, En, Hi>>,
    pub fun_kind: ast_defs::FunKind,
    pub ret: TypeHint<Hi>,
    pub trait_: Hint,
    pub method: Pstring,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
}

pub type Nsenv = ocamlrep::rc::RcOc<namespace_env::Env>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct Typedef<Ex, Fb, En, Hi> {
    pub annotation: En,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, Fb, En, Hi>>,
    pub constraint: Option<Hint>,
    pub kind: Hint,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub mode: file_info::Mode,
    pub vis: TypedefVisibility,
    pub namespace: Nsenv,
    pub emit_id: Option<EmitId>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct Gconst<Ex, Fb, En, Hi> {
    pub annotation: En,
    pub mode: file_info::Mode,
    pub name: Sid,
    pub type_: Option<Hint>,
    pub value: Expr<Ex, Fb, En, Hi>,
    pub namespace: Nsenv,
    pub span: Pos,
    pub emit_id: Option<EmitId>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct RecordDef<Ex, Fb, En, Hi> {
    pub annotation: En,
    pub name: Sid,
    pub extends: Option<Hint>,
    pub abstract_: bool,
    pub fields: Vec<(Sid, Hint, Option<Expr<Ex, Fb, En, Hi>>)>,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub namespace: Nsenv,
    pub span: Pos,
    pub doc_comment: Option<doc_comment::DocComment>,
    pub emit_id: Option<EmitId>,
}

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
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct PuEnum<Ex, Fb, En, Hi> {
    pub annotation: En,
    pub name: Sid,
    pub is_final: bool,
    pub case_types: Vec<(Sid, ReifyKind)>,
    pub case_values: Vec<(Sid, Hint)>,
    pub members: Vec<PuMember<Ex, Fb, En, Hi>>,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct PuMember<Ex, Fb, En, Hi> {
    pub atom: Sid,
    pub types: Vec<(Sid, Hint)>,
    pub exprs: Vec<(Sid, Expr<Ex, Fb, En, Hi>)>,
}

pub type FunDef<Ex, Fb, En, Hi> = Fun_<Ex, Fb, En, Hi>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum Def<Ex, Fb, En, Hi> {
    Fun(Box<FunDef<Ex, Fb, En, Hi>>),
    Class(Box<Class_<Ex, Fb, En, Hi>>),
    RecordDef(Box<RecordDef<Ex, Fb, En, Hi>>),
    Stmt(Box<Stmt<Ex, Fb, En, Hi>>),
    Typedef(Box<Typedef<Ex, Fb, En, Hi>>),
    Constant(Box<Gconst<Ex, Fb, En, Hi>>),
    Namespace(Box<(Sid, Program<Ex, Fb, En, Hi>)>),
    NamespaceUse(Vec<(NsKind, Sid, Sid)>),
    SetNamespaceEnv(Box<Nsenv>),
    FileAttributes(Box<FileAttribute<Ex, Fb, En, Hi>>),
}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum NsKind {
    NSNamespace,
    NSClass,
    NSClassAndNamespace,
    NSFun,
    NSConst,
}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum ReifyKind {
    Erased,
    SoftReified,
    Reified,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum BreakContinueLevel {
    LevelOk(Option<isize>),
    LevelNonLiteral,
    LevelNonPositive,
}

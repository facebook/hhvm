// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<73a4db64b471816cb3ee0660b5373376>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_rc/regen.sh

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
pub type Program<Ex, Fb, En, Hi> = Vec<Def<Ex, Fb, En, Hi>>;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Stmt<Ex, Fb, En, Hi>(pub std::rc::Rc<Pos>, pub Stmt_<Ex, Fb, En, Hi>);

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Stmt_<Ex, Fb, En, Hi> {
    Fallthrough,
    Expr(std::rc::Rc<Expr<Ex, Fb, En, Hi>>),
    Break,
    Continue,
    Throw(std::rc::Rc<Expr<Ex, Fb, En, Hi>>),
    Return(std::rc::Rc<Option<Expr<Ex, Fb, En, Hi>>>),
    GotoLabel(std::rc::Rc<Pstring>),
    Goto(std::rc::Rc<Pstring>),
    Awaitall(
        std::rc::Rc<(
            Vec<(Option<Lid>, Expr<Ex, Fb, En, Hi>)>,
            Block<Ex, Fb, En, Hi>,
        )>,
    ),
    If(
        std::rc::Rc<(
            Expr<Ex, Fb, En, Hi>,
            Block<Ex, Fb, En, Hi>,
            Block<Ex, Fb, En, Hi>,
        )>,
    ),
    Do(std::rc::Rc<(Block<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>),
    While(std::rc::Rc<(Expr<Ex, Fb, En, Hi>, Block<Ex, Fb, En, Hi>)>),
    Using(std::rc::Rc<UsingStmt<Ex, Fb, En, Hi>>),
    For(
        std::rc::Rc<(
            Expr<Ex, Fb, En, Hi>,
            Expr<Ex, Fb, En, Hi>,
            Expr<Ex, Fb, En, Hi>,
            Block<Ex, Fb, En, Hi>,
        )>,
    ),
    Switch(std::rc::Rc<(Expr<Ex, Fb, En, Hi>, Vec<Case<Ex, Fb, En, Hi>>)>),
    Foreach(
        std::rc::Rc<(
            Expr<Ex, Fb, En, Hi>,
            AsExpr<Ex, Fb, En, Hi>,
            Block<Ex, Fb, En, Hi>,
        )>,
    ),
    Try(
        std::rc::Rc<(
            Block<Ex, Fb, En, Hi>,
            Vec<Catch<Ex, Fb, En, Hi>>,
            Block<Ex, Fb, En, Hi>,
        )>,
    ),
    Noop,
    Block(Block<Ex, Fb, En, Hi>),
    Markup(std::rc::Rc<Pstring>),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct UsingStmt<Ex, Fb, En, Hi> {
    pub is_block_scoped: bool,
    pub has_await: bool,
    pub expr: Expr<Ex, Fb, En, Hi>,
    pub block: Block<Ex, Fb, En, Hi>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum AsExpr<Ex, Fb, En, Hi> {
    AsV(Expr<Ex, Fb, En, Hi>),
    AsKv(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>),
    AwaitAsV(std::rc::Rc<Pos>, Expr<Ex, Fb, En, Hi>),
    AwaitAsKv(std::rc::Rc<Pos>, Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>),
}

pub type Block<Ex, Fb, En, Hi> = Vec<Stmt<Ex, Fb, En, Hi>>;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ClassId<Ex, Fb, En, Hi>(pub Ex, pub ClassId_<Ex, Fb, En, Hi>);

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum ClassId_<Ex, Fb, En, Hi> {
    CIparent,
    CIself,
    CIstatic,
    CIexpr(Expr<Ex, Fb, En, Hi>),
    CI(Sid),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Expr<Ex, Fb, En, Hi>(pub Ex, pub Expr_<Ex, Fb, En, Hi>);

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum CollectionTarg<Hi> {
    CollectionTV(Targ<Hi>),
    CollectionTKV(Targ<Hi>, Targ<Hi>),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum FunctionPtrId<Ex, Fb, En, Hi> {
    FPId(Sid),
    FPClassConst(ClassId<Ex, Fb, En, Hi>, Pstring),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Expr_<Ex, Fb, En, Hi> {
    Darray(
        std::rc::Rc<(
            Option<(Targ<Hi>, Targ<Hi>)>,
            Vec<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>,
        )>,
    ),
    Varray(std::rc::Rc<(Option<Targ<Hi>>, Vec<Expr<Ex, Fb, En, Hi>>)>),
    Shape(Vec<(ast_defs::ShapeFieldName, Expr<Ex, Fb, En, Hi>)>),
    /// TODO: T38184446 Consolidate collections in AAST
    ValCollection(
        std::rc::Rc<(
            oxidized::aast::VcKind,
            Option<Targ<Hi>>,
            Vec<Expr<Ex, Fb, En, Hi>>,
        )>,
    ),
    /// TODO: T38184446 Consolidate collections in AAST
    KeyValCollection(
        std::rc::Rc<(
            oxidized::aast::KvcKind,
            Option<(Targ<Hi>, Targ<Hi>)>,
            Vec<Field<Ex, Fb, En, Hi>>,
        )>,
    ),
    Null,
    This,
    True,
    False,
    Omitted,
    Id(std::rc::Rc<Sid>),
    Lvar(std::rc::Rc<Lid>),
    Dollardollar(std::rc::Rc<Lid>),
    Clone(std::rc::Rc<Expr<Ex, Fb, En, Hi>>),
    ObjGet(
        std::rc::Rc<(
            Expr<Ex, Fb, En, Hi>,
            Expr<Ex, Fb, En, Hi>,
            oxidized::aast::OgNullFlavor,
        )>,
    ),
    ArrayGet(std::rc::Rc<(Expr<Ex, Fb, En, Hi>, Option<Expr<Ex, Fb, En, Hi>>)>),
    ClassGet(std::rc::Rc<(ClassId<Ex, Fb, En, Hi>, ClassGetExpr<Ex, Fb, En, Hi>)>),
    ClassConst(std::rc::Rc<(ClassId<Ex, Fb, En, Hi>, Pstring)>),
    Call(
        std::rc::Rc<(
            oxidized::aast::CallType,
            Expr<Ex, Fb, En, Hi>,
            Vec<Targ<Hi>>,
            Vec<Expr<Ex, Fb, En, Hi>>,
            Option<Expr<Ex, Fb, En, Hi>>,
        )>,
    ),
    FunctionPointer(std::rc::Rc<(FunctionPtrId<Ex, Fb, En, Hi>, Vec<Targ<Hi>>)>),
    Int(std::rc::Rc<std::rc::Rc<String>>),
    Float(std::rc::Rc<std::rc::Rc<String>>),
    String(std::rc::Rc<std::rc::Rc<String>>),
    String2(Vec<Expr<Ex, Fb, En, Hi>>),
    PrefixedString(std::rc::Rc<(std::rc::Rc<String>, Expr<Ex, Fb, En, Hi>)>),
    Yield(std::rc::Rc<Afield<Ex, Fb, En, Hi>>),
    YieldBreak,
    Await(std::rc::Rc<Expr<Ex, Fb, En, Hi>>),
    Suspend(std::rc::Rc<Expr<Ex, Fb, En, Hi>>),
    List(Vec<Expr<Ex, Fb, En, Hi>>),
    ExprList(Vec<Expr<Ex, Fb, En, Hi>>),
    Cast(std::rc::Rc<(Hint, Expr<Ex, Fb, En, Hi>)>),
    Unop(std::rc::Rc<(oxidized::ast_defs::Uop, Expr<Ex, Fb, En, Hi>)>),
    Binop(std::rc::Rc<(ast_defs::Bop, Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>),
    /// The lid is the ID of the $$ that is implicitly declared by this pipe.
    Pipe(std::rc::Rc<(Lid, Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>),
    Eif(
        std::rc::Rc<(
            Expr<Ex, Fb, En, Hi>,
            Option<Expr<Ex, Fb, En, Hi>>,
            Expr<Ex, Fb, En, Hi>,
        )>,
    ),
    Is(std::rc::Rc<(Expr<Ex, Fb, En, Hi>, Hint)>),
    As(std::rc::Rc<(Expr<Ex, Fb, En, Hi>, Hint, bool)>),
    New(
        std::rc::Rc<(
            ClassId<Ex, Fb, En, Hi>,
            Vec<Targ<Hi>>,
            Vec<Expr<Ex, Fb, En, Hi>>,
            Option<Expr<Ex, Fb, En, Hi>>,
            Ex,
        )>,
    ),
    Record(std::rc::Rc<(Sid, Vec<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>)>),
    Efun(std::rc::Rc<(Fun_<Ex, Fb, En, Hi>, Vec<Lid>)>),
    Lfun(std::rc::Rc<(Fun_<Ex, Fb, En, Hi>, Vec<Lid>)>),
    Xml(
        std::rc::Rc<(
            Sid,
            Vec<XhpAttribute<Ex, Fb, En, Hi>>,
            Vec<Expr<Ex, Fb, En, Hi>>,
        )>,
    ),
    Callconv(std::rc::Rc<(oxidized::ast_defs::ParamKind, Expr<Ex, Fb, En, Hi>)>),
    Import(std::rc::Rc<(oxidized::aast::ImportFlavor, Expr<Ex, Fb, En, Hi>)>),
    /// TODO: T38184446 Consolidate collections in AAST
    Collection(std::rc::Rc<(Sid, Option<CollectionTarg<Hi>>, Vec<Afield<Ex, Fb, En, Hi>>)>),
    BracedExpr(std::rc::Rc<Expr<Ex, Fb, En, Hi>>),
    ParenthesizedExpr(std::rc::Rc<Expr<Ex, Fb, En, Hi>>),
    Lplaceholder(std::rc::Rc<std::rc::Rc<Pos>>),
    FunId(std::rc::Rc<Sid>),
    MethodId(std::rc::Rc<(Expr<Ex, Fb, En, Hi>, Pstring)>),
    /// meth_caller('Class name', 'method name')
    MethodCaller(std::rc::Rc<(Sid, Pstring)>),
    SmethodId(std::rc::Rc<(Sid, Pstring)>),
    Pair(
        std::rc::Rc<(
            Option<(Targ<Hi>, Targ<Hi>)>,
            Expr<Ex, Fb, En, Hi>,
            Expr<Ex, Fb, En, Hi>,
        )>,
    ),
    Assert(std::rc::Rc<AssertExpr<Ex, Fb, En, Hi>>),
    PUAtom(std::rc::Rc<std::rc::Rc<String>>),
    PUIdentifier(std::rc::Rc<(ClassId<Ex, Fb, En, Hi>, Pstring, Pstring)>),
    Any,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum ClassGetExpr<Ex, Fb, En, Hi> {
    CGstring(Pstring),
    CGexpr(Expr<Ex, Fb, En, Hi>),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum AssertExpr<Ex, Fb, En, Hi> {
    AEAssert(Expr<Ex, Fb, En, Hi>),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Case<Ex, Fb, En, Hi> {
    Default(std::rc::Rc<Pos>, Block<Ex, Fb, En, Hi>),
    Case(Expr<Ex, Fb, En, Hi>, Block<Ex, Fb, En, Hi>),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Catch<Ex, Fb, En, Hi>(pub Sid, pub Lid, pub Block<Ex, Fb, En, Hi>);

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Field<Ex, Fb, En, Hi>(pub Expr<Ex, Fb, En, Hi>, pub Expr<Ex, Fb, En, Hi>);

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Afield<Ex, Fb, En, Hi> {
    AFvalue(Expr<Ex, Fb, En, Hi>),
    AFkvalue(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum XhpAttribute<Ex, Fb, En, Hi> {
    XhpSimple(Pstring, Expr<Ex, Fb, En, Hi>),
    XhpSpread(Expr<Ex, Fb, En, Hi>),
}

pub use oxidized::aast::IsVariadic;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct FunParam<Ex, Fb, En, Hi> {
    pub annotation: Ex,
    pub type_hint: TypeHint<Hi>,
    pub is_variadic: oxidized::aast::IsVariadic,
    pub pos: std::rc::Rc<Pos>,
    pub name: std::rc::Rc<String>,
    pub expr: Option<Expr<Ex, Fb, En, Hi>>,
    pub callconv: Option<oxidized::ast_defs::ParamKind>,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub visibility: Option<oxidized::aast::Visibility>,
}

/// does function take varying number of args?
#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum FunVariadicity<Ex, Fb, En, Hi> {
    /// PHP5.6 ...$args finishes the func declaration
    FVvariadicArg(FunParam<Ex, Fb, En, Hi>),
    /// HH ... finishes the declaration; deprecate for ...$args?
    FVellipsis(std::rc::Rc<Pos>),
    /// standard non variadic function
    FVnonVariadic,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Fun_<Ex, Fb, En, Hi> {
    pub span: std::rc::Rc<Pos>,
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    pub ret: TypeHint<Hi>,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, Fb, En, Hi>>,
    pub where_constraints: Vec<WhereConstraint>,
    pub variadic: FunVariadicity<Ex, Fb, En, Hi>,
    pub params: Vec<FunParam<Ex, Fb, En, Hi>>,
    pub cap: TypeHint<Hi>,
    pub body: FuncBody<Ex, Fb, En, Hi>,
    pub fun_kind: oxidized::ast_defs::FunKind,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub file_attributes: Vec<FileAttribute<Ex, Fb, En, Hi>>,
    /// true if this declaration has no body because it is an
    /// external function declaration (e.g. from an HHI file)
    pub external: bool,
    pub namespace: Nsenv,
    pub doc_comment: Option<DocComment>,
    pub static_: bool,
}

/// Naming has two phases and the annotation helps to indicate the phase.
/// In the first pass, it will perform naming on everything except for function
/// and method bodies and collect information needed. Then, another round of
/// naming is performed where function bodies are named. Thus, naming will
/// have named and unnamed variants of the annotation.
/// See BodyNamingAnnotation in nast.ml and the comment in naming.ml
#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct FuncBody<Ex, Fb, En, Hi> {
    pub ast: Block<Ex, Fb, En, Hi>,
    pub annotation: Fb,
}

/// A type annotation is two things:
/// - the localized hint, or if the hint is missing, the inferred type
/// - The typehint associated to this expression if it exists
#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct TypeHint<Hi>(pub Hi, pub TypeHint_);

/// Explicit type argument to function, constructor, or collection literal.
/// 'hi = unit in NAST
/// 'hi = Typing_defs.(locl ty) in TAST,
/// and is used to record inferred type arguments, with wildcard hint.
#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Targ<Hi>(pub Hi, pub Hint);

pub type TypeHint_ = Option<Hint>;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct UserAttribute<Ex, Fb, En, Hi> {
    pub name: Sid,
    /// user attributes are restricted to scalar values
    pub params: Vec<Expr<Ex, Fb, En, Hi>>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct FileAttribute<Ex, Fb, En, Hi> {
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub namespace: Nsenv,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Tparam<Ex, Fb, En, Hi> {
    pub variance: oxidized::ast_defs::Variance,
    pub name: Sid,
    pub parameters: Vec<Tparam<Ex, Fb, En, Hi>>,
    pub constraints: Vec<(oxidized::ast_defs::ConstraintKind, Hint)>,
    pub reified: oxidized::aast::ReifyKind,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ClassTparams<Ex, Fb, En, Hi> {
    pub list: Vec<Tparam<Ex, Fb, En, Hi>>,
    /// keeping around the ast version of the constraint only
    /// for the purposes of Naming.class_meth_bodies
    /// TODO: remove this and use tp_constraints
    pub constraints: s_map::SMap<(
        oxidized::aast::ReifyKind,
        Vec<(oxidized::ast_defs::ConstraintKind, Hint)>,
    )>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct UseAsAlias(
    pub Option<Sid>,
    pub Pstring,
    pub Option<Sid>,
    pub Vec<oxidized::aast::UseAsVisibility>,
);

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct InsteadofAlias(pub Sid, pub Pstring, pub Vec<Sid>);

pub use oxidized::aast::IsExtends;

pub use oxidized::aast::EmitId;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Class_<Ex, Fb, En, Hi> {
    pub span: std::rc::Rc<Pos>,
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    pub final_: bool,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub kind: oxidized::ast_defs::ClassKind,
    pub name: Sid,
    /// The type parameters of a class A<T> (T is the parameter)
    pub tparams: ClassTparams<Ex, Fb, En, Hi>,
    pub extends: Vec<ClassHint>,
    pub uses: Vec<TraitHint>,
    pub use_as_alias: Vec<UseAsAlias>,
    pub insteadof_alias: Vec<InsteadofAlias>,
    pub method_redeclarations: Vec<MethodRedeclaration<Ex, Fb, En, Hi>>,
    pub xhp_attr_uses: Vec<XhpAttrHint>,
    pub xhp_category: Option<(std::rc::Rc<Pos>, Vec<Pstring>)>,
    pub reqs: Vec<(ClassHint, oxidized::aast::IsExtends)>,
    pub implements: Vec<ClassHint>,
    pub where_constraints: Vec<WhereConstraint>,
    pub consts: Vec<ClassConst<Ex, Fb, En, Hi>>,
    pub typeconsts: Vec<ClassTypeconst<Ex, Fb, En, Hi>>,
    pub vars: Vec<ClassVar<Ex, Fb, En, Hi>>,
    pub methods: Vec<Method_<Ex, Fb, En, Hi>>,
    pub attributes: Vec<ClassAttr<Ex, Fb, En, Hi>>,
    pub xhp_children: Vec<(std::rc::Rc<Pos>, XhpChild)>,
    pub xhp_attrs: Vec<XhpAttr<Ex, Fb, En, Hi>>,
    pub namespace: Nsenv,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub file_attributes: Vec<FileAttribute<Ex, Fb, En, Hi>>,
    pub enum_: Option<Enum_>,
    pub pu_enums: Vec<PuEnum<Ex, Fb, En, Hi>>,
    pub doc_comment: Option<DocComment>,
    pub emit_id: Option<oxidized::aast::EmitId>,
}

pub type ClassHint = Hint;

pub type TraitHint = Hint;

pub type XhpAttrHint = Hint;

pub use oxidized::aast::XhpAttrTag;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct XhpAttr<Ex, Fb, En, Hi>(
    pub TypeHint<Hi>,
    pub ClassVar<Ex, Fb, En, Hi>,
    pub Option<oxidized::aast::XhpAttrTag>,
    pub Option<(std::rc::Rc<Pos>, bool, Vec<Expr<Ex, Fb, En, Hi>>)>,
);

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum ClassAttr<Ex, Fb, En, Hi> {
    CAName(Sid),
    CAField(CaField<Ex, Fb, En, Hi>),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct CaField<Ex, Fb, En, Hi> {
    pub type_: CaType,
    pub id: Sid,
    pub value: Option<Expr<Ex, Fb, En, Hi>>,
    pub required: bool,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum CaType {
    CAHint(Hint),
    CAEnum(Vec<std::rc::Rc<String>>),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ClassConst<Ex, Fb, En, Hi> {
    pub type_: Option<Hint>,
    pub id: Sid,
    /// expr = None indicates an abstract const
    pub expr: Option<Expr<Ex, Fb, En, Hi>>,
    pub doc_comment: Option<DocComment>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
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
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ClassTypeconst<Ex, Fb, En, Hi> {
    pub abstract_: TypeconstAbstractKind,
    pub name: Sid,
    pub constraint: Option<Hint>,
    pub type_: Option<Hint>,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub span: std::rc::Rc<Pos>,
    pub doc_comment: Option<DocComment>,
}

pub use oxidized::aast::XhpAttrInfo;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ClassVar<Ex, Fb, En, Hi> {
    pub final_: bool,
    pub xhp_attr: Option<oxidized::aast::XhpAttrInfo>,
    pub abstract_: bool,
    pub visibility: oxidized::aast::Visibility,
    pub type_: TypeHint<Hi>,
    pub id: Sid,
    pub expr: Option<Expr<Ex, Fb, En, Hi>>,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub doc_comment: Option<DocComment>,
    pub is_promoted_variadic: bool,
    pub is_static: bool,
    pub span: std::rc::Rc<Pos>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Method_<Ex, Fb, En, Hi> {
    pub span: std::rc::Rc<Pos>,
    pub annotation: En,
    pub final_: bool,
    pub abstract_: bool,
    pub static_: bool,
    pub visibility: oxidized::aast::Visibility,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, Fb, En, Hi>>,
    pub where_constraints: Vec<WhereConstraint>,
    pub variadic: FunVariadicity<Ex, Fb, En, Hi>,
    pub params: Vec<FunParam<Ex, Fb, En, Hi>>,
    pub cap: TypeHint<Hi>,
    pub body: FuncBody<Ex, Fb, En, Hi>,
    pub fun_kind: oxidized::ast_defs::FunKind,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub ret: TypeHint<Hi>,
    /// true if this declaration has no body because it is an external method
    /// declaration (e.g. from an HHI file)
    pub external: bool,
    pub doc_comment: Option<DocComment>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct MethodRedeclaration<Ex, Fb, En, Hi> {
    pub final_: bool,
    pub abstract_: bool,
    pub static_: bool,
    pub visibility: oxidized::aast::Visibility,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, Fb, En, Hi>>,
    pub where_constraints: Vec<WhereConstraint>,
    pub variadic: FunVariadicity<Ex, Fb, En, Hi>,
    pub params: Vec<FunParam<Ex, Fb, En, Hi>>,
    pub fun_kind: oxidized::ast_defs::FunKind,
    pub ret: TypeHint<Hi>,
    pub trait_: TraitHint,
    pub method: Pstring,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
}

pub type Nsenv = ocamlrep::rc::RcOc<namespace_env::Env>;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Typedef<Ex, Fb, En, Hi> {
    pub annotation: En,
    pub name: Sid,
    pub tparams: Vec<Tparam<Ex, Fb, En, Hi>>,
    pub constraint: Option<Hint>,
    pub kind: Hint,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub mode: oxidized::file_info::Mode,
    pub vis: oxidized::aast::TypedefVisibility,
    pub namespace: Nsenv,
    pub span: std::rc::Rc<Pos>,
    pub emit_id: Option<oxidized::aast::EmitId>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Gconst<Ex, Fb, En, Hi> {
    pub annotation: En,
    pub mode: oxidized::file_info::Mode,
    pub name: Sid,
    pub type_: Option<Hint>,
    pub value: Expr<Ex, Fb, En, Hi>,
    pub namespace: Nsenv,
    pub span: std::rc::Rc<Pos>,
    pub emit_id: Option<oxidized::aast::EmitId>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct RecordDef<Ex, Fb, En, Hi> {
    pub annotation: En,
    pub name: Sid,
    pub extends: Option<RecordHint>,
    pub abstract_: bool,
    pub fields: Vec<(Sid, Hint, Option<Expr<Ex, Fb, En, Hi>>)>,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub namespace: Nsenv,
    pub span: std::rc::Rc<Pos>,
    pub doc_comment: Option<DocComment>,
    pub emit_id: Option<oxidized::aast::EmitId>,
}

pub type RecordHint = Hint;

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
pub struct PuEnum<Ex, Fb, En, Hi> {
    pub annotation: En,
    pub name: Sid,
    pub user_attributes: Vec<UserAttribute<Ex, Fb, En, Hi>>,
    pub is_final: bool,
    pub case_types: Vec<Tparam<Ex, Fb, En, Hi>>,
    pub case_values: Vec<PuCaseValue>,
    pub members: Vec<PuMember<Ex, Fb, En, Hi>>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct PuCaseValue(pub Sid, pub Hint);

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct PuMember<Ex, Fb, En, Hi> {
    pub atom: Sid,
    pub types: Vec<(Sid, Hint)>,
    pub exprs: Vec<(Sid, Expr<Ex, Fb, En, Hi>)>,
}

pub type FunDef<Ex, Fb, En, Hi> = Fun_<Ex, Fb, En, Hi>;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Def<Ex, Fb, En, Hi> {
    Fun(std::rc::Rc<FunDef<Ex, Fb, En, Hi>>),
    Class(std::rc::Rc<Class_<Ex, Fb, En, Hi>>),
    RecordDef(std::rc::Rc<RecordDef<Ex, Fb, En, Hi>>),
    Stmt(std::rc::Rc<Stmt<Ex, Fb, En, Hi>>),
    Typedef(std::rc::Rc<Typedef<Ex, Fb, En, Hi>>),
    Constant(std::rc::Rc<Gconst<Ex, Fb, En, Hi>>),
    Namespace(std::rc::Rc<(Sid, Program<Ex, Fb, En, Hi>)>),
    NamespaceUse(Vec<(oxidized::aast::NsKind, Sid, Sid)>),
    SetNamespaceEnv(std::rc::Rc<Nsenv>),
    FileAttributes(std::rc::Rc<FileAttribute<Ex, Fb, En, Hi>>),
}

pub use oxidized::aast::NsKind;

pub use oxidized::aast::ReifyKind;

pub use oxidized::aast::BreakContinueLevel;

// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<8c9301ca73460f3d91119a552097257e>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use crate::aast::*;
use crate::ast_defs;
impl<Ex, Fb, En, Hi> Stmt_<Ex, Fb, En, Hi> {
    pub fn mk_fallthrough() -> Self {
        Stmt_::Fallthrough
    }
    pub fn mk_expr(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Stmt_::Expr(Box::new(p0))
    }
    pub fn mk_break() -> Self {
        Stmt_::Break
    }
    pub fn mk_continue() -> Self {
        Stmt_::Continue
    }
    pub fn mk_throw(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Stmt_::Throw(Box::new(p0))
    }
    pub fn mk_return(p0: Option<Expr<Ex, Fb, En, Hi>>) -> Self {
        Stmt_::Return(Box::new(p0))
    }
    pub fn mk_goto_label(p0: Pstring) -> Self {
        Stmt_::GotoLabel(Box::new(p0))
    }
    pub fn mk_goto(p0: Pstring) -> Self {
        Stmt_::Goto(Box::new(p0))
    }
    pub fn mk_awaitall(
        p0: Vec<(Option<Lid>, Expr<Ex, Fb, En, Hi>)>,
        p1: Block<Ex, Fb, En, Hi>,
    ) -> Self {
        Stmt_::Awaitall(Box::new((p0, p1)))
    }
    pub fn mk_if(
        p0: Expr<Ex, Fb, En, Hi>,
        p1: Block<Ex, Fb, En, Hi>,
        p2: Block<Ex, Fb, En, Hi>,
    ) -> Self {
        Stmt_::If(Box::new((p0, p1, p2)))
    }
    pub fn mk_do(p0: Block<Ex, Fb, En, Hi>, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        Stmt_::Do(Box::new((p0, p1)))
    }
    pub fn mk_while(p0: Expr<Ex, Fb, En, Hi>, p1: Block<Ex, Fb, En, Hi>) -> Self {
        Stmt_::While(Box::new((p0, p1)))
    }
    pub fn mk_using(p0: UsingStmt<Ex, Fb, En, Hi>) -> Self {
        Stmt_::Using(Box::new(p0))
    }
    pub fn mk_for(
        p0: Expr<Ex, Fb, En, Hi>,
        p1: Expr<Ex, Fb, En, Hi>,
        p2: Expr<Ex, Fb, En, Hi>,
        p3: Block<Ex, Fb, En, Hi>,
    ) -> Self {
        Stmt_::For(Box::new((p0, p1, p2, p3)))
    }
    pub fn mk_switch(p0: Expr<Ex, Fb, En, Hi>, p1: Vec<Case<Ex, Fb, En, Hi>>) -> Self {
        Stmt_::Switch(Box::new((p0, p1)))
    }
    pub fn mk_foreach(
        p0: Expr<Ex, Fb, En, Hi>,
        p1: AsExpr<Ex, Fb, En, Hi>,
        p2: Block<Ex, Fb, En, Hi>,
    ) -> Self {
        Stmt_::Foreach(Box::new((p0, p1, p2)))
    }
    pub fn mk_try(
        p0: Block<Ex, Fb, En, Hi>,
        p1: Vec<Catch<Ex, Fb, En, Hi>>,
        p2: Block<Ex, Fb, En, Hi>,
    ) -> Self {
        Stmt_::Try(Box::new((p0, p1, p2)))
    }
    pub fn mk_def_inline(p0: Def<Ex, Fb, En, Hi>) -> Self {
        Stmt_::DefInline(Box::new(p0))
    }
    pub fn mk_let(p0: Lid, p1: Option<Hint>, p2: Expr<Ex, Fb, En, Hi>) -> Self {
        Stmt_::Let(Box::new((p0, p1, p2)))
    }
    pub fn mk_noop() -> Self {
        Stmt_::Noop
    }
    pub fn mk_block(p0: Block<Ex, Fb, En, Hi>) -> Self {
        Stmt_::Block(p0)
    }
    pub fn mk_markup(p0: Pstring, p1: Option<Expr<Ex, Fb, En, Hi>>) -> Self {
        Stmt_::Markup(Box::new((p0, p1)))
    }
}
impl<Ex, Fb, En, Hi> AsExpr<Ex, Fb, En, Hi> {
    pub fn mk_as_v(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        AsExpr::AsV(p0)
    }
    pub fn mk_as_kv(p0: Expr<Ex, Fb, En, Hi>, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        AsExpr::AsKv(p0, p1)
    }
    pub fn mk_await_as_v(p0: Pos, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        AsExpr::AwaitAsV(p0, p1)
    }
    pub fn mk_await_as_kv(p0: Pos, p1: Expr<Ex, Fb, En, Hi>, p2: Expr<Ex, Fb, En, Hi>) -> Self {
        AsExpr::AwaitAsKv(p0, p1, p2)
    }
}
impl<Ex, Fb, En, Hi> ClassId_<Ex, Fb, En, Hi> {
    pub fn mk_ciparent() -> Self {
        ClassId_::CIparent
    }
    pub fn mk_ciself() -> Self {
        ClassId_::CIself
    }
    pub fn mk_cistatic() -> Self {
        ClassId_::CIstatic
    }
    pub fn mk_ciexpr(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        ClassId_::CIexpr(p0)
    }
    pub fn mk_ci(p0: Sid) -> Self {
        ClassId_::CI(p0)
    }
}
impl<Hi> CollectionTarg<Hi> {
    pub fn mk_collectiontv(p0: Targ<Hi>) -> Self {
        CollectionTarg::CollectionTV(p0)
    }
    pub fn mk_collectiontkv(p0: Targ<Hi>, p1: Targ<Hi>) -> Self {
        CollectionTarg::CollectionTKV(p0, p1)
    }
}
impl<Ex, Fb, En, Hi> Expr_<Ex, Fb, En, Hi> {
    pub fn mk_array(p0: Vec<Afield<Ex, Fb, En, Hi>>) -> Self {
        Expr_::Array(p0)
    }
    pub fn mk_darray(
        p0: Option<(Targ<Hi>, Targ<Hi>)>,
        p1: Vec<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>,
    ) -> Self {
        Expr_::Darray(Box::new((p0, p1)))
    }
    pub fn mk_varray(p0: Option<Targ<Hi>>, p1: Vec<Expr<Ex, Fb, En, Hi>>) -> Self {
        Expr_::Varray(Box::new((p0, p1)))
    }
    pub fn mk_shape(p0: Vec<(ast_defs::ShapeFieldName, Expr<Ex, Fb, En, Hi>)>) -> Self {
        Expr_::Shape(p0)
    }
    pub fn mk_val_collection(
        p0: VcKind,
        p1: Option<Targ<Hi>>,
        p2: Vec<Expr<Ex, Fb, En, Hi>>,
    ) -> Self {
        Expr_::ValCollection(Box::new((p0, p1, p2)))
    }
    pub fn mk_key_val_collection(
        p0: KvcKind,
        p1: Option<(Targ<Hi>, Targ<Hi>)>,
        p2: Vec<Field<Ex, Fb, En, Hi>>,
    ) -> Self {
        Expr_::KeyValCollection(Box::new((p0, p1, p2)))
    }
    pub fn mk_null() -> Self {
        Expr_::Null
    }
    pub fn mk_this() -> Self {
        Expr_::This
    }
    pub fn mk_true() -> Self {
        Expr_::True
    }
    pub fn mk_false() -> Self {
        Expr_::False
    }
    pub fn mk_omitted() -> Self {
        Expr_::Omitted
    }
    pub fn mk_id(p0: Sid) -> Self {
        Expr_::Id(Box::new(p0))
    }
    pub fn mk_lvar(p0: Lid) -> Self {
        Expr_::Lvar(Box::new(p0))
    }
    pub fn mk_immutable_var(p0: Lid) -> Self {
        Expr_::ImmutableVar(Box::new(p0))
    }
    pub fn mk_dollardollar(p0: Lid) -> Self {
        Expr_::Dollardollar(Box::new(p0))
    }
    pub fn mk_clone(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Clone(Box::new(p0))
    }
    pub fn mk_obj_get(
        p0: Expr<Ex, Fb, En, Hi>,
        p1: Expr<Ex, Fb, En, Hi>,
        p2: OgNullFlavor,
    ) -> Self {
        Expr_::ObjGet(Box::new((p0, p1, p2)))
    }
    pub fn mk_array_get(p0: Expr<Ex, Fb, En, Hi>, p1: Option<Expr<Ex, Fb, En, Hi>>) -> Self {
        Expr_::ArrayGet(Box::new((p0, p1)))
    }
    pub fn mk_class_get(p0: ClassId<Ex, Fb, En, Hi>, p1: ClassGetExpr<Ex, Fb, En, Hi>) -> Self {
        Expr_::ClassGet(Box::new((p0, p1)))
    }
    pub fn mk_class_const(p0: ClassId<Ex, Fb, En, Hi>, p1: Pstring) -> Self {
        Expr_::ClassConst(Box::new((p0, p1)))
    }
    pub fn mk_call(
        p0: CallType,
        p1: Expr<Ex, Fb, En, Hi>,
        p2: Vec<Targ<Hi>>,
        p3: Vec<Expr<Ex, Fb, En, Hi>>,
        p4: Vec<Expr<Ex, Fb, En, Hi>>,
    ) -> Self {
        Expr_::Call(Box::new((p0, p1, p2, p3, p4)))
    }
    pub fn mk_int(p0: String) -> Self {
        Expr_::Int(p0)
    }
    pub fn mk_float(p0: String) -> Self {
        Expr_::Float(p0)
    }
    pub fn mk_string(p0: String) -> Self {
        Expr_::String(p0)
    }
    pub fn mk_string2(p0: Vec<Expr<Ex, Fb, En, Hi>>) -> Self {
        Expr_::String2(p0)
    }
    pub fn mk_prefixed_string(p0: String, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::PrefixedString(Box::new((p0, p1)))
    }
    pub fn mk_yield(p0: Afield<Ex, Fb, En, Hi>) -> Self {
        Expr_::Yield(Box::new(p0))
    }
    pub fn mk_yield_break() -> Self {
        Expr_::YieldBreak
    }
    pub fn mk_yield_from(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::YieldFrom(Box::new(p0))
    }
    pub fn mk_await(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Await(Box::new(p0))
    }
    pub fn mk_suspend(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Suspend(Box::new(p0))
    }
    pub fn mk_list(p0: Vec<Expr<Ex, Fb, En, Hi>>) -> Self {
        Expr_::List(p0)
    }
    pub fn mk_expr_list(p0: Vec<Expr<Ex, Fb, En, Hi>>) -> Self {
        Expr_::ExprList(p0)
    }
    pub fn mk_cast(p0: Hint, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Cast(Box::new((p0, p1)))
    }
    pub fn mk_unop(p0: ast_defs::Uop, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Unop(Box::new((p0, p1)))
    }
    pub fn mk_binop(p0: ast_defs::Bop, p1: Expr<Ex, Fb, En, Hi>, p2: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Binop(Box::new((p0, p1, p2)))
    }
    pub fn mk_pipe(p0: Lid, p1: Expr<Ex, Fb, En, Hi>, p2: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Pipe(Box::new((p0, p1, p2)))
    }
    pub fn mk_eif(
        p0: Expr<Ex, Fb, En, Hi>,
        p1: Option<Expr<Ex, Fb, En, Hi>>,
        p2: Expr<Ex, Fb, En, Hi>,
    ) -> Self {
        Expr_::Eif(Box::new((p0, p1, p2)))
    }
    pub fn mk_is(p0: Expr<Ex, Fb, En, Hi>, p1: Hint) -> Self {
        Expr_::Is(Box::new((p0, p1)))
    }
    pub fn mk_as(p0: Expr<Ex, Fb, En, Hi>, p1: Hint, p2: bool) -> Self {
        Expr_::As(Box::new((p0, p1, p2)))
    }
    pub fn mk_new(
        p0: ClassId<Ex, Fb, En, Hi>,
        p1: Vec<Targ<Hi>>,
        p2: Vec<Expr<Ex, Fb, En, Hi>>,
        p3: Vec<Expr<Ex, Fb, En, Hi>>,
        p4: Ex,
    ) -> Self {
        Expr_::New(Box::new((p0, p1, p2, p3, p4)))
    }
    pub fn mk_record(
        p0: ClassId<Ex, Fb, En, Hi>,
        p1: bool,
        p2: Vec<(Expr<Ex, Fb, En, Hi>, Expr<Ex, Fb, En, Hi>)>,
    ) -> Self {
        Expr_::Record(Box::new((p0, p1, p2)))
    }
    pub fn mk_efun(p0: Fun_<Ex, Fb, En, Hi>, p1: Vec<Lid>) -> Self {
        Expr_::Efun(Box::new((p0, p1)))
    }
    pub fn mk_lfun(p0: Fun_<Ex, Fb, En, Hi>, p1: Vec<Lid>) -> Self {
        Expr_::Lfun(Box::new((p0, p1)))
    }
    pub fn mk_xml(
        p0: Sid,
        p1: Vec<XhpAttribute<Ex, Fb, En, Hi>>,
        p2: Vec<Expr<Ex, Fb, En, Hi>>,
    ) -> Self {
        Expr_::Xml(Box::new((p0, p1, p2)))
    }
    pub fn mk_callconv(p0: ast_defs::ParamKind, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Callconv(Box::new((p0, p1)))
    }
    pub fn mk_import(p0: ImportFlavor, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Import(Box::new((p0, p1)))
    }
    pub fn mk_collection(
        p0: Sid,
        p1: Option<CollectionTarg<Hi>>,
        p2: Vec<Afield<Ex, Fb, En, Hi>>,
    ) -> Self {
        Expr_::Collection(Box::new((p0, p1, p2)))
    }
    pub fn mk_braced_expr(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::BracedExpr(Box::new(p0))
    }
    pub fn mk_parenthesized_expr(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::ParenthesizedExpr(Box::new(p0))
    }
    pub fn mk_lplaceholder(p0: Pos) -> Self {
        Expr_::Lplaceholder(Box::new(p0))
    }
    pub fn mk_fun_id(p0: Sid) -> Self {
        Expr_::FunId(Box::new(p0))
    }
    pub fn mk_method_id(p0: Expr<Ex, Fb, En, Hi>, p1: Pstring) -> Self {
        Expr_::MethodId(Box::new((p0, p1)))
    }
    pub fn mk_method_caller(p0: Sid, p1: Pstring) -> Self {
        Expr_::MethodCaller(Box::new((p0, p1)))
    }
    pub fn mk_smethod_id(p0: Sid, p1: Pstring) -> Self {
        Expr_::SmethodId(Box::new((p0, p1)))
    }
    pub fn mk_special_func(p0: SpecialFunc<Ex, Fb, En, Hi>) -> Self {
        Expr_::SpecialFunc(Box::new(p0))
    }
    pub fn mk_pair(p0: Expr<Ex, Fb, En, Hi>, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Pair(Box::new((p0, p1)))
    }
    pub fn mk_assert(p0: AssertExpr<Ex, Fb, En, Hi>) -> Self {
        Expr_::Assert(Box::new(p0))
    }
    pub fn mk_typename(p0: Sid) -> Self {
        Expr_::Typename(Box::new(p0))
    }
    pub fn mk_puatom(p0: String) -> Self {
        Expr_::PUAtom(p0)
    }
    pub fn mk_puidentifier(p0: ClassId<Ex, Fb, En, Hi>, p1: Pstring, p2: Pstring) -> Self {
        Expr_::PUIdentifier(Box::new((p0, p1, p2)))
    }
    pub fn mk_any() -> Self {
        Expr_::Any
    }
}
impl<Ex, Fb, En, Hi> ClassGetExpr<Ex, Fb, En, Hi> {
    pub fn mk_cgstring(p0: Pstring) -> Self {
        ClassGetExpr::CGstring(p0)
    }
    pub fn mk_cgexpr(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        ClassGetExpr::CGexpr(p0)
    }
}
impl<Ex, Fb, En, Hi> AssertExpr<Ex, Fb, En, Hi> {
    pub fn mk_aeassert(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        AssertExpr::AEAssert(p0)
    }
}
impl<Ex, Fb, En, Hi> Case<Ex, Fb, En, Hi> {
    pub fn mk_default(p0: Pos, p1: Block<Ex, Fb, En, Hi>) -> Self {
        Case::Default(p0, p1)
    }
    pub fn mk_case(p0: Expr<Ex, Fb, En, Hi>, p1: Block<Ex, Fb, En, Hi>) -> Self {
        Case::Case(p0, p1)
    }
}
impl<Ex, Fb, En, Hi> Afield<Ex, Fb, En, Hi> {
    pub fn mk_afvalue(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        Afield::AFvalue(p0)
    }
    pub fn mk_afkvalue(p0: Expr<Ex, Fb, En, Hi>, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        Afield::AFkvalue(p0, p1)
    }
}
impl<Ex, Fb, En, Hi> XhpAttribute<Ex, Fb, En, Hi> {
    pub fn mk_xhp_simple(p0: Pstring, p1: Expr<Ex, Fb, En, Hi>) -> Self {
        XhpAttribute::XhpSimple(p0, p1)
    }
    pub fn mk_xhp_spread(p0: Expr<Ex, Fb, En, Hi>) -> Self {
        XhpAttribute::XhpSpread(p0)
    }
}
impl<Ex, Fb, En, Hi> SpecialFunc<Ex, Fb, En, Hi> {
    pub fn mk_genva(p0: Vec<Expr<Ex, Fb, En, Hi>>) -> Self {
        SpecialFunc::Genva(p0)
    }
}
impl<Ex, Fb, En, Hi> FunVariadicity<Ex, Fb, En, Hi> {
    pub fn mk_fvvariadic_arg(p0: FunParam<Ex, Fb, En, Hi>) -> Self {
        FunVariadicity::FVvariadicArg(p0)
    }
    pub fn mk_fvellipsis(p0: Pos) -> Self {
        FunVariadicity::FVellipsis(p0)
    }
    pub fn mk_fvnon_variadic() -> Self {
        FunVariadicity::FVnonVariadic
    }
}
impl<Ex, Fb, En, Hi> ClassAttr<Ex, Fb, En, Hi> {
    pub fn mk_caname(p0: Sid) -> Self {
        ClassAttr::CAName(p0)
    }
    pub fn mk_cafield(p0: CaField<Ex, Fb, En, Hi>) -> Self {
        ClassAttr::CAField(p0)
    }
}
impl CaType {
    pub fn mk_cahint(p0: Hint) -> Self {
        CaType::CAHint(p0)
    }
    pub fn mk_caenum(p0: Vec<String>) -> Self {
        CaType::CAEnum(p0)
    }
}
impl TypeconstAbstractKind {
    pub fn mk_tcabstract(p0: Option<Hint>) -> Self {
        TypeconstAbstractKind::TCAbstract(p0)
    }
    pub fn mk_tcpartially_abstract() -> Self {
        TypeconstAbstractKind::TCPartiallyAbstract
    }
    pub fn mk_tcconcrete() -> Self {
        TypeconstAbstractKind::TCConcrete
    }
}
impl<Ex, Fb, En, Hi> Def<Ex, Fb, En, Hi> {
    pub fn mk_fun(p0: FunDef<Ex, Fb, En, Hi>) -> Self {
        Def::Fun(Box::new(p0))
    }
    pub fn mk_class(p0: Class_<Ex, Fb, En, Hi>) -> Self {
        Def::Class(Box::new(p0))
    }
    pub fn mk_record_def(p0: RecordDef<Ex, Fb, En, Hi>) -> Self {
        Def::RecordDef(Box::new(p0))
    }
    pub fn mk_stmt(p0: Stmt<Ex, Fb, En, Hi>) -> Self {
        Def::Stmt(Box::new(p0))
    }
    pub fn mk_typedef(p0: Typedef<Ex, Fb, En, Hi>) -> Self {
        Def::Typedef(Box::new(p0))
    }
    pub fn mk_constant(p0: Gconst<Ex, Fb, En, Hi>) -> Self {
        Def::Constant(Box::new(p0))
    }
    pub fn mk_namespace(p0: Sid, p1: Program<Ex, Fb, En, Hi>) -> Self {
        Def::Namespace(Box::new((p0, p1)))
    }
    pub fn mk_namespace_use(p0: Vec<(NsKind, Sid, Sid)>) -> Self {
        Def::NamespaceUse(p0)
    }
    pub fn mk_set_namespace_env(p0: Nsenv) -> Self {
        Def::SetNamespaceEnv(Box::new(p0))
    }
    pub fn mk_file_attributes(p0: FileAttribute<Ex, Fb, En, Hi>) -> Self {
        Def::FileAttributes(Box::new(p0))
    }
}
impl BreakContinueLevel {
    pub fn mk_level_ok(p0: Option<isize>) -> Self {
        BreakContinueLevel::LevelOk(p0)
    }
    pub fn mk_level_non_literal() -> Self {
        BreakContinueLevel::LevelNonLiteral
    }
    pub fn mk_level_non_positive() -> Self {
        BreakContinueLevel::LevelNonPositive
    }
}

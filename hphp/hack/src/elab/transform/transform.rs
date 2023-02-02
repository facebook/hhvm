// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ops::ControlFlow;

use oxidized::aast_defs::Afield;
use oxidized::aast_defs::AsExpr;
use oxidized::aast_defs::Block;
use oxidized::aast_defs::Case;
use oxidized::aast_defs::Catch;
use oxidized::aast_defs::ClassAbstractTypeconst;
use oxidized::aast_defs::ClassConcreteTypeconst;
use oxidized::aast_defs::ClassConst;
use oxidized::aast_defs::ClassConstKind;
use oxidized::aast_defs::ClassGetExpr;
use oxidized::aast_defs::ClassHint;
use oxidized::aast_defs::ClassId;
use oxidized::aast_defs::ClassId_;
use oxidized::aast_defs::ClassTypeconst;
use oxidized::aast_defs::ClassTypeconstDef;
use oxidized::aast_defs::ClassVar;
use oxidized::aast_defs::Class_;
use oxidized::aast_defs::CollectionTarg;
use oxidized::aast_defs::Context;
use oxidized::aast_defs::Contexts;
use oxidized::aast_defs::CtxRefinement;
use oxidized::aast_defs::CtxRefinementBounds;
use oxidized::aast_defs::Def;
use oxidized::aast_defs::DefaultCase;
use oxidized::aast_defs::Efun;
use oxidized::aast_defs::Enum_;
use oxidized::aast_defs::Expr;
use oxidized::aast_defs::Expr_;
use oxidized::aast_defs::ExpressionTree;
use oxidized::aast_defs::Field;
use oxidized::aast_defs::FileAttribute;
use oxidized::aast_defs::FunDef;
use oxidized::aast_defs::FunParam;
use oxidized::aast_defs::Fun_;
use oxidized::aast_defs::FuncBody;
use oxidized::aast_defs::FunctionPtrId;
use oxidized::aast_defs::Gconst;
use oxidized::aast_defs::GenCase;
use oxidized::aast_defs::Hint;
use oxidized::aast_defs::HintFun;
use oxidized::aast_defs::Hint_;
use oxidized::aast_defs::Method_;
use oxidized::aast_defs::ModuleDef;
use oxidized::aast_defs::NastShapeInfo;
use oxidized::aast_defs::Program;
use oxidized::aast_defs::Refinement;
use oxidized::aast_defs::RequireKind;
use oxidized::aast_defs::ShapeFieldInfo;
use oxidized::aast_defs::Stmt;
use oxidized::aast_defs::Stmt_;
use oxidized::aast_defs::Targ;
use oxidized::aast_defs::Tparam;
use oxidized::aast_defs::TraitHint;
use oxidized::aast_defs::TypeHint;
use oxidized::aast_defs::TypeHint_;
use oxidized::aast_defs::TypeRefinement;
use oxidized::aast_defs::TypeRefinementBounds;
use oxidized::aast_defs::Typedef;
use oxidized::aast_defs::UserAttribute;
use oxidized::aast_defs::UserAttributes;
use oxidized::aast_defs::UsingStmt;
use oxidized::aast_defs::VariadicHint;
use oxidized::aast_defs::WhereConstraintHint;
use oxidized::aast_defs::XhpAttr;
use oxidized::aast_defs::XhpAttrHint;
use oxidized::aast_defs::XhpAttribute;
use oxidized::aast_defs::XhpSimple;

pub trait Pass {
    type Ctx: Clone;

    #[inline(always)]
    fn on_ty_program<Ex, En>(
        &self,
        _elem: &mut Program<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Def ------------------------------------------------------------------

    #[inline(always)]
    fn on_ty_def<Ex, En>(
        &self,
        _elem: &mut Def<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- TypeDef --------------------------------------------------------------

    #[inline(always)]
    fn on_ty_typedef<Ex, En>(
        &self,
        _elem: &mut Typedef<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- FunDef ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_fun_def<Ex, En>(
        &self,
        _elem: &mut FunDef<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- ModuleDef ------------------------------------------------------------

    #[inline(always)]
    fn on_ty_module_def<Ex, En>(
        &self,
        _elem: &mut ModuleDef<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Gconst ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_gconst<Ex, En>(
        &self,
        _elem: &mut Gconst<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_gconst_type_(
        &self,
        _elem: &mut Option<Hint>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_gconst_value<Ex, En>(
        &self,
        _elem: &mut Expr<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Stmt -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_stmt<Ex, En>(
        &self,
        _elem: &mut Stmt<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_stmt_<Ex, En>(
        &self,
        _elem: &mut Stmt_<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_block<Ex, En>(
        &self,
        _elem: &mut Block<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_using_stmt<Ex, En>(
        &self,
        _elem: &mut UsingStmt<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_gen_case<Ex, En>(
        &self,
        _elem: &mut GenCase<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_case<Ex, En>(
        &self,
        _elem: &mut Case<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_default_case<Ex, En>(
        &self,
        _elem: &mut DefaultCase<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_catch<Ex, En>(
        &self,
        _elem: &mut Catch<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_as_expr<Ex, En>(
        &self,
        _elem: &mut AsExpr<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- ClassId --------------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_id<Ex, En>(
        &self,
        _elem: &mut ClassId<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_class_id_<Ex, En>(
        &self,
        _elem: &mut ClassId_<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Expr -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_expr<Ex, En>(
        &self,
        _elem: &mut Expr<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_expr_<Ex, En>(
        &self,
        _elem: &mut Expr_<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_collection_targ<Ex>(
        &self,
        _elem: &mut CollectionTarg<Ex>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_funcion_ptr_id<Ex, En>(
        &self,
        _elem: &mut FunctionPtrId<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_expression_tree<Ex, En>(
        &self,
        _elem: &mut ExpressionTree<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_class_get_expr<Ex, En>(
        &self,
        _elem: &mut ClassGetExpr<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_field<Ex, En>(
        &self,
        _elem: &mut Field<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_afield<Ex, En>(
        &self,
        _elem: &mut Afield<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_xhp_simple<Ex, En>(
        &self,
        _elem: &mut XhpSimple<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_xhp_attribute<Ex, En>(
        &self,
        _elem: &mut XhpAttribute<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_xhp_attr<Ex, En>(
        &self,
        _elem: &mut XhpAttr<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }
    // -- Fun_ -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_fun_<Ex, En>(
        &self,
        _elem: &mut Fun_<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_fun__ret<Ex>(
        &self,
        _elem: &mut TypeHint<Ex>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Method_ --------------------------------------------------------------

    #[inline(always)]
    fn on_ty_method_<Ex, En>(
        &self,
        _elem: &mut Method_<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_method__ret<Ex>(
        &self,
        _elem: &mut TypeHint<Ex>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- FunParam -------------------------------------------------------------

    #[inline(always)]
    fn on_ty_fun_param<Ex, En>(
        &self,
        _elem: &mut FunParam<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Efun -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_efun<Ex, En>(
        &self,
        _elem: &mut Efun<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- FuncBody -------------------------------------------------------------

    #[inline(always)]
    fn on_ty_func_body<Ex, En>(
        &self,
        _elem: &mut FuncBody<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Class_ ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_<Ex, En>(
        &self,
        _elem: &mut Class_<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__tparams<Ex, En>(
        &self,
        _elem: &mut Vec<Tparam<Ex, En>>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__extends(
        &self,
        _elem: &mut Vec<ClassHint>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__uses(
        &self,
        _elem: &mut Vec<TraitHint>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__xhp_attr_uses(
        &self,
        _elem: &mut Vec<XhpAttrHint>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__reqs(
        &self,
        _elem: &mut Vec<(ClassHint, RequireKind)>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__implements(
        &self,
        _elem: &mut Vec<ClassHint>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__where_constraints(
        &self,
        _elem: &mut Vec<WhereConstraintHint>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__consts<Ex, En>(
        &self,
        _elem: &mut Vec<ClassConst<Ex, En>>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__typeconsts<Ex, En>(
        &self,
        _elem: &mut Vec<ClassTypeconstDef<Ex, En>>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__vars<Ex, En>(
        &self,
        _elem: &mut Vec<ClassVar<Ex, En>>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__methods<Ex, En>(
        &self,
        _elem: &mut Vec<Method_<Ex, En>>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__xhp_attrs<Ex, En>(
        &self,
        _elem: &mut Vec<XhpAttr<Ex, En>>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__user_attributes<Ex, En>(
        &self,
        _elem: &mut UserAttributes<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__enum_(
        &self,
        _elem: &mut Option<Enum_>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- ClassVar -------------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_var<Ex, En>(
        &self,
        _elem: &mut ClassVar<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_class_var_type_<Ex>(
        &self,
        _elem: &mut TypeHint<Ex>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_class_var_expr<Ex, En>(
        &self,
        _elem: &mut Option<Expr<Ex, En>>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- ClassConst -----------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_const<Ex, En>(
        &self,
        _elem: &mut ClassConst<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_class_const_kind<Ex, En>(
        &self,
        _elem: &mut ClassConstKind<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- ClassTypeconstDef ----------------------------------------------------
    #[inline(always)]
    fn on_ty_class_typeconst_def<Ex, En>(
        &self,
        _elem: &mut ClassTypeconstDef<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst(
        &self,
        _elem: &mut ClassTypeconst,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_class_abstract_typeconst(
        &self,
        _elem: &mut ClassAbstractTypeconst,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_class_concrete_typeconst(
        &self,
        _elem: &mut ClassConcreteTypeconst,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Context(s) -----------------------------------------------------------

    #[inline(always)]
    fn on_ty_context(&self, _elem: &mut Context, _ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- WhereConstraintHint --------------------------------------------------

    #[inline(always)]
    fn on_ty_where_constraint_hint(
        &self,
        _elem: &mut WhereConstraintHint,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Enum -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_enum_(&self, _elem: &mut Enum_, _ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- TypeHint<Ex> ---------------------------------------------------------

    #[inline(always)]
    fn on_ty_type_hint<Ex>(
        &self,
        _elem: &mut TypeHint<Ex>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_type_hint_(&self, _elem: &mut TypeHint_, _ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Targ -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_targ<Ex>(&self, _elem: &mut Targ<Ex>, _ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Tparam ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_tparam<Ex, En>(
        &self,
        _elem: &mut Tparam<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- UserAttribute(s) -----------------------------------------------------

    #[inline(always)]
    fn on_ty_user_attributes<Ex, En>(
        &self,
        _elem: &mut UserAttributes<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_user_attribute<Ex, En>(
        &self,
        _elem: &mut UserAttribute<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- FileAttribute --------------------------------------------------------

    #[inline(always)]
    fn on_ty_file_attribute<Ex, En>(
        &self,
        _elem: &mut FileAttribute<Ex, En>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    // -- Hints ----------------------------------------------------------------
    #[inline(always)]
    fn on_ty_hint(&self, _elem: &mut Hint, _ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_hint_(&self, _elem: &mut Hint_, _ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_hint_fun(&self, _elem: &mut HintFun, _ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_hint_fun_param_tys(
        &self,
        _elem: &mut Vec<Hint>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_hint_fun_variadic_ty(
        &self,
        _elem: &mut VariadicHint,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_hint_fun_ctxs(
        &self,
        _elem: &mut Option<Contexts>,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_fld_hint_fun_return_ty(
        &self,
        _elem: &mut Hint,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_refinement(
        &self,
        _elem: &mut Refinement,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_type_refinement(
        &self,
        _elem: &mut TypeRefinement,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_type_refinement_bounds(
        &self,
        _elem: &mut TypeRefinementBounds,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_ctx_refinement(
        &self,
        _elem: &mut CtxRefinement,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_ctx_refinement_bounds(
        &self,
        _elem: &mut CtxRefinementBounds,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_contexts(&self, _elem: &mut Contexts, _ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_nast_shape_info(
        &self,
        _elem: &mut NastShapeInfo,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_shape_field_info(
        &self,
        _elem: &mut ShapeFieldInfo,
        _ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_class_hint(&self, _elem: &mut Hint, _ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_trait_hint(&self, _elem: &mut Hint, _ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }

    #[inline(always)]
    fn on_ty_xhp_attr_hint(&self, _elem: &mut Hint, _ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        ControlFlow::Continue(())
    }
}

struct Passes<Ctx, P, Q>
where
    Ctx: Clone,
    P: Pass<Ctx = Ctx>,
    Q: Pass<Ctx = Ctx>,
{
    fst: P,
    snd: Q,
}

impl<Ctx, P, Q> Pass for Passes<Ctx, P, Q>
where
    Ctx: Clone,
    P: Pass<Ctx = Ctx>,
    Q: Pass<Ctx = Ctx>,
{
    type Ctx = Ctx;

    #[inline(always)]
    fn on_ty_program<Ex, En>(
        &self,
        elem: &mut Program<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_program(elem, ctx)?;
        self.snd.on_ty_program(elem, ctx)
    }

    // -- Def ------------------------------------------------------------------

    #[inline(always)]
    fn on_ty_def<Ex, En>(
        &self,
        elem: &mut Def<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_def(elem, ctx)?;
        self.snd.on_ty_def(elem, ctx)
    }

    // -- TypeDef --------------------------------------------------------------

    #[inline(always)]
    fn on_ty_typedef<Ex, En>(
        &self,
        elem: &mut Typedef<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_typedef(elem, ctx)?;
        self.snd.on_ty_typedef(elem, ctx)
    }

    // -- FunDef ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_fun_def<Ex, En>(
        &self,
        elem: &mut FunDef<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_fun_def(elem, ctx)?;
        self.snd.on_ty_fun_def(elem, ctx)
    }

    // -- ModuleDef ------------------------------------------------------------

    #[inline(always)]
    fn on_ty_module_def<Ex, En>(
        &self,
        elem: &mut ModuleDef<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_module_def(elem, ctx)?;
        self.snd.on_ty_module_def(elem, ctx)
    }

    // -- Gconst ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_gconst<Ex, En>(
        &self,
        elem: &mut Gconst<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_gconst(elem, ctx)?;
        self.snd.on_ty_gconst(elem, ctx)
    }

    #[inline(always)]
    fn on_fld_gconst_type_(
        &self,
        elem: &mut Option<Hint>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_gconst_type_(elem, ctx)?;
        self.snd.on_fld_gconst_type_(elem, ctx)
    }

    #[inline(always)]
    fn on_fld_gconst_value<Ex, En>(
        &self,
        elem: &mut Expr<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_gconst_value(elem, ctx)?;
        self.snd.on_fld_gconst_value(elem, ctx)
    }

    // -- Stmt -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_stmt<Ex, En>(
        &self,
        elem: &mut Stmt<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_stmt(elem, ctx)?;
        self.snd.on_ty_stmt(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_stmt_<Ex, En>(
        &self,
        elem: &mut Stmt_<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_stmt_(elem, ctx)?;
        self.snd.on_ty_stmt_(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_block<Ex, En>(
        &self,
        elem: &mut Block<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_block(elem, ctx)?;
        self.snd.on_ty_block(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_using_stmt<Ex, En>(
        &self,
        elem: &mut UsingStmt<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_using_stmt(elem, ctx)?;
        self.snd.on_ty_using_stmt(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_gen_case<Ex, En>(
        &self,
        elem: &mut GenCase<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_gen_case(elem, ctx)?;
        self.snd.on_ty_gen_case(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_case<Ex, En>(
        &self,
        elem: &mut Case<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_case(elem, ctx)?;
        self.snd.on_ty_case(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_default_case<Ex, En>(
        &self,
        elem: &mut DefaultCase<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_default_case(elem, ctx)?;
        self.snd.on_ty_default_case(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_catch<Ex, En>(
        &self,
        elem: &mut Catch<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_catch(elem, ctx)?;
        self.snd.on_ty_catch(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_as_expr<Ex, En>(
        &self,
        elem: &mut AsExpr<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_as_expr(elem, ctx)?;
        self.snd.on_ty_as_expr(elem, ctx)
    }

    // -- ClassId --------------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_id<Ex, En>(
        &self,
        elem: &mut ClassId<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_id(elem, ctx)?;
        self.snd.on_ty_class_id(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_class_id_<Ex, En>(
        &self,
        elem: &mut ClassId_<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_id_(elem, ctx)?;
        self.snd.on_ty_class_id_(elem, ctx)
    }

    // -- Expr -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_expr<Ex, En>(
        &self,
        elem: &mut Expr<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_expr(elem, ctx)?;
        self.snd.on_ty_expr(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_expr_<Ex, En>(
        &self,
        elem: &mut Expr_<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_expr_(elem, ctx)?;
        self.snd.on_ty_expr_(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_collection_targ<Ex>(
        &self,
        elem: &mut CollectionTarg<Ex>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_collection_targ(elem, ctx)?;
        self.snd.on_ty_collection_targ(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_funcion_ptr_id<Ex, En>(
        &self,
        elem: &mut FunctionPtrId<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_funcion_ptr_id(elem, ctx)?;
        self.snd.on_ty_funcion_ptr_id(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_expression_tree<Ex, En>(
        &self,
        elem: &mut ExpressionTree<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_expression_tree(elem, ctx)?;
        self.snd.on_ty_expression_tree(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_class_get_expr<Ex, En>(
        &self,
        elem: &mut ClassGetExpr<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_get_expr(elem, ctx)?;
        self.snd.on_ty_class_get_expr(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_field<Ex, En>(
        &self,
        elem: &mut Field<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_field(elem, ctx)?;
        self.snd.on_ty_field(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_afield<Ex, En>(
        &self,
        elem: &mut Afield<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_afield(elem, ctx)?;
        self.snd.on_ty_afield(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_xhp_simple<Ex, En>(
        &self,
        elem: &mut XhpSimple<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_xhp_simple(elem, ctx)?;
        self.snd.on_ty_xhp_simple(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_xhp_attribute<Ex, En>(
        &self,
        elem: &mut XhpAttribute<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_xhp_attribute(elem, ctx)?;
        self.snd.on_ty_xhp_attribute(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_xhp_attr<Ex, En>(
        &self,
        elem: &mut XhpAttr<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_xhp_attr(elem, ctx)?;
        self.snd.on_ty_xhp_attr(elem, ctx)
    }
    // -- Fun_ -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_fun_<Ex, En>(
        &self,
        elem: &mut Fun_<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_fun_(elem, ctx)?;
        self.snd.on_ty_fun_(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_fun__ret<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_fun__ret(elem, ctx)?;
        self.snd.on_fld_fun__ret(elem, ctx)
    }

    // -- Method_ --------------------------------------------------------------

    #[inline(always)]
    fn on_ty_method_<Ex, En>(
        &self,
        elem: &mut Method_<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_method_(elem, ctx)?;
        self.snd.on_ty_method_(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_method__ret<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_method__ret(elem, ctx)?;
        self.snd.on_fld_method__ret(elem, ctx)
    }

    // -- FunParam -------------------------------------------------------------

    #[inline(always)]
    fn on_ty_fun_param<Ex, En>(
        &self,
        elem: &mut FunParam<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_fun_param(elem, ctx)?;
        self.snd.on_ty_fun_param(elem, ctx)
    }

    // -- Efun -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_efun<Ex, En>(
        &self,
        elem: &mut Efun<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_efun(elem, ctx)?;
        self.snd.on_ty_efun(elem, ctx)
    }

    // -- FuncBody -------------------------------------------------------------

    #[inline(always)]
    fn on_ty_func_body<Ex, En>(
        &self,
        elem: &mut FuncBody<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_func_body(elem, ctx)?;
        self.snd.on_ty_func_body(elem, ctx)
    }

    // -- Class_ ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_<Ex, En>(
        &self,
        elem: &mut Class_<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_(elem, ctx)?;
        self.snd.on_ty_class_(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__tparams<Ex, En>(
        &self,
        elem: &mut Vec<Tparam<Ex, En>>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__tparams(elem, ctx)?;
        self.snd.on_fld_class__tparams(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__extends(
        &self,
        elem: &mut Vec<ClassHint>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__extends(elem, ctx)?;
        self.snd.on_fld_class__extends(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__uses(
        &self,
        elem: &mut Vec<TraitHint>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__uses(elem, ctx)?;
        self.snd.on_fld_class__uses(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__xhp_attr_uses(
        &self,
        elem: &mut Vec<XhpAttrHint>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__xhp_attr_uses(elem, ctx)?;
        self.snd.on_fld_class__xhp_attr_uses(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__reqs(
        &self,
        elem: &mut Vec<(ClassHint, RequireKind)>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__reqs(elem, ctx)?;
        self.snd.on_fld_class__reqs(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__implements(
        &self,
        elem: &mut Vec<ClassHint>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__implements(elem, ctx)?;
        self.snd.on_fld_class__implements(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__where_constraints(
        &self,
        elem: &mut Vec<WhereConstraintHint>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__where_constraints(elem, ctx)?;
        self.snd.on_fld_class__where_constraints(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__consts<Ex, En>(
        &self,
        elem: &mut Vec<ClassConst<Ex, En>>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__consts(elem, ctx)?;
        self.snd.on_fld_class__consts(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__typeconsts<Ex, En>(
        &self,
        elem: &mut Vec<ClassTypeconstDef<Ex, En>>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__typeconsts(elem, ctx)?;
        self.snd.on_fld_class__typeconsts(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__vars<Ex, En>(
        &self,
        elem: &mut Vec<ClassVar<Ex, En>>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__vars(elem, ctx)?;
        self.snd.on_fld_class__vars(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__methods<Ex, En>(
        &self,
        elem: &mut Vec<Method_<Ex, En>>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__methods(elem, ctx)?;
        self.snd.on_fld_class__methods(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__xhp_attrs<Ex, En>(
        &self,
        elem: &mut Vec<XhpAttr<Ex, En>>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__xhp_attrs(elem, ctx)?;
        self.snd.on_fld_class__xhp_attrs(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__user_attributes<Ex, En>(
        &self,
        elem: &mut UserAttributes<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__user_attributes(elem, ctx)?;
        self.snd.on_fld_class__user_attributes(elem, ctx)
    }

    #[inline(always)]
    #[allow(non_snake_case)]
    fn on_fld_class__enum_(
        &self,
        elem: &mut Option<Enum_>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__enum_(elem, ctx)?;
        self.snd.on_fld_class__enum_(elem, ctx)
    }

    // -- ClassVar -------------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_var<Ex, En>(
        &self,
        elem: &mut ClassVar<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_var(elem, ctx)?;
        self.snd.on_ty_class_var(elem, ctx)
    }

    #[inline(always)]
    fn on_fld_class_var_type_<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class_var_type_(elem, ctx)?;
        self.snd.on_fld_class_var_type_(elem, ctx)
    }

    #[inline(always)]
    fn on_fld_class_var_expr<Ex, En>(
        &self,
        elem: &mut Option<Expr<Ex, En>>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class_var_expr(elem, ctx)?;
        self.snd.on_fld_class_var_expr(elem, ctx)
    }

    // -- ClassConst -----------------------------------------------------------

    #[inline(always)]
    fn on_ty_class_const<Ex, En>(
        &self,
        elem: &mut ClassConst<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_const(elem, ctx)?;
        self.snd.on_ty_class_const(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_class_const_kind<Ex, En>(
        &self,
        elem: &mut ClassConstKind<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_const_kind(elem, ctx)?;
        self.snd.on_ty_class_const_kind(elem, ctx)
    }

    // -- ClassTypeconstDef ----------------------------------------------------
    #[inline(always)]
    fn on_ty_class_typeconst_def<Ex, En>(
        &self,
        elem: &mut ClassTypeconstDef<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_typeconst_def(elem, ctx)?;
        self.snd.on_ty_class_typeconst_def(elem, ctx)
    }
    #[inline(always)]
    fn on_ty_class_typeconst(
        &self,
        elem: &mut ClassTypeconst,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_typeconst(elem, ctx)?;
        self.snd.on_ty_class_typeconst(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_class_abstract_typeconst(
        &self,
        elem: &mut ClassAbstractTypeconst,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_abstract_typeconst(elem, ctx)?;
        self.snd.on_ty_class_abstract_typeconst(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_class_concrete_typeconst(
        &self,
        elem: &mut ClassConcreteTypeconst,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_concrete_typeconst(elem, ctx)?;
        self.snd.on_ty_class_concrete_typeconst(elem, ctx)
    }

    // -- Context(s) -----------------------------------------------------------

    #[inline(always)]
    fn on_ty_contexts(&self, elem: &mut Contexts, ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        self.fst.on_ty_contexts(elem, ctx)?;
        self.snd.on_ty_contexts(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_context(&self, elem: &mut Context, ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        self.fst.on_ty_context(elem, ctx)?;
        self.snd.on_ty_context(elem, ctx)
    }

    // -- WhereConstraintHint --------------------------------------------------

    #[inline(always)]
    fn on_ty_where_constraint_hint(
        &self,
        elem: &mut WhereConstraintHint,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_where_constraint_hint(elem, ctx)?;
        self.snd.on_ty_where_constraint_hint(elem, ctx)
    }

    // -- Enum -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_enum_(&self, elem: &mut Enum_, ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        self.fst.on_ty_enum_(elem, ctx)?;
        self.snd.on_ty_enum_(elem, ctx)
    }

    // -- TypeHint<Ex> ---------------------------------------------------------

    #[inline(always)]
    fn on_ty_type_hint<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_type_hint(elem, ctx)?;
        self.snd.on_ty_type_hint(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_type_hint_(&self, elem: &mut TypeHint_, ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        self.fst.on_ty_type_hint_(elem, ctx)?;
        self.snd.on_ty_type_hint_(elem, ctx)
    }

    // -- Targ -----------------------------------------------------------------

    #[inline(always)]
    fn on_ty_targ<Ex>(&self, elem: &mut Targ<Ex>, ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        self.fst.on_ty_targ(elem, ctx)?;
        self.snd.on_ty_targ(elem, ctx)
    }

    // -- Tparam ---------------------------------------------------------------

    #[inline(always)]
    fn on_ty_tparam<Ex, En>(
        &self,
        elem: &mut Tparam<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_tparam(elem, ctx)?;
        self.snd.on_ty_tparam(elem, ctx)
    }

    // -- UserAttribute(s) -----------------------------------------------------

    #[inline(always)]
    fn on_ty_user_attributes<Ex, En>(
        &self,
        elem: &mut UserAttributes<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_user_attributes(elem, ctx)?;
        self.snd.on_ty_user_attributes(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_user_attribute<Ex, En>(
        &self,
        elem: &mut UserAttribute<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_user_attribute(elem, ctx)?;
        self.snd.on_ty_user_attribute(elem, ctx)
    }

    // -- FileAttribute --------------------------------------------------------

    #[inline(always)]
    fn on_ty_file_attribute<Ex, En>(
        &self,
        elem: &mut FileAttribute<Ex, En>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_file_attribute(elem, ctx)?;
        self.snd.on_ty_file_attribute(elem, ctx)
    }

    // -- Hints ----------------------------------------------------------------
    #[inline(always)]
    fn on_ty_hint(&self, elem: &mut Hint, ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        self.fst.on_ty_hint(elem, ctx)?;
        self.snd.on_ty_hint(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_hint_(&self, elem: &mut Hint_, ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        self.fst.on_ty_hint_(elem, ctx)?;
        self.snd.on_ty_hint_(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_hint_fun(&self, elem: &mut HintFun, ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        self.fst.on_ty_hint_fun(elem, ctx)?;
        self.snd.on_ty_hint_fun(elem, ctx)
    }

    #[inline(always)]
    fn on_fld_hint_fun_param_tys(
        &self,
        elem: &mut Vec<Hint>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_hint_fun_param_tys(elem, ctx)?;
        self.snd.on_fld_hint_fun_param_tys(elem, ctx)
    }

    #[inline(always)]
    fn on_fld_hint_fun_variadic_ty(
        &self,
        elem: &mut VariadicHint,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_hint_fun_variadic_ty(elem, ctx)?;
        self.snd.on_fld_hint_fun_variadic_ty(elem, ctx)
    }

    #[inline(always)]
    fn on_fld_hint_fun_ctxs(
        &self,
        elem: &mut Option<Contexts>,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_hint_fun_ctxs(elem, ctx)?;
        self.snd.on_fld_hint_fun_ctxs(elem, ctx)
    }

    #[inline(always)]
    fn on_fld_hint_fun_return_ty(
        &self,
        elem: &mut Hint,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_hint_fun_return_ty(elem, ctx)?;
        self.snd.on_fld_hint_fun_return_ty(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_refinement(&self, elem: &mut Refinement, ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        self.fst.on_ty_refinement(elem, ctx)?;
        self.snd.on_ty_refinement(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_type_refinement(
        &self,
        elem: &mut TypeRefinement,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_type_refinement(elem, ctx)?;
        self.snd.on_ty_type_refinement(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_type_refinement_bounds(
        &self,
        elem: &mut TypeRefinementBounds,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_type_refinement_bounds(elem, ctx)?;
        self.snd.on_ty_type_refinement_bounds(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_ctx_refinement(
        &self,
        elem: &mut CtxRefinement,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_ctx_refinement(elem, ctx)?;
        self.snd.on_ty_ctx_refinement(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_ctx_refinement_bounds(
        &self,
        elem: &mut CtxRefinementBounds,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_ctx_refinement_bounds(elem, ctx)?;
        self.snd.on_ty_ctx_refinement_bounds(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_nast_shape_info(
        &self,
        elem: &mut NastShapeInfo,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_nast_shape_info(elem, ctx)?;
        self.snd.on_ty_nast_shape_info(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_shape_field_info(
        &self,
        elem: &mut ShapeFieldInfo,
        ctx: &mut Self::Ctx,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_shape_field_info(elem, ctx)?;
        self.snd.on_ty_shape_field_info(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_class_hint(&self, elem: &mut Hint, ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_hint(elem, ctx)?;
        self.snd.on_ty_class_hint(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_trait_hint(&self, elem: &mut Hint, ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        self.fst.on_ty_trait_hint(elem, ctx)?;
        self.snd.on_ty_trait_hint(elem, ctx)
    }

    #[inline(always)]
    fn on_ty_xhp_attr_hint(&self, elem: &mut Hint, ctx: &mut Self::Ctx) -> ControlFlow<(), ()> {
        self.fst.on_ty_xhp_attr_hint(elem, ctx)?;
        self.snd.on_ty_xhp_attr_hint(elem, ctx)
    }
}

// Used to combine multiple types implementing `Pass` into nested `Passes` types
// without requiring them to hand write it so :
// `!passes (p1,p2,p3)` => `Passes(p1,Passes(p2,p3))`
#[macro_export]
macro_rules! passes{
    ( $p:expr $(,$ps:expr)+ $(,)? ) => {
        $crate::transform::Passes($p, passes!($($ps),*))
    };
    ( $p:expr $(,)? ) => {
        $p
    };
}

// -----------------------------------------------------------------------------
// -- Transform & traverse
// -----------------------------------------------------------------------------

pub fn transform_ty_program<Ctx: Clone, Ex, En>(
    elem: &mut Program<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_program(elem, ctx) {
        return;
    }
    traverse_ty_program(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_program(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_program<Ctx: Clone, Ex, En>(
    elem: &mut Program<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|def| transform_ty_def(def, &mut ctx.clone(), top_down, bottom_up))
}

// -- Def ----------------------------------------------------------------------

pub fn transform_ty_def<Ctx: Clone, Ex, En>(
    elem: &mut Def<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_def(elem, ctx) {
        return;
    }
    traverse_ty_def(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_def(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_def<Ctx: Clone, Ex, En>(
    elem: &mut Def<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        Def::Fun(elem) => transform_ty_fun_def(elem, ctx, top_down, bottom_up),
        Def::Class(elem) => transform_ty_class_(elem, ctx, top_down, bottom_up),
        Def::Stmt(elem) => transform_ty_stmt(elem, ctx, top_down, bottom_up),
        Def::Typedef(elem) => transform_ty_typedef(elem, ctx, top_down, bottom_up),
        Def::Constant(elem) => transform_ty_gconst(elem, ctx, top_down, bottom_up),
        Def::Module(elem) => transform_ty_module_def(elem, ctx, top_down, bottom_up),
        Def::Namespace(_)
        | Def::NamespaceUse(_)
        | Def::SetNamespaceEnv(_)
        | Def::FileAttributes(_)
        | Def::SetModule(_) => (),
    }
}

// -- TypeDef ------------------------------------------------------------------

pub fn transform_ty_typedef<Ctx: Clone, Ex, En>(
    elem: &mut Typedef<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_typedef(elem, ctx) {
        return;
    }
    traverse_ty_typedef(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_typedef(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_typedef<Ctx: Clone, Ex, En>(
    elem: &mut Typedef<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.tparams
        .iter_mut()
        .for_each(|elem| transform_ty_tparam(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.as_constraint
        .iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.super_constraint
        .iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up));
    transform_ty_hint(&mut elem.kind, &mut ctx.clone(), top_down, bottom_up);
    transform_ty_user_attributes(
        &mut elem.user_attributes,
        &mut ctx.clone(),
        top_down,
        bottom_up,
    );
    elem.file_attributes
        .iter_mut()
        .for_each(|elem| transform_ty_file_attribute(elem, &mut ctx.clone(), top_down, bottom_up))
}
// -- FunDef -------------------------------------------------------------------

pub fn transform_ty_fun_def<Ctx: Clone, Ex, En>(
    elem: &mut FunDef<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_fun_def(elem, ctx) {
        return;
    }
    traverse_ty_fun_def(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_fun_def(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_fun_def<Ctx: Clone, Ex, En>(
    elem: &mut FunDef<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.file_attributes
        .iter_mut()
        .for_each(|elem| transform_ty_file_attribute(elem, &mut ctx.clone(), top_down, bottom_up));
    transform_ty_fun_(&mut elem.fun, ctx, top_down, bottom_up)
}

// -- ModuleDef ----------------------------------------------------------------

pub fn transform_ty_module_def<Ctx: Clone, Ex, En>(
    elem: &mut ModuleDef<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_module_def(elem, ctx) {
        return;
    }
    traverse_ty_module_def(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_module_def(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_module_def<Ctx: Clone, Ex, En>(
    elem: &mut ModuleDef<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_user_attributes(&mut elem.user_attributes, ctx, top_down, bottom_up);
}

// -- Gconst -------------------------------------------------------------------

pub fn transform_ty_gconst<Ctx: Clone, Ex, En>(
    elem: &mut Gconst<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_gconst(elem, ctx) {
        return;
    }
    traverse_ty_gconst(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_gconst(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_gconst<Ctx: Clone, Ex, En>(
    elem: &mut Gconst<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.type_
        .iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up));
    transform_ty_expr(&mut elem.value, ctx, top_down, bottom_up)
}

pub fn transform_fld_gconst_type_<Ctx: Clone>(
    elem: &mut Option<Hint>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_gconst_type_(elem, ctx) {
        return;
    }
    traverse_fld_gconst_type_(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_gconst_type_(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_fld_gconst_type_<Ctx: Clone>(
    elem: &mut Option<Hint>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up))
}

pub fn transform_fld_gconst_value_<Ctx: Clone, Ex, En>(
    elem: &mut Expr<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_gconst_value(elem, ctx) {
        return;
    }
    traverse_fld_gconst_value(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_gconst_value(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_fld_gconst_value<Ctx: Clone, Ex, En>(
    elem: &mut Expr<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_expr(elem, ctx, top_down, bottom_up)
}

// -- Stmt ---------------------------------------------------------------------

pub fn transform_ty_stmt<Ctx: Clone, Ex, En>(
    elem: &mut Stmt<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_stmt(elem, ctx) {
        return;
    }
    traverse_ty_stmt(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_stmt(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_stmt<Ctx: Clone, Ex, En>(
    elem: &mut Stmt<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_stmt_(&mut elem.1, ctx, top_down, bottom_up)
}

pub fn transform_ty_stmt_<Ctx: Clone, Ex, En>(
    elem: &mut Stmt_<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_stmt_(elem, ctx) {
        return;
    }
    traverse_ty_stmt_(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_stmt_(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_stmt_<Ctx: Clone, Ex, En>(
    elem: &mut Stmt_<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        Stmt_::Expr(elem) | Stmt_::Throw(elem) => transform_ty_expr(elem, ctx, top_down, bottom_up),
        Stmt_::Return(elem) => elem
            .iter_mut()
            .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up)),
        Stmt_::Awaitall(elem) => {
            elem.0.iter_mut().for_each(|(_elem00, elem01)| {
                transform_ty_expr(elem01, &mut ctx.clone(), top_down, bottom_up)
            });
            transform_ty_block(&mut elem.1, ctx, top_down, bottom_up)
        }
        Stmt_::If(elem) => {
            transform_ty_expr(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_block(&mut elem.1, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_block(&mut elem.2, ctx, top_down, bottom_up)
        }
        Stmt_::Do(elem) => {
            transform_ty_block(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_expr(&mut elem.1, ctx, top_down, bottom_up)
        }
        Stmt_::While(elem) => {
            transform_ty_expr(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_block(&mut elem.1, ctx, top_down, bottom_up);
        }
        Stmt_::Using(elem) => {
            transform_ty_using_stmt(elem, ctx, top_down, bottom_up);
        }
        Stmt_::For(elem) => {
            elem.0
                .iter_mut()
                .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up));
            elem.1
                .iter_mut()
                .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up));
            elem.2
                .iter_mut()
                .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up));
            transform_ty_block(&mut elem.3, ctx, top_down, bottom_up)
        }
        Stmt_::Switch(elem) => {
            transform_ty_expr(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            elem.1
                .iter_mut()
                .for_each(|elem| transform_ty_case(elem, &mut ctx.clone(), top_down, bottom_up));
            elem.2.iter_mut().for_each(|elem| {
                transform_ty_default_case(elem, &mut ctx.clone(), top_down, bottom_up)
            });
        }
        Stmt_::Foreach(elem) => {
            transform_ty_expr(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_as_expr(&mut elem.1, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_block(&mut elem.2, ctx, top_down, bottom_up)
        }
        Stmt_::Try(elem) => {
            transform_ty_block(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            elem.1
                .iter_mut()
                .for_each(|elem| transform_ty_catch(elem, &mut ctx.clone(), top_down, bottom_up));
            transform_ty_block(&mut elem.2, ctx, top_down, bottom_up)
        }
        Stmt_::Block(elem) => transform_ty_block(elem, ctx, top_down, bottom_up),
        Stmt_::Noop
        | Stmt_::Fallthrough
        | Stmt_::Break
        | Stmt_::Continue
        | Stmt_::YieldBreak
        | Stmt_::Markup(_)
        | Stmt_::AssertEnv(_) => (),
    }
}

pub fn transform_ty_block<Ctx: Clone, Ex, En>(
    elem: &mut Block<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_block(elem, ctx) {
        return;
    }
    traverse_ty_block(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_block(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_block<Ctx: Clone, Ex, En>(
    elem: &mut Block<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_stmt(elem, &mut ctx.clone(), top_down, bottom_up))
}

pub fn transform_ty_using_stmt<Ctx: Clone, Ex, En>(
    elem: &mut UsingStmt<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_using_stmt(elem, ctx) {
        return;
    }
    traverse_ty_using_stmt(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_using_stmt(elem, &mut in_ctx);
}
#[inline(always)]
fn traverse_ty_using_stmt<Ctx: Clone, Ex, En>(
    elem: &mut UsingStmt<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.exprs
        .1
        .iter_mut()
        .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up));
    transform_ty_block(&mut elem.block, ctx, top_down, bottom_up)
}

pub fn transform_ty_gen_case<Ctx: Clone, Ex, En>(
    elem: &mut GenCase<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_gen_case(elem, ctx) {
        return;
    }
    traverse_ty_gen_case(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_gen_case(elem, &mut in_ctx);
}
#[inline(always)]
fn traverse_ty_gen_case<Ctx: Clone, Ex, En>(
    elem: &mut GenCase<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        GenCase::Case(elem) => transform_ty_case(elem, ctx, top_down, bottom_up),
        GenCase::Default(elem) => transform_ty_default_case(elem, ctx, top_down, bottom_up),
    }
}

pub fn transform_ty_case<Ctx: Clone, Ex, En>(
    elem: &mut Case<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_case(elem, ctx) {
        return;
    }
    traverse_ty_case(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_case(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_case<Ctx: Clone, Ex, En>(
    elem: &mut Case<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_expr(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
    transform_ty_block(&mut elem.1, ctx, top_down, bottom_up);
}

pub fn transform_ty_default_case<Ctx: Clone, Ex, En>(
    elem: &mut DefaultCase<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_default_case(elem, ctx) {
        return;
    }
    traverse_ty_default_case(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_default_case(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_default_case<Ctx: Clone, Ex, En>(
    elem: &mut DefaultCase<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_block(&mut elem.1, ctx, top_down, bottom_up);
}

pub fn transform_ty_catch<Ctx: Clone, Ex, En>(
    elem: &mut Catch<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_catch(elem, ctx) {
        return;
    }
    traverse_ty_catch(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_catch(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_catch<Ctx: Clone, Ex, En>(
    elem: &mut Catch<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_block(&mut elem.2, ctx, top_down, bottom_up);
}

pub fn transform_ty_as_expr<Ctx: Clone, Ex, En>(
    elem: &mut AsExpr<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_as_expr(elem, ctx) {
        return;
    }
    traverse_ty_as_expr(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_as_expr(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_as_expr<Ctx: Clone, Ex, En>(
    elem: &mut AsExpr<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        AsExpr::AsV(elem) => transform_ty_expr(elem, ctx, top_down, bottom_up),
        AsExpr::AsKv(elem0, elem1) => {
            transform_ty_expr(elem0, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_expr(elem1, ctx, top_down, bottom_up)
        }
        AsExpr::AwaitAsV(_, elem1) => transform_ty_expr(elem1, ctx, top_down, bottom_up),
        AsExpr::AwaitAsKv(_, elem1, elem2) => {
            transform_ty_expr(elem1, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_expr(elem2, ctx, top_down, bottom_up)
        }
    }
}

// -- ClassId ------------------------------------------------------------------

pub fn transform_ty_class_id<Ctx: Clone, Ex, En>(
    elem: &mut ClassId<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_class_id(elem, ctx) {
        return;
    }
    traverse_ty_class_id(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_class_id(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_class_id<Ctx: Clone, Ex, En>(
    elem: &mut ClassId<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_class_id_(&mut elem.2, ctx, top_down, bottom_up)
}

pub fn transform_ty_class_id_<Ctx: Clone, Ex, En>(
    elem: &mut ClassId_<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_class_id_(elem, ctx) {
        return;
    }
    traverse_ty_class_id_(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_class_id_(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_class_id_<Ctx: Clone, Ex, En>(
    elem: &mut ClassId_<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        ClassId_::CIparent | ClassId_::CIself | ClassId_::CIstatic | ClassId_::CI(_) => (),
        ClassId_::CIexpr(elem) => transform_ty_expr(elem, ctx, top_down, bottom_up),
    }
}

// -- Expr ---------------------------------------------------------------------

pub fn transform_ty_expr<Ctx: Clone, Ex, En>(
    elem: &mut Expr<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_expr(elem, ctx) {
        return;
    }
    traverse_ty_expr(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_expr(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_expr<Ctx: Clone, Ex, En>(
    elem: &mut Expr<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_expr_(&mut elem.2, ctx, top_down, bottom_up)
}

pub fn transform_ty_expr_<Ctx: Clone, Ex, En>(
    elem: &mut Expr_<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_expr_(elem, ctx) {
        return;
    }
    traverse_ty_expr_(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_expr_(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_expr_<Ctx: Clone, Ex, En>(
    elem: &mut Expr_<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        Expr_::Darray(elem) => {
            elem.0.iter_mut().for_each(|(elem0, elem1)| {
                transform_ty_targ(elem0, &mut ctx.clone(), top_down, bottom_up);
                transform_ty_targ(elem1, &mut ctx.clone(), top_down, bottom_up);
            });
            elem.1.iter_mut().for_each(|(elem0, elem1)| {
                transform_ty_expr(elem0, &mut ctx.clone(), top_down, bottom_up);
                transform_ty_expr(elem1, &mut ctx.clone(), top_down, bottom_up);
            })
        }
        Expr_::Varray(elem) => {
            elem.0.iter_mut().for_each(|elem| {
                transform_ty_targ(elem, &mut ctx.clone(), top_down, bottom_up);
            });
            elem.1.iter_mut().for_each(|elem| {
                transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up);
            })
        }
        Expr_::Shape(elem) => elem.iter_mut().for_each(|(_elem0, elem1)| {
            transform_ty_expr(elem1, &mut ctx.clone(), top_down, bottom_up)
        }),

        Expr_::ValCollection(elem) => {
            elem.1
                .iter_mut()
                .for_each(|elem| transform_ty_targ(elem, &mut ctx.clone(), top_down, bottom_up));
            elem.2
                .iter_mut()
                .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up))
        }

        Expr_::KeyValCollection(elem) => {
            elem.1.iter_mut().for_each(|(elem10, elem11)| {
                transform_ty_targ(elem10, &mut ctx.clone(), top_down, bottom_up);
                transform_ty_targ(elem11, &mut ctx.clone(), top_down, bottom_up)
            });
            elem.2
                .iter_mut()
                .for_each(|elem| transform_ty_field(elem, &mut ctx.clone(), top_down, bottom_up))
        }

        Expr_::Invalid(elem) => elem
            .iter_mut()
            .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up)),

        Expr_::Clone(elem) => transform_ty_expr(elem, ctx, top_down, bottom_up),

        Expr_::ArrayGet(elem) => {
            transform_ty_expr(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            elem.1
                .iter_mut()
                .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up))
        }

        Expr_::ObjGet(elem) => {
            transform_ty_expr(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_expr(&mut elem.1, ctx, top_down, bottom_up);
        }

        Expr_::ClassGet(elem) => {
            transform_ty_class_id(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_class_get_expr(&mut elem.1, ctx, top_down, bottom_up);
        }

        Expr_::ClassConst(elem) => {
            transform_ty_class_id(&mut elem.0, ctx, top_down, bottom_up);
        }

        Expr_::Call(elem) => {
            transform_ty_expr(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            elem.1
                .iter_mut()
                .for_each(|elem| transform_ty_targ(elem, &mut ctx.clone(), top_down, bottom_up));
            elem.2.iter_mut().for_each(|(_elem0, elem1)| {
                transform_ty_expr(elem1, &mut ctx.clone(), top_down, bottom_up)
            });
            elem.3
                .iter_mut()
                .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up))
        }

        Expr_::FunctionPointer(elem) => {
            transform_ty_funcion_ptr_id(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            elem.1
                .iter_mut()
                .for_each(|elem| transform_ty_targ(elem, &mut ctx.clone(), top_down, bottom_up))
        }

        Expr_::String2(elem) => elem
            .iter_mut()
            .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up)),

        Expr_::PrefixedString(elem) => transform_ty_expr(&mut elem.1, ctx, top_down, bottom_up),

        Expr_::Yield(elem) => transform_ty_afield(elem, ctx, top_down, bottom_up),

        Expr_::Await(elem) => transform_ty_expr(elem, ctx, top_down, bottom_up),

        Expr_::ReadonlyExpr(elem) => transform_ty_expr(elem, ctx, top_down, bottom_up),

        Expr_::Tuple(elem) => elem
            .iter_mut()
            .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up)),

        Expr_::List(elem) => elem
            .iter_mut()
            .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up)),

        Expr_::Cast(elem) => {
            transform_ty_hint(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_expr(&mut elem.1, ctx, top_down, bottom_up);
        }

        Expr_::Unop(elem) => transform_ty_expr(&mut elem.1, ctx, top_down, bottom_up),

        Expr_::Binop(elem) => {
            transform_ty_expr(&mut elem.1, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_expr(&mut elem.2, ctx, top_down, bottom_up)
        }

        Expr_::Pipe(elem) => {
            transform_ty_expr(&mut elem.1, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_expr(&mut elem.2, ctx, top_down, bottom_up)
        }

        Expr_::Eif(elem) => {
            transform_ty_expr(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            elem.1
                .iter_mut()
                .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up));
            transform_ty_expr(&mut elem.2, ctx, top_down, bottom_up)
        }

        Expr_::Is(elem) => {
            transform_ty_expr(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_hint(&mut elem.1, ctx, top_down, bottom_up);
        }

        Expr_::As(elem) => {
            transform_ty_expr(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_hint(&mut elem.1, ctx, top_down, bottom_up);
        }

        Expr_::Upcast(elem) => {
            transform_ty_expr(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_hint(&mut elem.1, ctx, top_down, bottom_up);
        }

        Expr_::New(elem) => {
            transform_ty_class_id(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
            elem.1
                .iter_mut()
                .for_each(|elem| transform_ty_targ(elem, &mut ctx.clone(), top_down, bottom_up));
            elem.2
                .iter_mut()
                .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up));
            elem.3
                .iter_mut()
                .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up))
        }

        Expr_::Efun(elem) => transform_ty_efun(elem, ctx, top_down, bottom_up),

        Expr_::Lfun(elem) => transform_ty_fun_(&mut elem.0, ctx, top_down, bottom_up),

        Expr_::Xml(elem) => {
            elem.1.iter_mut().for_each(|elem| {
                transform_ty_xhp_attribute(elem, &mut ctx.clone(), top_down, bottom_up)
            });
            elem.2
                .iter_mut()
                .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up));
        }

        Expr_::Import(elem) => transform_ty_expr(&mut elem.1, ctx, top_down, bottom_up),

        Expr_::Collection(elem) => {
            elem.1.iter_mut().for_each(|elem| {
                transform_ty_collection_targ(elem, &mut ctx.clone(), top_down, bottom_up)
            });
            elem.2
                .iter_mut()
                .for_each(|elem| transform_ty_afield(elem, &mut ctx.clone(), top_down, bottom_up))
        }

        Expr_::ExpressionTree(elem) => transform_ty_expression_tree(elem, ctx, top_down, bottom_up),

        Expr_::MethodId(elem) => transform_ty_expr(&mut elem.0, ctx, top_down, bottom_up),

        Expr_::SmethodId(elem) => transform_ty_class_id(&mut elem.0, ctx, top_down, bottom_up),

        Expr_::Pair(elem) => {
            elem.0.iter_mut().for_each(|(elem00, elem01)| {
                transform_ty_targ(elem00, &mut ctx.clone(), top_down, bottom_up);
                transform_ty_targ(elem01, &mut ctx.clone(), top_down, bottom_up)
            });
            transform_ty_expr(&mut elem.1, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_expr(&mut elem.2, ctx, top_down, bottom_up)
        }

        Expr_::ETSplice(elem) => transform_ty_expr(elem, ctx, top_down, bottom_up),

        Expr_::Hole(elem) => transform_ty_expr(&mut elem.0, ctx, top_down, bottom_up),

        Expr_::Null
        | Expr_::This
        | Expr_::True
        | Expr_::False
        | Expr_::Omitted
        | Expr_::EnumClassLabel(_)
        | Expr_::MethodCaller(_)
        | Expr_::FunId(_)
        | Expr_::Id(_)
        | Expr_::Lvar(_)
        | Expr_::Dollardollar(_)
        | Expr_::Int(_)
        | Expr_::Float(_)
        | Expr_::String(_)
        | Expr_::Lplaceholder(_) => (),
    }
}

pub fn transform_ty_collection_targ<Ctx: Clone, Ex>(
    elem: &mut CollectionTarg<Ex>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_collection_targ(elem, ctx) {
        return;
    }
    traverse_ty_collection_targ(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_collection_targ(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_collection_targ<Ctx: Clone, Ex>(
    elem: &mut CollectionTarg<Ex>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        CollectionTarg::CollectionTV(elem) => transform_ty_targ(elem, ctx, top_down, bottom_up),
        CollectionTarg::CollectionTKV(elem0, elem1) => {
            transform_ty_targ(elem0, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_targ(elem1, ctx, top_down, bottom_up)
        }
    }
}

pub fn transform_ty_funcion_ptr_id<Ctx: Clone, Ex, En>(
    elem: &mut FunctionPtrId<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_funcion_ptr_id(elem, ctx) {
        return;
    }
    traverse_ty_funcion_ptr_id(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_funcion_ptr_id(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_funcion_ptr_id<Ctx: Clone, Ex, En>(
    elem: &mut FunctionPtrId<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        FunctionPtrId::FPClassConst(elem0, _elem1) => {
            transform_ty_class_id(elem0, ctx, top_down, bottom_up)
        }
        FunctionPtrId::FPId(_) => (),
    }
}

pub fn transform_ty_expression_tree<Ctx: Clone, Ex, En>(
    elem: &mut ExpressionTree<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_expression_tree(elem, ctx) {
        return;
    }
    traverse_ty_expression_tree(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_expression_tree(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_expression_tree<Ctx: Clone, Ex, En>(
    elem: &mut ExpressionTree<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_hint(&mut elem.hint, &mut ctx.clone(), top_down, bottom_up);
    elem.splices
        .iter_mut()
        .for_each(|elem| transform_ty_stmt(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.function_pointers
        .iter_mut()
        .for_each(|elem| transform_ty_stmt(elem, &mut ctx.clone(), top_down, bottom_up));
    transform_ty_expr(
        &mut elem.virtualized_expr,
        &mut ctx.clone(),
        top_down,
        bottom_up,
    );
    transform_ty_expr(&mut elem.runtime_expr, ctx, top_down, bottom_up);
}

pub fn transform_ty_class_get_expr<Ctx: Clone, Ex, En>(
    elem: &mut ClassGetExpr<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_class_get_expr(elem, ctx) {
        return;
    }
    traverse_ty_class_get_expr(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_class_get_expr(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_class_get_expr<Ctx: Clone, Ex, En>(
    elem: &mut ClassGetExpr<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        ClassGetExpr::CGexpr(elem) => transform_ty_expr(elem, ctx, top_down, bottom_up),
        ClassGetExpr::CGstring(_) => (),
    }
}

pub fn transform_ty_field<Ctx: Clone, Ex, En>(
    elem: &mut Field<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_field(elem, ctx) {
        return;
    }
    traverse_ty_field(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_field(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_field<Ctx: Clone, Ex, En>(
    elem: &mut Field<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_expr(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
    transform_ty_expr(&mut elem.1, ctx, top_down, bottom_up)
}

pub fn transform_ty_afield<Ctx: Clone, Ex, En>(
    elem: &mut Afield<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_afield(elem, ctx) {
        return;
    }
    traverse_ty_afield(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_afield(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_afield<Ctx: Clone, Ex, En>(
    elem: &mut Afield<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        Afield::AFvalue(elem) => transform_ty_expr(elem, ctx, top_down, bottom_up),
        Afield::AFkvalue(elem0, elem1) => {
            transform_ty_expr(elem0, &mut ctx.clone(), top_down, bottom_up);
            transform_ty_expr(elem1, ctx, top_down, bottom_up);
        }
    }
}

pub fn transform_ty_xhp_simple<Ctx: Clone, Ex, En>(
    elem: &mut XhpSimple<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_xhp_simple(elem, ctx) {
        return;
    }
    traverse_ty_xhp_simple(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_xhp_simple(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_xhp_simple<Ctx: Clone, Ex, En>(
    elem: &mut XhpSimple<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_expr(&mut elem.expr, ctx, top_down, bottom_up)
}

pub fn transform_ty_xhp_attribute<Ctx: Clone, Ex, En>(
    elem: &mut XhpAttribute<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_xhp_attribute(elem, ctx) {
        return;
    }
    traverse_ty_xhp_attribute(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_xhp_attribute(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_xhp_attribute<Ctx: Clone, Ex, En>(
    elem: &mut XhpAttribute<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        XhpAttribute::XhpSimple(elem) => transform_ty_xhp_simple(elem, ctx, top_down, bottom_up),
        XhpAttribute::XhpSpread(elem) => transform_ty_expr(elem, ctx, top_down, bottom_up),
    }
}

pub fn transform_ty_xhp_attr<Ctx: Clone, Ex, En>(
    elem: &mut XhpAttr<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_xhp_attr(elem, ctx) {
        return;
    }
    traverse_ty_xhp_attr(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_xhp_attr(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_xhp_attr<Ctx: Clone, Ex, En>(
    elem: &mut XhpAttr<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_type_hint(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
    transform_ty_class_var(&mut elem.1, &mut ctx.clone(), top_down, bottom_up);
    elem.3.iter_mut().for_each(|elem| {
        elem.1
            .iter_mut()
            .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up))
    })
}
// -- Fun_ ---------------------------------------------------------------------

pub fn transform_ty_fun_<Ctx: Clone, Ex, En>(
    elem: &mut Fun_<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_fun_(elem, ctx) {
        return;
    }
    traverse_ty_fun_(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_fun_(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_fun_<Ctx: Clone, Ex, En>(
    elem: &mut Fun_<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_fld_fun__ret(&mut elem.ret, &mut ctx.clone(), top_down, bottom_up);
    elem.tparams
        .iter_mut()
        .for_each(|elem| transform_ty_tparam(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.where_constraints.iter_mut().for_each(|elem| {
        transform_ty_where_constraint_hint(elem, &mut ctx.clone(), top_down, bottom_up)
    });
    elem.params
        .iter_mut()
        .for_each(|elem| transform_ty_fun_param(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.ctxs
        .iter_mut()
        .for_each(|elem| transform_ty_contexts(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.unsafe_ctxs
        .iter_mut()
        .for_each(|elem| transform_ty_contexts(elem, &mut ctx.clone(), top_down, bottom_up));
    transform_ty_func_body(&mut elem.body, &mut ctx.clone(), top_down, bottom_up);
    transform_ty_user_attributes(&mut elem.user_attributes, ctx, top_down, bottom_up);
}

#[allow(non_snake_case)]
pub fn transform_fld_fun__ret<Ctx: Clone, Ex>(
    elem: &mut TypeHint<Ex>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_fun__ret(elem, ctx) {
        return;
    }
    traverse_fld_fun__ret(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_fun__ret(elem, &mut in_ctx);
}

#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_fun__ret<Ctx: Clone, Ex>(
    elem: &mut TypeHint<Ex>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_type_hint(elem, ctx, top_down, bottom_up)
}

// -- Method_ ------------------------------------------------------------------

pub fn transform_ty_method_<Ctx: Clone, Ex, En>(
    elem: &mut Method_<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_method_(elem, ctx) {
        return;
    }
    traverse_ty_method_(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_method_(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_method_<Ctx: Clone, Ex, En>(
    elem: &mut Method_<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_fld_method__ret(&mut elem.ret, &mut ctx.clone(), top_down, bottom_up);
    elem.tparams
        .iter_mut()
        .for_each(|elem| transform_ty_tparam(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.where_constraints.iter_mut().for_each(|elem| {
        transform_ty_where_constraint_hint(elem, &mut ctx.clone(), top_down, bottom_up)
    });
    elem.params
        .iter_mut()
        .for_each(|elem| transform_ty_fun_param(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.ctxs
        .iter_mut()
        .for_each(|elem| transform_ty_contexts(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.unsafe_ctxs
        .iter_mut()
        .for_each(|elem| transform_ty_contexts(elem, &mut ctx.clone(), top_down, bottom_up));
    transform_ty_func_body(&mut elem.body, &mut ctx.clone(), top_down, bottom_up);
    transform_ty_user_attributes(&mut elem.user_attributes, ctx, top_down, bottom_up);
}

#[allow(non_snake_case)]
pub fn transform_fld_method__ret<Ctx: Clone, Ex>(
    elem: &mut TypeHint<Ex>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_method__ret(elem, ctx) {
        return;
    }
    traverse_fld_method__ret(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_method__ret(elem, &mut in_ctx);
}

#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_method__ret<Ctx: Clone, Ex>(
    elem: &mut TypeHint<Ex>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_type_hint(elem, ctx, top_down, bottom_up)
}

// -- FunParam -----------------------------------------------------------------

pub fn transform_ty_fun_param<Ctx: Clone, Ex, En>(
    elem: &mut FunParam<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_fun_param(elem, ctx) {
        return;
    }
    traverse_ty_fun_param(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_fun_param(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_fun_param<Ctx: Clone, Ex, En>(
    elem: &mut FunParam<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_type_hint(&mut elem.type_hint, &mut ctx.clone(), top_down, bottom_up);
    elem.expr
        .iter_mut()
        .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up));
    transform_ty_user_attributes(&mut elem.user_attributes, ctx, top_down, bottom_up)
}

// -- Efun ---------------------------------------------------------------------

pub fn transform_ty_efun<Ctx: Clone, Ex, En>(
    elem: &mut Efun<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_efun(elem, ctx) {
        return;
    }
    traverse_ty_efun(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_efun(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_efun<Ctx: Clone, Ex, En>(
    elem: &mut Efun<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_fun_(&mut elem.fun, ctx, top_down, bottom_up)
}

// -- FuncBody -----------------------------------------------------------------

pub fn transform_ty_func_body<Ctx: Clone, Ex, En>(
    elem: &mut FuncBody<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_func_body(elem, ctx) {
        return;
    }
    traverse_ty_func_body(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_func_body(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_func_body<Ctx: Clone, Ex, En>(
    elem: &mut FuncBody<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_block(&mut elem.fb_ast, ctx, top_down, bottom_up)
}

// -- Class_ -------------------------------------------------------------------

pub fn transform_ty_class_<Ctx: Clone, Ex, En>(
    elem: &mut Class_<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_class_(elem, ctx) {
        return;
    }
    traverse_ty_class_(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_class_(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_class_<Ctx: Clone, Ex, En>(
    elem: &mut Class_<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_fld_class__tparams(&mut elem.tparams, &mut ctx.clone(), top_down, bottom_up);
    transform_fld_class__extends(&mut elem.extends, &mut ctx.clone(), top_down, bottom_up);
    transform_fld_class__uses(&mut elem.uses, &mut ctx.clone(), top_down, bottom_up);
    transform_fld_class__xhp_attr_uses(
        &mut elem.xhp_attr_uses,
        &mut ctx.clone(),
        top_down,
        bottom_up,
    );
    transform_fld_class__reqs(&mut elem.reqs, &mut ctx.clone(), top_down, bottom_up);
    transform_fld_class__implements(&mut elem.implements, &mut ctx.clone(), top_down, bottom_up);
    elem.where_constraints.iter_mut().for_each(|elem| {
        transform_ty_where_constraint_hint(elem, &mut ctx.clone(), top_down, bottom_up)
    });
    transform_fld_class__consts(&mut elem.consts, &mut ctx.clone(), top_down, bottom_up);
    elem.typeconsts.iter_mut().for_each(|elem| {
        transform_ty_class_typeconst_def(elem, &mut ctx.clone(), top_down, bottom_up)
    });
    elem.vars
        .iter_mut()
        .for_each(|elem| transform_ty_class_var(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.methods
        .iter_mut()
        .for_each(|elem| transform_ty_method_(elem, &mut ctx.clone(), top_down, bottom_up));
    transform_fld_class__xhp_attrs(&mut elem.xhp_attrs, &mut ctx.clone(), top_down, bottom_up);
    transform_fld_class__user_attributes(
        &mut elem.user_attributes,
        &mut ctx.clone(),
        top_down,
        bottom_up,
    );
    elem.file_attributes
        .iter_mut()
        .for_each(|elem| transform_ty_file_attribute(elem, &mut ctx.clone(), top_down, bottom_up));
}

#[allow(non_snake_case)]
pub fn transform_fld_class__tparams<Ctx: Clone, Ex, En>(
    elem: &mut Vec<Tparam<Ex, En>>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class__tparams(elem, ctx) {
        return;
    }
    traverse_fld_class__tparams(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class__tparams(elem, &mut in_ctx);
}

#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_class__tparams<Ctx: Clone, Ex, En>(
    elem: &mut [Tparam<Ex, En>],
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_tparam(elem, &mut ctx.clone(), top_down, bottom_up))
}

#[allow(non_snake_case)]
pub fn transform_fld_class__extends<Ctx: Clone>(
    elem: &mut Vec<ClassHint>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class__extends(elem, ctx) {
        return;
    }
    traverse_fld_class__extends(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class__extends(elem, &mut in_ctx);
}
#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_class__extends<Ctx: Clone>(
    elem: &mut [ClassHint],
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_class_hint(elem, &mut ctx.clone(), top_down, bottom_up))
}

#[allow(non_snake_case)]
pub fn transform_fld_class__uses<Ctx: Clone>(
    elem: &mut Vec<TraitHint>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class__uses(elem, ctx) {
        return;
    }
    traverse_fld_class__uses(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class__uses(elem, &mut in_ctx);
}
#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_class__uses<Ctx: Clone>(
    elem: &mut [TraitHint],
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_trait_hint(elem, &mut ctx.clone(), top_down, bottom_up))
}

#[allow(non_snake_case)]
pub fn transform_fld_class__xhp_attr_uses<Ctx: Clone>(
    elem: &mut Vec<XhpAttrHint>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class__xhp_attr_uses(elem, ctx) {
        return;
    }
    traverse_fld_class__xhp_attr_uses(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class__xhp_attr_uses(elem, &mut in_ctx);
}

#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_class__xhp_attr_uses<Ctx: Clone>(
    elem: &mut [XhpAttrHint],
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_xhp_attr_hint(elem, &mut ctx.clone(), top_down, bottom_up))
}

#[allow(non_snake_case)]
pub fn transform_fld_class__reqs<Ctx: Clone>(
    elem: &mut Vec<(ClassHint, RequireKind)>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class__reqs(elem, ctx) {
        return;
    }
    traverse_fld_class__reqs(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class__reqs(elem, &mut in_ctx);
}

#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_class__reqs<Ctx: Clone>(
    elem: &mut [(ClassHint, RequireKind)],
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut().for_each(|(elem0, _elem1)| {
        transform_ty_class_hint(elem0, &mut ctx.clone(), top_down, bottom_up)
    })
}

#[allow(non_snake_case)]
pub fn transform_fld_class__implements<Ctx: Clone>(
    elem: &mut Vec<ClassHint>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class__implements(elem, ctx) {
        return;
    }
    traverse_fld_class__implements(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class__implements(elem, &mut in_ctx);
}

#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_class__implements<Ctx: Clone>(
    elem: &mut [ClassHint],
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_class_hint(elem, &mut ctx.clone(), top_down, bottom_up))
}

#[allow(non_snake_case)]
pub fn transform_fld_class__where_constraints<Ctx: Clone>(
    elem: &mut Vec<WhereConstraintHint>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class__where_constraints(elem, ctx) {
        return;
    }
    traverse_fld_class__where_constraints(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class__where_constraints(elem, &mut in_ctx);
}

#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_class__where_constraints<Ctx: Clone>(
    elem: &mut [WhereConstraintHint],
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut().for_each(|elem| {
        transform_ty_where_constraint_hint(elem, &mut ctx.clone(), top_down, bottom_up)
    })
}

#[allow(non_snake_case)]
pub fn transform_fld_class__consts<Ctx: Clone, Ex, En>(
    elem: &mut Vec<ClassConst<Ex, En>>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class__consts(elem, ctx) {
        return;
    }
    traverse_fld_class__consts(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class__consts(elem, &mut in_ctx);
}

#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_class__consts<Ctx: Clone, Ex, En>(
    elem: &mut [ClassConst<Ex, En>],
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_class_const(elem, &mut ctx.clone(), top_down, bottom_up))
}

#[allow(non_snake_case)]
pub fn transform_fld_class__typeconsts<Ctx: Clone, Ex, En>(
    elem: &mut Vec<ClassTypeconstDef<Ex, En>>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class__typeconsts(elem, ctx) {
        return;
    }
    traverse_fld_class__typeconsts(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class__typeconsts(elem, &mut in_ctx);
}

#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_class__typeconsts<Ctx: Clone, Ex, En>(
    elem: &mut [ClassTypeconstDef<Ex, En>],
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut().for_each(|elem| {
        transform_ty_class_typeconst_def(elem, &mut ctx.clone(), top_down, bottom_up)
    })
}

#[allow(non_snake_case)]
pub fn transform_fld_class__vars<Ctx: Clone, Ex, En>(
    elem: &mut Vec<ClassVar<Ex, En>>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class__vars(elem, ctx) {
        return;
    }
    traverse_fld_class__vars(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class__vars(elem, &mut in_ctx);
}

#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_class__vars<Ctx: Clone, Ex, En>(
    elem: &mut [ClassVar<Ex, En>],
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_class_var(elem, &mut ctx.clone(), top_down, bottom_up))
}

#[allow(non_snake_case)]
pub fn transform_fld_class__methods<Ctx: Clone, Ex, En>(
    elem: &mut Vec<Method_<Ex, En>>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class__methods(elem, ctx) {
        return;
    }
    traverse_fld_class__methods(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class__methods(elem, &mut in_ctx);
}

#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_class__methods<Ctx: Clone, Ex, En>(
    elem: &mut [Method_<Ex, En>],
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_method_(elem, &mut ctx.clone(), top_down, bottom_up))
}

#[allow(non_snake_case)]
pub fn transform_fld_class__xhp_attrs<Ctx: Clone, Ex, En>(
    elem: &mut Vec<XhpAttr<Ex, En>>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class__xhp_attrs(elem, ctx) {
        return;
    }
    traverse_fld_class__xhp_attrs(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class__xhp_attrs(elem, &mut in_ctx);
}

#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_class__xhp_attrs<Ctx: Clone, Ex, En>(
    elem: &mut [XhpAttr<Ex, En>],
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_xhp_attr(elem, &mut ctx.clone(), top_down, bottom_up))
}

#[allow(non_snake_case)]
pub fn transform_fld_class__user_attributes<Ctx: Clone, Ex, En>(
    elem: &mut UserAttributes<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class__user_attributes(elem, ctx) {
        return;
    }
    traverse_fld_class__user_attributes(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class__user_attributes(elem, &mut in_ctx);
}

#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_class__user_attributes<Ctx: Clone, Ex, En>(
    elem: &mut UserAttributes<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_user_attributes(elem, ctx, top_down, bottom_up)
}

#[allow(non_snake_case)]
pub fn transform_fld_class__enum_<Ctx: Clone>(
    elem: &mut Option<Enum_>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class__enum_(elem, ctx) {
        return;
    }
    traverse_fld_class__enum_(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class__enum_(elem, &mut in_ctx);
}

#[inline(always)]
#[allow(non_snake_case)]
fn traverse_fld_class__enum_<Ctx: Clone>(
    elem: &mut Option<Enum_>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_enum_(elem, &mut ctx.clone(), top_down, bottom_up))
}
// -- ClassVar -----------------------------------------------------------------

pub fn transform_ty_class_var<Ctx: Clone, Ex, En>(
    elem: &mut ClassVar<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_class_var(elem, ctx) {
        return;
    }
    traverse_ty_class_var(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_class_var(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_class_var<Ctx: Clone, Ex, En>(
    elem: &mut ClassVar<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_fld_class_var_type_(&mut elem.type_, &mut ctx.clone(), top_down, bottom_up);
    transform_fld_class_var_expr(&mut elem.expr, &mut ctx.clone(), top_down, bottom_up);
    transform_ty_user_attributes(&mut elem.user_attributes, ctx, top_down, bottom_up)
}

pub fn transform_fld_class_var_type_<Ctx: Clone, Ex>(
    elem: &mut TypeHint<Ex>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class_var_type_(elem, ctx) {
        return;
    }
    traverse_fld_class_var_type_(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class_var_type_(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_fld_class_var_type_<Ctx: Clone, Ex>(
    elem: &mut TypeHint<Ex>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_type_hint(elem, ctx, top_down, bottom_up)
}

pub fn transform_fld_class_var_expr<Ctx: Clone, Ex, En>(
    elem: &mut Option<Expr<Ex, En>>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_class_var_expr(elem, ctx) {
        return;
    }
    traverse_fld_class_var_expr(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_class_var_expr(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_fld_class_var_expr<Ctx: Clone, Ex, En>(
    elem: &mut Option<Expr<Ex, En>>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up))
}

// -- ClassConst ---------------------------------------------------------------

pub fn transform_ty_class_const<Ctx: Clone, Ex, En>(
    elem: &mut ClassConst<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_class_const(elem, ctx) {
        return;
    }
    traverse_ty_class_const(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_class_const(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_class_const<Ctx: Clone, Ex, En>(
    elem: &mut ClassConst<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_user_attributes(
        &mut elem.user_attributes,
        &mut ctx.clone(),
        top_down,
        bottom_up,
    );
    elem.type_
        .iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up));
    transform_ty_class_const_kind(&mut elem.kind, ctx, top_down, bottom_up)
}

pub fn transform_ty_class_const_kind<Ctx: Clone, Ex, En>(
    elem: &mut ClassConstKind<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_class_const_kind(elem, ctx) {
        return;
    }
    traverse_ty_class_const_kind(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_class_const_kind(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_class_const_kind<Ctx: Clone, Ex, En>(
    elem: &mut ClassConstKind<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        ClassConstKind::CCAbstract(elem) => elem
            .iter_mut()
            .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up)),
        ClassConstKind::CCConcrete(elem) => transform_ty_expr(elem, ctx, top_down, bottom_up),
    }
}

// -- ClassTypeconstDef --------------------------------------------------------
pub fn transform_ty_class_typeconst_def<Ctx: Clone, Ex, En>(
    elem: &mut ClassTypeconstDef<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_class_typeconst_def(elem, ctx) {
        return;
    }
    traverse_ty_class_typeconst_def(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_class_typeconst_def(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_class_typeconst_def<Ctx: Clone, Ex, En>(
    elem: &mut ClassTypeconstDef<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_user_attributes(
        &mut elem.user_attributes,
        &mut ctx.clone(),
        top_down,
        bottom_up,
    );
    transform_ty_class_typeconst(&mut elem.kind, ctx, top_down, bottom_up)
}

pub fn transform_ty_class_typeconst<Ctx: Clone>(
    elem: &mut ClassTypeconst,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_class_typeconst(elem, ctx) {
        return;
    }
    traverse_ty_class_typeconst(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_class_typeconst(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_class_typeconst<Ctx: Clone>(
    elem: &mut ClassTypeconst,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        ClassTypeconst::TCAbstract(elem) => {
            transform_ty_class_abstract_typeconst(elem, ctx, top_down, bottom_up)
        }
        ClassTypeconst::TCConcrete(elem) => {
            transform_ty_class_concrete_typeconst(elem, ctx, top_down, bottom_up)
        }
    }
}

pub fn transform_ty_class_abstract_typeconst<Ctx: Clone>(
    elem: &mut ClassAbstractTypeconst,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_class_abstract_typeconst(elem, ctx) {
        return;
    }
    traverse_ty_class_abstract_typeconst(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_class_abstract_typeconst(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_class_abstract_typeconst<Ctx: Clone>(
    elem: &mut ClassAbstractTypeconst,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.as_constraint
        .iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.super_constraint
        .iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.default
        .iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up))
}

pub fn transform_ty_class_concrete_typeconst<Ctx: Clone>(
    elem: &mut ClassConcreteTypeconst,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_class_concrete_typeconst(elem, ctx) {
        return;
    }
    traverse_ty_class_concrete_typeconst(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_class_concrete_typeconst(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_class_concrete_typeconst<Ctx: Clone>(
    elem: &mut ClassConcreteTypeconst,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_hint(&mut elem.c_tc_type, ctx, top_down, bottom_up)
}

pub fn transform_ty_context<Ctx: Clone>(
    elem: &mut Context,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_context(elem, ctx) {
        return;
    }
    traverse_ty_context(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_context(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_context<Ctx: Clone>(
    elem: &mut Context,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_hint(elem, ctx, top_down, bottom_up)
}

// -- WhereConstraintHint ------------------------------------------------------

pub fn transform_ty_where_constraint_hint<Ctx: Clone>(
    elem: &mut WhereConstraintHint,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_where_constraint_hint(elem, ctx) {
        return;
    }
    traverse_ty_where_constraint_hint(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_where_constraint_hint(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_where_constraint_hint<Ctx: Clone>(
    elem: &mut WhereConstraintHint,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_hint(&mut elem.0, &mut ctx.clone(), top_down, bottom_up);
    transform_ty_hint(&mut elem.2, ctx, top_down, bottom_up)
}

// -- Enum ---------------------------------------------------------------------

pub fn transform_ty_enum_<Ctx: Clone>(
    elem: &mut Enum_,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_enum_(elem, ctx) {
        return;
    }
    traverse_ty_enum_(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_enum_(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_enum_<Ctx: Clone>(
    elem: &mut Enum_,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_hint(&mut elem.base, &mut ctx.clone(), top_down, bottom_up);
    elem.constraint
        .iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.includes
        .iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up));
}

// -- TypeHint<Ex> -------------------------------------------------------------

pub fn transform_ty_type_hint<Ctx: Clone, Ex>(
    elem: &mut TypeHint<Ex>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_type_hint(elem, ctx) {
        return;
    }
    traverse_ty_type_hint(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_type_hint(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_type_hint<Ctx: Clone, Ex>(
    elem: &mut TypeHint<Ex>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_type_hint_(&mut elem.1, ctx, top_down, bottom_up)
}

pub fn transform_ty_type_hint_<Ctx: Clone>(
    elem: &mut TypeHint_,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_type_hint_(elem, ctx) {
        return;
    }
    traverse_ty_type_hint_(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_type_hint_(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_type_hint_<Ctx: Clone>(
    elem: &mut TypeHint_,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up))
}

// -- Targ ---------------------------------------------------------------------

pub fn transform_ty_targ<Ctx: Clone, Ex>(
    elem: &mut Targ<Ex>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_targ(elem, ctx) {
        return;
    }
    traverse_ty_targ(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_targ(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_targ<Ctx: Clone, Ex>(
    elem: &mut Targ<Ex>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_hint(&mut elem.1, ctx, top_down, bottom_up)
}
// -- Tparam -------------------------------------------------------------------

pub fn transform_ty_tparam<Ctx: Clone, Ex, En>(
    elem: &mut Tparam<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_tparam(elem, ctx) {
        return;
    }
    traverse_ty_tparam(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_tparam(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_tparam<Ctx: Clone, Ex, En>(
    elem: &mut Tparam<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.parameters
        .iter_mut()
        .for_each(|elem| transform_ty_tparam(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.constraints.iter_mut().for_each(|(_elem0, elem1)| {
        transform_ty_hint(elem1, &mut ctx.clone(), top_down, bottom_up)
    });
    transform_ty_user_attributes(&mut elem.user_attributes, ctx, top_down, bottom_up)
}

// -- UserAttribute(s) ---------------------------------------------------------

pub fn transform_ty_user_attributes<Ctx: Clone, Ex, En>(
    elem: &mut UserAttributes<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_user_attributes(elem, ctx) {
        return;
    }
    traverse_ty_user_attributes(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_user_attributes(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_user_attributes<Ctx: Clone, Ex, En>(
    elem: &mut UserAttributes<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_user_attribute(elem, &mut ctx.clone(), top_down, bottom_up))
}

pub fn transform_ty_user_attribute<Ctx: Clone, Ex, En>(
    elem: &mut UserAttribute<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_user_attribute(elem, ctx) {
        return;
    }
    traverse_ty_user_attribute(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_user_attribute(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_user_attribute<Ctx: Clone, Ex, En>(
    elem: &mut UserAttribute<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.params
        .iter_mut()
        .for_each(|elem| transform_ty_expr(elem, &mut ctx.clone(), top_down, bottom_up));
}

// -- FileAttribute ------------------------------------------------------------

pub fn transform_ty_file_attribute<Ctx: Clone, Ex, En>(
    elem: &mut FileAttribute<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_file_attribute(elem, ctx) {
        return;
    }
    traverse_ty_file_attribute(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_file_attribute(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_file_attribute<Ctx: Clone, Ex, En>(
    elem: &mut FileAttribute<Ex, En>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_user_attributes(&mut elem.user_attributes, ctx, top_down, bottom_up)
}

// -- Hints --------------------------------------------------------------------

pub fn transform_ty_class_hint<Ctx: Clone>(
    elem: &mut Hint,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_class_hint(elem, ctx) {
        return;
    }
    traverse_ty_class_hint(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_class_hint(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_class_hint<Ctx: Clone>(
    elem: &mut Hint,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_hint(elem, ctx, top_down, bottom_up)
}

pub fn transform_ty_trait_hint<Ctx: Clone>(
    elem: &mut Hint,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_trait_hint(elem, ctx) {
        return;
    }
    traverse_ty_trait_hint(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_trait_hint(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_trait_hint<Ctx: Clone>(
    elem: &mut Hint,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_hint(elem, ctx, top_down, bottom_up)
}

pub fn transform_ty_xhp_attr_hint<Ctx: Clone>(
    elem: &mut Hint,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_xhp_attr_hint(elem, ctx) {
        return;
    }
    traverse_ty_xhp_attr_hint(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_xhp_attr_hint(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_xhp_attr_hint<Ctx: Clone>(
    elem: &mut Hint,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_hint(elem, ctx, top_down, bottom_up)
}

// -- Hint ---------------------------------------------------------------------

pub fn transform_ty_hint<Ctx: Clone>(
    elem: &mut Hint,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_hint(elem, ctx) {
        return;
    }
    traverse_ty_hint(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_hint(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_hint<Ctx: Clone>(
    elem: &mut Hint,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_hint_(&mut elem.1, ctx, top_down, bottom_up)
}

// -- Hint_ --------------------------------------------------------------------

pub fn transform_ty_hint_<Ctx: Clone>(
    elem: &mut Hint_,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_hint_(elem, ctx) {
        return;
    }
    traverse_ty_hint_(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_hint_(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_hint_<Ctx: Clone>(
    elem: &mut Hint_,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        Hint_::Hoption(elem)
        | Hint_::Hlike(elem)
        | Hint_::Hsoft(elem)
        | Hint_::Haccess(elem, _) => transform_ty_hint(elem, ctx, top_down, bottom_up),
        Hint_::Htuple(elem)
        | Hint_::Hunion(elem)
        | Hint_::Hintersection(elem)
        | Hint_::Happly(_, elem)
        | Hint_::Habstr(_, elem) => elem
            .iter_mut()
            .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up)),
        Hint_::Hrefinement(elem0, elem1) => {
            transform_ty_hint(elem0, &mut ctx.clone(), top_down, bottom_up);
            elem1.iter_mut().for_each(|elem| {
                transform_ty_refinement(elem, &mut ctx.clone(), top_down, bottom_up)
            })
        }
        Hint_::HvecOrDict(elem0, elem1) => {
            elem0
                .iter_mut()
                .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up));
            transform_ty_hint(elem1, ctx, top_down, bottom_up)
        }
        Hint_::Hfun(elem) => transform_ty_hint_fun(elem, ctx, top_down, bottom_up),
        Hint_::Hshape(elem) => transform_ty_nast_shape_info(elem, ctx, top_down, bottom_up),
        Hint_::Hany
        | Hint_::Herr
        | Hint_::Hmixed
        | Hint_::Hnonnull
        | Hint_::Hthis
        | Hint_::Hdynamic
        | Hint_::Hnothing
        | Hint_::Hprim(_)
        | Hint_::HfunContext(_)
        | Hint_::Hvar(_) => (),
    }
}

// -- HintFun ------------------------------------------------------------------

pub fn transform_ty_hint_fun<Ctx: Clone>(
    elem: &mut HintFun,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_hint_fun(elem, ctx) {
        return;
    }
    traverse_ty_hint_fun(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_hint_fun(elem, &mut in_ctx);
}

#[inline(always)]

fn traverse_ty_hint_fun<Ctx: Clone>(
    elem: &mut HintFun,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_fld_hint_fun_param_tys(&mut elem.param_tys, &mut ctx.clone(), top_down, bottom_up);
    transform_fld_hint_fun_variadic_ty(
        &mut elem.variadic_ty,
        &mut ctx.clone(),
        top_down,
        bottom_up,
    );
    transform_fld_hint_fun_ctxs(&mut elem.ctxs, &mut ctx.clone(), top_down, bottom_up);
    transform_fld_hint_fun_return_ty(&mut elem.return_ty, ctx, top_down, bottom_up);
}

// -- HintFun.param_tys --------------------------------------------------------

pub fn transform_fld_hint_fun_param_tys<Ctx: Clone>(
    elem: &mut Vec<Hint>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_hint_fun_param_tys(elem, ctx) {
        return;
    }
    traverse_fld_hint_fun_param_tys(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_hint_fun_param_tys(elem, &mut in_ctx);
}

#[inline(always)]

fn traverse_fld_hint_fun_param_tys<Ctx: Clone>(
    elem: &mut [Hint],
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up))
}

// -- HintFun.variadic_ty-------------------------------------------------------

pub fn transform_fld_hint_fun_variadic_ty<Ctx: Clone>(
    elem: &mut VariadicHint,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_hint_fun_variadic_ty(elem, ctx) {
        return;
    }
    traverse_fld_hint_fun_variadic_ty(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_hint_fun_variadic_ty(elem, &mut in_ctx);
}

#[inline(always)]

fn traverse_fld_hint_fun_variadic_ty<Ctx: Clone>(
    elem: &mut VariadicHint,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up))
}

// -- HintFun.ctxs--------------------------------------------------------------

pub fn transform_fld_hint_fun_ctxs<Ctx: Clone>(
    elem: &mut Option<Contexts>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_hint_fun_ctxs(elem, ctx) {
        return;
    }
    traverse_fld_hint_fun_ctxs(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_hint_fun_ctxs(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_fld_hint_fun_ctxs<Ctx: Clone>(
    elem: &mut Option<Contexts>,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.iter_mut()
        .for_each(|elem| transform_ty_contexts(elem, &mut ctx.clone(), top_down, bottom_up))
}

// -- HintFun.return_ty --------------------------------------------------------

pub fn transform_fld_hint_fun_return_ty<Ctx: Clone>(
    elem: &mut Hint,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_fld_hint_fun_return_ty(elem, ctx) {
        return;
    }
    traverse_fld_hint_fun_return_ty(elem, ctx, top_down, bottom_up);
    bottom_up.on_fld_hint_fun_return_ty(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_fld_hint_fun_return_ty<Ctx: Clone>(
    elem: &mut Hint,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_hint(elem, ctx, top_down, bottom_up)
}

// -- Contexts -----------------------------------------------------------------

pub fn transform_ty_contexts<Ctx: Clone>(
    elem: &mut Contexts,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_contexts(elem, ctx) {
        return;
    }
    traverse_ty_contexts(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_contexts(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_contexts<Ctx: Clone>(
    elem: &mut Contexts,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.1
        .iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up))
}

// -- Refinement ---------------------------------------------------------------

pub fn transform_ty_refinement<Ctx: Clone>(
    elem: &mut Refinement,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_refinement(elem, ctx) {
        return;
    }
    traverse_ty_refinement(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_refinement(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_refinement<Ctx: Clone>(
    elem: &mut Refinement,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        Refinement::Rctx(_, elem) => transform_ty_ctx_refinement(elem, ctx, top_down, bottom_up),
        Refinement::Rtype(_, elem) => transform_ty_type_refinement(elem, ctx, top_down, bottom_up),
    }
}

// -- CtxRefinement ------------------------------------------------------------

pub fn transform_ty_ctx_refinement<Ctx: Clone>(
    elem: &mut CtxRefinement,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_ctx_refinement(elem, ctx) {
        return;
    }
    traverse_ty_ctx_refinement(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_ctx_refinement(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_ctx_refinement<Ctx: Clone>(
    elem: &mut CtxRefinement,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        CtxRefinement::CRexact(elem) => transform_ty_hint(elem, ctx, top_down, bottom_up),
        CtxRefinement::CRloose(elem) => {
            transform_ty_ctx_refinement_bounds(elem, ctx, top_down, bottom_up)
        }
    }
}

// -- TypeRefinement -----------------------------------------------------------

pub fn transform_ty_type_refinement<Ctx: Clone>(
    elem: &mut TypeRefinement,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_type_refinement(elem, ctx) {
        return;
    }
    traverse_ty_type_refinement(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_type_refinement(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_type_refinement<Ctx: Clone>(
    elem: &mut TypeRefinement,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    match elem {
        TypeRefinement::TRexact(elem) => transform_ty_hint(elem, ctx, top_down, bottom_up),
        TypeRefinement::TRloose(elem) => {
            transform_ty_type_refinement_bounds(elem, ctx, top_down, bottom_up)
        }
    }
}

// -- CtxRefinementBounds ------------------------------------------------------

pub fn transform_ty_ctx_refinement_bounds<Ctx: Clone>(
    elem: &mut CtxRefinementBounds,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_ctx_refinement_bounds(elem, ctx) {
        return;
    }
    traverse_ty_ctx_refinement_bounds(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_ctx_refinement_bounds(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_ctx_refinement_bounds<Ctx: Clone>(
    elem: &mut CtxRefinementBounds,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.lower
        .iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.upper
        .iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up))
}

// -- TypeRefinementBounds -----------------------------------------------------

pub fn transform_ty_type_refinement_bounds<Ctx: Clone>(
    elem: &mut TypeRefinementBounds,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_type_refinement_bounds(elem, ctx) {
        return;
    }
    traverse_ty_type_refinement_bounds(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_type_refinement_bounds(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_type_refinement_bounds<Ctx: Clone>(
    elem: &mut TypeRefinementBounds,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.lower
        .iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up));
    elem.upper
        .iter_mut()
        .for_each(|elem| transform_ty_hint(elem, &mut ctx.clone(), top_down, bottom_up))
}

// -- NastShapeInfo ------------------------------------------------------------

pub fn transform_ty_nast_shape_info<Ctx: Clone>(
    elem: &mut NastShapeInfo,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_nast_shape_info(elem, ctx) {
        return;
    }
    traverse_ty_nast_shape_info(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_nast_shape_info(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_nast_shape_info<Ctx: Clone>(
    elem: &mut NastShapeInfo,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    elem.field_map
        .iter_mut()
        .for_each(|elem| transform_ty_shape_field_info(elem, &mut ctx.clone(), top_down, bottom_up))
}

// -- ShapeFieldInfo -----------------------------------------------------------

pub fn transform_ty_shape_field_info<Ctx: Clone>(
    elem: &mut ShapeFieldInfo,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    let mut in_ctx = ctx.clone();
    if let ControlFlow::Break(..) = top_down.on_ty_shape_field_info(elem, ctx) {
        return;
    }
    traverse_ty_shape_field_info(elem, ctx, top_down, bottom_up);
    bottom_up.on_ty_shape_field_info(elem, &mut in_ctx);
}

#[inline(always)]
fn traverse_ty_shape_field_info<Ctx: Clone>(
    elem: &mut ShapeFieldInfo,
    ctx: &mut Ctx,
    top_down: &impl Pass<Ctx = Ctx>,
    bottom_up: &impl Pass<Ctx = Ctx>,
) {
    transform_ty_hint(&mut elem.hint, ctx, top_down, bottom_up)
}

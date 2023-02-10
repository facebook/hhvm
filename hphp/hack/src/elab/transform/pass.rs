// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<c28261219bcdfb1f63c48a4a33f17657>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_variables)]
use std::ops::ControlFlow;
use std::ops::ControlFlow::Continue;

use oxidized::aast_defs::*;
use oxidized::ast_defs::*;
pub trait Pass {
    type Ctx: Clone;
    type Err;
    #[inline(always)]
    fn on_ty_lid(
        &self,
        elem: &mut Lid,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_program<Ex, En>(
        &self,
        elem: &mut Program<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt<Ex, En>(
        &self,
        elem: &mut Stmt<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_<Ex, En>(
        &self,
        elem: &mut Stmt_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_env_annot(
        &self,
        elem: &mut EnvAnnot,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_using_stmt<Ex, En>(
        &self,
        elem: &mut UsingStmt<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_as_expr<Ex, En>(
        &self,
        elem: &mut AsExpr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_block<Ex, En>(
        &self,
        elem: &mut Block<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_id<Ex, En>(
        &self,
        elem: &mut ClassId<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_id_<Ex, En>(
        &self,
        elem: &mut ClassId_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expr<Ex, En>(
        &self,
        elem: &mut Expr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_collection_targ<Ex>(
        &self,
        elem: &mut CollectionTarg<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_function_ptr_id<Ex, En>(
        &self,
        elem: &mut FunctionPtrId<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expression_tree<Ex, En>(
        &self,
        elem: &mut ExpressionTree<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expr_<Ex, En>(
        &self,
        elem: &mut Expr_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hole_source(
        &self,
        elem: &mut HoleSource,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_get_expr<Ex, En>(
        &self,
        elem: &mut ClassGetExpr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_case<Ex, En>(
        &self,
        elem: &mut Case<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_default_case<Ex, En>(
        &self,
        elem: &mut DefaultCase<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_catch<Ex, En>(
        &self,
        elem: &mut Catch<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_field<Ex, En>(
        &self,
        elem: &mut Field<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_afield<Ex, En>(
        &self,
        elem: &mut Afield<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_simple<Ex, En>(
        &self,
        elem: &mut XhpSimple<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attribute<Ex, En>(
        &self,
        elem: &mut XhpAttribute<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_param<Ex, En>(
        &self,
        elem: &mut FunParam<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_<Ex, En>(
        &self,
        elem: &mut Fun_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_fun__ret<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_efun<Ex, En>(
        &self,
        elem: &mut Efun<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_func_body<Ex, En>(
        &self,
        elem: &mut FuncBody<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_hint<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_targ<Ex>(
        &self,
        elem: &mut Targ<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_user_attribute<Ex, En>(
        &self,
        elem: &mut UserAttribute<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_file_attribute<Ex, En>(
        &self,
        elem: &mut FileAttribute<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_tparam<Ex, En>(
        &self,
        elem: &mut Tparam<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_require_kind(
        &self,
        elem: &mut RequireKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_emit_id(
        &self,
        elem: &mut EmitId,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_<Ex, En>(
        &self,
        elem: &mut Class_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__tparams<Ex, En>(
        &self,
        elem: &mut Vec<Tparam<Ex, En>>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__extends(
        &self,
        elem: &mut Vec<ClassHint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__uses(
        &self,
        elem: &mut Vec<TraitHint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__xhp_attr_uses(
        &self,
        elem: &mut Vec<XhpAttrHint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__reqs(
        &self,
        elem: &mut Vec<(ClassHint, RequireKind)>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__implements(
        &self,
        elem: &mut Vec<ClassHint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__consts<Ex, En>(
        &self,
        elem: &mut Vec<ClassConst<Ex, En>>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__xhp_attrs<Ex, En>(
        &self,
        elem: &mut Vec<XhpAttr<Ex, En>>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__user_attributes<Ex, En>(
        &self,
        elem: &mut UserAttributes<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr_tag(
        &self,
        elem: &mut XhpAttrTag,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr<Ex, En>(
        &self,
        elem: &mut XhpAttr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_const_kind<Ex, En>(
        &self,
        elem: &mut ClassConstKind<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_const<Ex, En>(
        &self,
        elem: &mut ClassConst<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_abstract_typeconst(
        &self,
        elem: &mut ClassAbstractTypeconst,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_concrete_typeconst(
        &self,
        elem: &mut ClassConcreteTypeconst,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst(
        &self,
        elem: &mut ClassTypeconst,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst_def<Ex, En>(
        &self,
        elem: &mut ClassTypeconstDef<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr_info(
        &self,
        elem: &mut XhpAttrInfo,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_var<Ex, En>(
        &self,
        elem: &mut ClassVar<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class_var_type_<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_method_<Ex, En>(
        &self,
        elem: &mut Method_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_method__ret<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_typedef<Ex, En>(
        &self,
        elem: &mut Typedef<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_gconst<Ex, En>(
        &self,
        elem: &mut Gconst<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_gconst_value<Ex, En>(
        &self,
        elem: &mut Expr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_def<Ex, En>(
        &self,
        elem: &mut FunDef<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_module_def<Ex, En>(
        &self,
        elem: &mut ModuleDef<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_md_name_kind(
        &self,
        elem: &mut MdNameKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_def<Ex, En>(
        &self,
        elem: &mut Def<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ns_kind(
        &self,
        elem: &mut NsKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_import_flavor(
        &self,
        elem: &mut ImportFlavor,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_child(
        &self,
        elem: &mut XhpChild,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_child_op(
        &self,
        elem: &mut XhpChildOp,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint(
        &self,
        elem: &mut Hint,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_contexts(
        &self,
        elem: &mut Contexts,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hf_param_info(
        &self,
        elem: &mut HfParamInfo,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint_fun(
        &self,
        elem: &mut HintFun,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_hint_fun_return_ty(
        &self,
        elem: &mut Hint,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint_(
        &self,
        elem: &mut Hint_,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_refinement(
        &self,
        elem: &mut Refinement,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_refinement(
        &self,
        elem: &mut TypeRefinement,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_refinement_bounds(
        &self,
        elem: &mut TypeRefinementBounds,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ctx_refinement(
        &self,
        elem: &mut CtxRefinement,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_bounds(
        &self,
        elem: &mut CtxRefinementBounds,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_shape_field_info(
        &self,
        elem: &mut ShapeFieldInfo,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_nast_shape_info(
        &self,
        elem: &mut NastShapeInfo,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_kvc_kind(
        &self,
        elem: &mut KvcKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_vc_kind(
        &self,
        elem: &mut VcKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_enum_(
        &self,
        elem: &mut Enum_,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_where_constraint_hint(
        &self,
        elem: &mut WhereConstraintHint,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_id(
        &self,
        elem: &mut Id,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_shape_field_name(
        &self,
        elem: &mut ShapeFieldName,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_variance(
        &self,
        elem: &mut Variance,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_constraint_kind(
        &self,
        elem: &mut ConstraintKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_abstraction(
        &self,
        elem: &mut Abstraction,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_classish_kind(
        &self,
        elem: &mut ClassishKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_param_kind(
        &self,
        elem: &mut ParamKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_readonly_kind(
        &self,
        elem: &mut ReadonlyKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_og_null_flavor(
        &self,
        elem: &mut OgNullFlavor,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_prop_or_method(
        &self,
        elem: &mut PropOrMethod,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_kind(
        &self,
        elem: &mut FunKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_bop(
        &self,
        elem: &mut Bop,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_uop(
        &self,
        elem: &mut Uop,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_visibility(
        &self,
        elem: &mut Visibility,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_enum_value(
        &self,
        elem: &mut XhpEnumValue,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_tprim(
        &self,
        elem: &mut Tprim,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_typedef_visibility(
        &self,
        elem: &mut TypedefVisibility,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_reify_kind(
        &self,
        elem: &mut ReifyKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        Continue(())
    }
}
pub struct Passes<Ctx, Err, P, Q>
where
    Ctx: Clone,
    P: Pass<Ctx = Ctx, Err = Err>,
    Q: Pass<Ctx = Ctx, Err = Err>,
{
    fst: P,
    snd: Q,
}
impl<Ctx, Err, P, Q> Pass for Passes<Ctx, Err, P, Q>
where
    Ctx: Clone,
    P: Pass<Ctx = Ctx, Err = Err>,
    Q: Pass<Ctx = Ctx, Err = Err>,
{
    type Ctx = Ctx;
    type Err = Err;
    #[inline(always)]
    fn on_ty_lid(
        &self,
        elem: &mut Lid,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_lid(elem, ctx, errs)?;
        self.snd.on_ty_lid(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_program<Ex, En>(
        &self,
        elem: &mut Program<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_program(elem, ctx, errs)?;
        self.snd.on_ty_program(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_stmt<Ex, En>(
        &self,
        elem: &mut Stmt<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_stmt(elem, ctx, errs)?;
        self.snd.on_ty_stmt(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_stmt_<Ex, En>(
        &self,
        elem: &mut Stmt_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_stmt_(elem, ctx, errs)?;
        self.snd.on_ty_stmt_(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_env_annot(
        &self,
        elem: &mut EnvAnnot,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_env_annot(elem, ctx, errs)?;
        self.snd.on_ty_env_annot(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_using_stmt<Ex, En>(
        &self,
        elem: &mut UsingStmt<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_using_stmt(elem, ctx, errs)?;
        self.snd.on_ty_using_stmt(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_as_expr<Ex, En>(
        &self,
        elem: &mut AsExpr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_as_expr(elem, ctx, errs)?;
        self.snd.on_ty_as_expr(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_block<Ex, En>(
        &self,
        elem: &mut Block<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_block(elem, ctx, errs)?;
        self.snd.on_ty_block(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_class_id<Ex, En>(
        &self,
        elem: &mut ClassId<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_class_id(elem, ctx, errs)?;
        self.snd.on_ty_class_id(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_class_id_<Ex, En>(
        &self,
        elem: &mut ClassId_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_class_id_(elem, ctx, errs)?;
        self.snd.on_ty_class_id_(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_expr<Ex, En>(
        &self,
        elem: &mut Expr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_expr(elem, ctx, errs)?;
        self.snd.on_ty_expr(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_collection_targ<Ex>(
        &self,
        elem: &mut CollectionTarg<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_collection_targ(elem, ctx, errs)?;
        self.snd.on_ty_collection_targ(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_function_ptr_id<Ex, En>(
        &self,
        elem: &mut FunctionPtrId<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_function_ptr_id(elem, ctx, errs)?;
        self.snd.on_ty_function_ptr_id(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_expression_tree<Ex, En>(
        &self,
        elem: &mut ExpressionTree<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_expression_tree(elem, ctx, errs)?;
        self.snd.on_ty_expression_tree(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_expr_<Ex, En>(
        &self,
        elem: &mut Expr_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_expr_(elem, ctx, errs)?;
        self.snd.on_ty_expr_(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_hole_source(
        &self,
        elem: &mut HoleSource,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_hole_source(elem, ctx, errs)?;
        self.snd.on_ty_hole_source(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_class_get_expr<Ex, En>(
        &self,
        elem: &mut ClassGetExpr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_class_get_expr(elem, ctx, errs)?;
        self.snd.on_ty_class_get_expr(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_case<Ex, En>(
        &self,
        elem: &mut Case<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_case(elem, ctx, errs)?;
        self.snd.on_ty_case(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_default_case<Ex, En>(
        &self,
        elem: &mut DefaultCase<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_default_case(elem, ctx, errs)?;
        self.snd.on_ty_default_case(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_catch<Ex, En>(
        &self,
        elem: &mut Catch<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_catch(elem, ctx, errs)?;
        self.snd.on_ty_catch(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_field<Ex, En>(
        &self,
        elem: &mut Field<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_field(elem, ctx, errs)?;
        self.snd.on_ty_field(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_afield<Ex, En>(
        &self,
        elem: &mut Afield<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_afield(elem, ctx, errs)?;
        self.snd.on_ty_afield(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_xhp_simple<Ex, En>(
        &self,
        elem: &mut XhpSimple<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_xhp_simple(elem, ctx, errs)?;
        self.snd.on_ty_xhp_simple(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_xhp_attribute<Ex, En>(
        &self,
        elem: &mut XhpAttribute<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_xhp_attribute(elem, ctx, errs)?;
        self.snd.on_ty_xhp_attribute(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_fun_param<Ex, En>(
        &self,
        elem: &mut FunParam<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_fun_param(elem, ctx, errs)?;
        self.snd.on_ty_fun_param(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_fun_<Ex, En>(
        &self,
        elem: &mut Fun_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_fun_(elem, ctx, errs)?;
        self.snd.on_ty_fun_(elem, ctx, errs)
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_fun__ret<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_fld_fun__ret(elem, ctx, errs)?;
        self.snd.on_fld_fun__ret(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_efun<Ex, En>(
        &self,
        elem: &mut Efun<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_efun(elem, ctx, errs)?;
        self.snd.on_ty_efun(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_func_body<Ex, En>(
        &self,
        elem: &mut FuncBody<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_func_body(elem, ctx, errs)?;
        self.snd.on_ty_func_body(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_type_hint<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_type_hint(elem, ctx, errs)?;
        self.snd.on_ty_type_hint(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_targ<Ex>(
        &self,
        elem: &mut Targ<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_targ(elem, ctx, errs)?;
        self.snd.on_ty_targ(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_user_attribute<Ex, En>(
        &self,
        elem: &mut UserAttribute<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_user_attribute(elem, ctx, errs)?;
        self.snd.on_ty_user_attribute(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_file_attribute<Ex, En>(
        &self,
        elem: &mut FileAttribute<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_file_attribute(elem, ctx, errs)?;
        self.snd.on_ty_file_attribute(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_tparam<Ex, En>(
        &self,
        elem: &mut Tparam<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_tparam(elem, ctx, errs)?;
        self.snd.on_ty_tparam(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_require_kind(
        &self,
        elem: &mut RequireKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_require_kind(elem, ctx, errs)?;
        self.snd.on_ty_require_kind(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_emit_id(
        &self,
        elem: &mut EmitId,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_emit_id(elem, ctx, errs)?;
        self.snd.on_ty_emit_id(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_class_<Ex, En>(
        &self,
        elem: &mut Class_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_class_(elem, ctx, errs)?;
        self.snd.on_ty_class_(elem, ctx, errs)
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__tparams<Ex, En>(
        &self,
        elem: &mut Vec<Tparam<Ex, En>>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_fld_class__tparams(elem, ctx, errs)?;
        self.snd.on_fld_class__tparams(elem, ctx, errs)
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__extends(
        &self,
        elem: &mut Vec<ClassHint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__extends(elem, ctx, errs)?;
        self.snd.on_fld_class__extends(elem, ctx, errs)
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__uses(
        &self,
        elem: &mut Vec<TraitHint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__uses(elem, ctx, errs)?;
        self.snd.on_fld_class__uses(elem, ctx, errs)
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__xhp_attr_uses(
        &self,
        elem: &mut Vec<XhpAttrHint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__xhp_attr_uses(elem, ctx, errs)?;
        self.snd.on_fld_class__xhp_attr_uses(elem, ctx, errs)
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__reqs(
        &self,
        elem: &mut Vec<(ClassHint, RequireKind)>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__reqs(elem, ctx, errs)?;
        self.snd.on_fld_class__reqs(elem, ctx, errs)
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__implements(
        &self,
        elem: &mut Vec<ClassHint>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_class__implements(elem, ctx, errs)?;
        self.snd.on_fld_class__implements(elem, ctx, errs)
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__consts<Ex, En>(
        &self,
        elem: &mut Vec<ClassConst<Ex, En>>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_fld_class__consts(elem, ctx, errs)?;
        self.snd.on_fld_class__consts(elem, ctx, errs)
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__xhp_attrs<Ex, En>(
        &self,
        elem: &mut Vec<XhpAttr<Ex, En>>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_fld_class__xhp_attrs(elem, ctx, errs)?;
        self.snd.on_fld_class__xhp_attrs(elem, ctx, errs)
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class__user_attributes<Ex, En>(
        &self,
        elem: &mut UserAttributes<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_fld_class__user_attributes(elem, ctx, errs)?;
        self.snd.on_fld_class__user_attributes(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_xhp_attr_tag(
        &self,
        elem: &mut XhpAttrTag,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_xhp_attr_tag(elem, ctx, errs)?;
        self.snd.on_ty_xhp_attr_tag(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_xhp_attr<Ex, En>(
        &self,
        elem: &mut XhpAttr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_xhp_attr(elem, ctx, errs)?;
        self.snd.on_ty_xhp_attr(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_class_const_kind<Ex, En>(
        &self,
        elem: &mut ClassConstKind<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_class_const_kind(elem, ctx, errs)?;
        self.snd.on_ty_class_const_kind(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_class_const<Ex, En>(
        &self,
        elem: &mut ClassConst<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_class_const(elem, ctx, errs)?;
        self.snd.on_ty_class_const(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_class_abstract_typeconst(
        &self,
        elem: &mut ClassAbstractTypeconst,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_abstract_typeconst(elem, ctx, errs)?;
        self.snd.on_ty_class_abstract_typeconst(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_class_concrete_typeconst(
        &self,
        elem: &mut ClassConcreteTypeconst,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_concrete_typeconst(elem, ctx, errs)?;
        self.snd.on_ty_class_concrete_typeconst(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_class_typeconst(
        &self,
        elem: &mut ClassTypeconst,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_class_typeconst(elem, ctx, errs)?;
        self.snd.on_ty_class_typeconst(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_class_typeconst_def<Ex, En>(
        &self,
        elem: &mut ClassTypeconstDef<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_class_typeconst_def(elem, ctx, errs)?;
        self.snd.on_ty_class_typeconst_def(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_xhp_attr_info(
        &self,
        elem: &mut XhpAttrInfo,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_xhp_attr_info(elem, ctx, errs)?;
        self.snd.on_ty_xhp_attr_info(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_class_var<Ex, En>(
        &self,
        elem: &mut ClassVar<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_class_var(elem, ctx, errs)?;
        self.snd.on_ty_class_var(elem, ctx, errs)
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_class_var_type_<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_fld_class_var_type_(elem, ctx, errs)?;
        self.snd.on_fld_class_var_type_(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_method_<Ex, En>(
        &self,
        elem: &mut Method_<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_method_(elem, ctx, errs)?;
        self.snd.on_ty_method_(elem, ctx, errs)
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_method__ret<Ex>(
        &self,
        elem: &mut TypeHint<Ex>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_fld_method__ret(elem, ctx, errs)?;
        self.snd.on_fld_method__ret(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_typedef<Ex, En>(
        &self,
        elem: &mut Typedef<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_typedef(elem, ctx, errs)?;
        self.snd.on_ty_typedef(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_gconst<Ex, En>(
        &self,
        elem: &mut Gconst<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_gconst(elem, ctx, errs)?;
        self.snd.on_ty_gconst(elem, ctx, errs)
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_gconst_value<Ex, En>(
        &self,
        elem: &mut Expr<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_fld_gconst_value(elem, ctx, errs)?;
        self.snd.on_fld_gconst_value(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_fun_def<Ex, En>(
        &self,
        elem: &mut FunDef<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_fun_def(elem, ctx, errs)?;
        self.snd.on_ty_fun_def(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_module_def<Ex, En>(
        &self,
        elem: &mut ModuleDef<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_module_def(elem, ctx, errs)?;
        self.snd.on_ty_module_def(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_md_name_kind(
        &self,
        elem: &mut MdNameKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_md_name_kind(elem, ctx, errs)?;
        self.snd.on_ty_md_name_kind(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_def<Ex, En>(
        &self,
        elem: &mut Def<Ex, En>,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()>
    where
        Ex: Default,
    {
        self.fst.on_ty_def(elem, ctx, errs)?;
        self.snd.on_ty_def(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_ns_kind(
        &self,
        elem: &mut NsKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_ns_kind(elem, ctx, errs)?;
        self.snd.on_ty_ns_kind(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_import_flavor(
        &self,
        elem: &mut ImportFlavor,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_import_flavor(elem, ctx, errs)?;
        self.snd.on_ty_import_flavor(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_xhp_child(
        &self,
        elem: &mut XhpChild,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_xhp_child(elem, ctx, errs)?;
        self.snd.on_ty_xhp_child(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_xhp_child_op(
        &self,
        elem: &mut XhpChildOp,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_xhp_child_op(elem, ctx, errs)?;
        self.snd.on_ty_xhp_child_op(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_hint(
        &self,
        elem: &mut Hint,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_hint(elem, ctx, errs)?;
        self.snd.on_ty_hint(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_contexts(
        &self,
        elem: &mut Contexts,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_contexts(elem, ctx, errs)?;
        self.snd.on_ty_contexts(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_hf_param_info(
        &self,
        elem: &mut HfParamInfo,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_hf_param_info(elem, ctx, errs)?;
        self.snd.on_ty_hf_param_info(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_hint_fun(
        &self,
        elem: &mut HintFun,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_hint_fun(elem, ctx, errs)?;
        self.snd.on_ty_hint_fun(elem, ctx, errs)
    }
    #[allow(non_snake_case)]
    #[inline(always)]
    fn on_fld_hint_fun_return_ty(
        &self,
        elem: &mut Hint,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_fld_hint_fun_return_ty(elem, ctx, errs)?;
        self.snd.on_fld_hint_fun_return_ty(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_hint_(
        &self,
        elem: &mut Hint_,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_hint_(elem, ctx, errs)?;
        self.snd.on_ty_hint_(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_refinement(
        &self,
        elem: &mut Refinement,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_refinement(elem, ctx, errs)?;
        self.snd.on_ty_refinement(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_type_refinement(
        &self,
        elem: &mut TypeRefinement,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_type_refinement(elem, ctx, errs)?;
        self.snd.on_ty_type_refinement(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_type_refinement_bounds(
        &self,
        elem: &mut TypeRefinementBounds,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_type_refinement_bounds(elem, ctx, errs)?;
        self.snd.on_ty_type_refinement_bounds(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_ctx_refinement(
        &self,
        elem: &mut CtxRefinement,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_ctx_refinement(elem, ctx, errs)?;
        self.snd.on_ty_ctx_refinement(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_bounds(
        &self,
        elem: &mut CtxRefinementBounds,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_ctx_refinement_bounds(elem, ctx, errs)?;
        self.snd.on_ty_ctx_refinement_bounds(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_shape_field_info(
        &self,
        elem: &mut ShapeFieldInfo,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_shape_field_info(elem, ctx, errs)?;
        self.snd.on_ty_shape_field_info(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_nast_shape_info(
        &self,
        elem: &mut NastShapeInfo,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_nast_shape_info(elem, ctx, errs)?;
        self.snd.on_ty_nast_shape_info(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_kvc_kind(
        &self,
        elem: &mut KvcKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_kvc_kind(elem, ctx, errs)?;
        self.snd.on_ty_kvc_kind(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_vc_kind(
        &self,
        elem: &mut VcKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_vc_kind(elem, ctx, errs)?;
        self.snd.on_ty_vc_kind(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_enum_(
        &self,
        elem: &mut Enum_,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_enum_(elem, ctx, errs)?;
        self.snd.on_ty_enum_(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_where_constraint_hint(
        &self,
        elem: &mut WhereConstraintHint,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_where_constraint_hint(elem, ctx, errs)?;
        self.snd.on_ty_where_constraint_hint(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_id(
        &self,
        elem: &mut Id,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_id(elem, ctx, errs)?;
        self.snd.on_ty_id(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_shape_field_name(
        &self,
        elem: &mut ShapeFieldName,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_shape_field_name(elem, ctx, errs)?;
        self.snd.on_ty_shape_field_name(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_variance(
        &self,
        elem: &mut Variance,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_variance(elem, ctx, errs)?;
        self.snd.on_ty_variance(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_constraint_kind(
        &self,
        elem: &mut ConstraintKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_constraint_kind(elem, ctx, errs)?;
        self.snd.on_ty_constraint_kind(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_abstraction(
        &self,
        elem: &mut Abstraction,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_abstraction(elem, ctx, errs)?;
        self.snd.on_ty_abstraction(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_classish_kind(
        &self,
        elem: &mut ClassishKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_classish_kind(elem, ctx, errs)?;
        self.snd.on_ty_classish_kind(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_param_kind(
        &self,
        elem: &mut ParamKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_param_kind(elem, ctx, errs)?;
        self.snd.on_ty_param_kind(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_readonly_kind(
        &self,
        elem: &mut ReadonlyKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_readonly_kind(elem, ctx, errs)?;
        self.snd.on_ty_readonly_kind(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_og_null_flavor(
        &self,
        elem: &mut OgNullFlavor,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_og_null_flavor(elem, ctx, errs)?;
        self.snd.on_ty_og_null_flavor(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_prop_or_method(
        &self,
        elem: &mut PropOrMethod,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_prop_or_method(elem, ctx, errs)?;
        self.snd.on_ty_prop_or_method(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_fun_kind(
        &self,
        elem: &mut FunKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_fun_kind(elem, ctx, errs)?;
        self.snd.on_ty_fun_kind(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_bop(
        &self,
        elem: &mut Bop,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_bop(elem, ctx, errs)?;
        self.snd.on_ty_bop(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_uop(
        &self,
        elem: &mut Uop,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_uop(elem, ctx, errs)?;
        self.snd.on_ty_uop(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_visibility(
        &self,
        elem: &mut Visibility,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_visibility(elem, ctx, errs)?;
        self.snd.on_ty_visibility(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_xhp_enum_value(
        &self,
        elem: &mut XhpEnumValue,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_xhp_enum_value(elem, ctx, errs)?;
        self.snd.on_ty_xhp_enum_value(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_tprim(
        &self,
        elem: &mut Tprim,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_tprim(elem, ctx, errs)?;
        self.snd.on_ty_tprim(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_typedef_visibility(
        &self,
        elem: &mut TypedefVisibility,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_typedef_visibility(elem, ctx, errs)?;
        self.snd.on_ty_typedef_visibility(elem, ctx, errs)
    }
    #[inline(always)]
    fn on_ty_reify_kind(
        &self,
        elem: &mut ReifyKind,
        ctx: &mut Self::Ctx,
        errs: &mut Vec<Self::Err>,
    ) -> ControlFlow<(), ()> {
        self.fst.on_ty_reify_kind(elem, ctx, errs)?;
        self.snd.on_ty_reify_kind(elem, ctx, errs)
    }
}
#[doc = r" Used to combine multiple types implementing `Pass` into nested `Passes` types"]
#[doc = r" without requiring them to hand write it so :"]
#[doc = r" `passes![p1, p2, p3]` => `Passes(p1, Passes(p2, p3))`"]
#[macro_export]
macro_rules ! passes { ($ p : expr $ (, $ ps : expr) + $ (,) ?) => { $ crate :: transform :: Passes ($ p , $ crate :: passes ! ($ ($ ps) , *)) } ; ($ p : expr $ (,) ?) => { $ p } ; }

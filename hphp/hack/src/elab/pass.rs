// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<84c9e5ca29bb4803f7fcfb72ce7ec5ab>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_variables, non_snake_case)]
use std::ops::ControlFlow;
use std::ops::ControlFlow::Continue;

use oxidized::aast_defs::*;
use oxidized::ast_defs::*;

use crate::env::Env;
type Ex = ();
type En = ();
pub trait Pass: PassClone {
    #[inline(always)]
    fn on_ty_program_top_down(&mut self, env: &Env, elem: &mut Program<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_program_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Program<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_top_down(&mut self, env: &Env, elem: &mut Stmt<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_bottom_up(&mut self, env: &Env, elem: &mut Stmt<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt__top_down(&mut self, env: &Env, elem: &mut Stmt_<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt__bottom_up(&mut self, env: &Env, elem: &mut Stmt_<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_using_stmt_top_down(
        &mut self,
        env: &Env,
        elem: &mut UsingStmt<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_using_stmt_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut UsingStmt<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_as_expr_top_down(&mut self, env: &Env, elem: &mut AsExpr<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_as_expr_bottom_up(&mut self, env: &Env, elem: &mut AsExpr<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_block_top_down(&mut self, env: &Env, elem: &mut Block<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_block_bottom_up(&mut self, env: &Env, elem: &mut Block<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_finally_block_top_down(
        &mut self,
        env: &Env,
        elem: &mut FinallyBlock<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_finally_block_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut FinallyBlock<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_match_top_down(
        &mut self,
        env: &Env,
        elem: &mut StmtMatch<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_match_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut StmtMatch<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_match_arm_top_down(
        &mut self,
        env: &Env,
        elem: &mut StmtMatchArm<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_match_arm_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut StmtMatchArm<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_pattern_top_down(&mut self, env: &Env, elem: &mut Pattern) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_pattern_bottom_up(&mut self, env: &Env, elem: &mut Pattern) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_pat_var_top_down(&mut self, env: &Env, elem: &mut PatVar) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_pat_var_bottom_up(&mut self, env: &Env, elem: &mut PatVar) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_pat_refinement_top_down(
        &mut self,
        env: &Env,
        elem: &mut PatRefinement,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_pat_refinement_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut PatRefinement,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_id_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassId<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_id_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassId<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_id__top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassId_<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_id__bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassId_<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expr_top_down(&mut self, env: &Env, elem: &mut Expr<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expr_bottom_up(&mut self, env: &Env, elem: &mut Expr<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_collection_targ_top_down(
        &mut self,
        env: &Env,
        elem: &mut CollectionTarg<Ex>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_collection_targ_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut CollectionTarg<Ex>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_function_ptr_id_top_down(
        &mut self,
        env: &Env,
        elem: &mut FunctionPtrId<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_function_ptr_id_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut FunctionPtrId<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expression_tree_top_down(
        &mut self,
        env: &Env,
        elem: &mut ExpressionTree<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expression_tree_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ExpressionTree<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expr__top_down(&mut self, env: &Env, elem: &mut Expr_<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expr__bottom_up(&mut self, env: &Env, elem: &mut Expr_<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hole_source_top_down(&mut self, env: &Env, elem: &mut HoleSource) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hole_source_bottom_up(&mut self, env: &Env, elem: &mut HoleSource) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_binop_top_down(&mut self, env: &Env, elem: &mut Binop<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_binop_bottom_up(&mut self, env: &Env, elem: &mut Binop<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_binop_lhs_top_down(&mut self, env: &Env, elem: &mut Expr<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_binop_lhs_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Expr<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_binop_rhs_top_down(&mut self, env: &Env, elem: &mut Expr<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_binop_rhs_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Expr<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_get_expr_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassGetExpr<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_get_expr_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassGetExpr<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_case_top_down(&mut self, env: &Env, elem: &mut Case<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_case_bottom_up(&mut self, env: &Env, elem: &mut Case<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_default_case_top_down(
        &mut self,
        env: &Env,
        elem: &mut DefaultCase<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_default_case_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut DefaultCase<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_catch_top_down(&mut self, env: &Env, elem: &mut Catch<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_catch_bottom_up(&mut self, env: &Env, elem: &mut Catch<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_field_top_down(&mut self, env: &Env, elem: &mut Field<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_field_bottom_up(&mut self, env: &Env, elem: &mut Field<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_afield_top_down(&mut self, env: &Env, elem: &mut Afield<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_afield_bottom_up(&mut self, env: &Env, elem: &mut Afield<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_simple_top_down(
        &mut self,
        env: &Env,
        elem: &mut XhpSimple<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_simple_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut XhpSimple<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attribute_top_down(
        &mut self,
        env: &Env,
        elem: &mut XhpAttribute<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attribute_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut XhpAttribute<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_param_top_down(
        &mut self,
        env: &Env,
        elem: &mut FunParam<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_param_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut FunParam<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun__top_down(&mut self, env: &Env, elem: &mut Fun_<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun__bottom_up(&mut self, env: &Env, elem: &mut Fun_<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_fun__ret_top_down(&mut self, env: &Env, elem: &mut TypeHint<Ex>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_fun__ret_bottom_up(&mut self, env: &Env, elem: &mut TypeHint<Ex>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_capture_lid_top_down(
        &mut self,
        env: &Env,
        elem: &mut CaptureLid<Ex>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_capture_lid_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut CaptureLid<Ex>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_efun_top_down(&mut self, env: &Env, elem: &mut Efun<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_efun_bottom_up(&mut self, env: &Env, elem: &mut Efun<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_func_body_top_down(
        &mut self,
        env: &Env,
        elem: &mut FuncBody<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_func_body_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut FuncBody<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_hint_top_down(&mut self, env: &Env, elem: &mut TypeHint<Ex>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_hint_bottom_up(&mut self, env: &Env, elem: &mut TypeHint<Ex>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_targ_top_down(&mut self, env: &Env, elem: &mut Targ<Ex>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_targ_bottom_up(&mut self, env: &Env, elem: &mut Targ<Ex>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_call_expr_top_down(
        &mut self,
        env: &Env,
        elem: &mut CallExpr<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_call_expr_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut CallExpr<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_user_attribute_top_down(
        &mut self,
        env: &Env,
        elem: &mut UserAttribute<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_user_attribute_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut UserAttribute<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_file_attribute_top_down(
        &mut self,
        env: &Env,
        elem: &mut FileAttribute<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_file_attribute_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut FileAttribute<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_tparam_top_down(&mut self, env: &Env, elem: &mut Tparam<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_tparam_bottom_up(&mut self, env: &Env, elem: &mut Tparam<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class__top_down(&mut self, env: &Env, elem: &mut Class_<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class__bottom_up(&mut self, env: &Env, elem: &mut Class_<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__tparams_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<Tparam<Ex, En>>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__tparams_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<Tparam<Ex, En>>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__extends_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassHint>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__extends_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassHint>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__uses_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<TraitHint>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__uses_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<TraitHint>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__xhp_attr_uses_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<XhpAttrHint>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__xhp_attr_uses_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<XhpAttrHint>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__reqs_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassReq>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__reqs_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassReq>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__implements_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassHint>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__implements_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassHint>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__consts_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassConst<Ex, En>>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__consts_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassConst<Ex, En>>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__xhp_attrs_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<XhpAttr<Ex, En>>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__xhp_attrs_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<XhpAttr<Ex, En>>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__user_attributes_top_down(
        &mut self,
        env: &Env,
        elem: &mut UserAttributes<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__user_attributes_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut UserAttributes<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_req_top_down(&mut self, env: &Env, elem: &mut ClassReq) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_req_bottom_up(&mut self, env: &Env, elem: &mut ClassReq) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr_top_down(
        &mut self,
        env: &Env,
        elem: &mut XhpAttr<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut XhpAttr<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_const_kind_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassConstKind<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_const_kind_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassConstKind<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_const_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassConst<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_const_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassConst<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_abstract_typeconst_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassAbstractTypeconst,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_abstract_typeconst_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassAbstractTypeconst,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class_abstract_typeconst_default_top_down(
        &mut self,
        env: &Env,
        elem: &mut Option<Hint>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class_abstract_typeconst_default_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Option<Hint>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_concrete_typeconst_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassConcreteTypeconst,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_concrete_typeconst_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassConcreteTypeconst,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassTypeconst,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassTypeconst,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst_def_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassTypeconstDef<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst_def_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassTypeconstDef<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr_info_top_down(
        &mut self,
        env: &Env,
        elem: &mut XhpAttrInfo,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr_info_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut XhpAttrInfo,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_var_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassVar<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_var_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassVar<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class_var_type__top_down(
        &mut self,
        env: &Env,
        elem: &mut TypeHint<Ex>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class_var_type__bottom_up(
        &mut self,
        env: &Env,
        elem: &mut TypeHint<Ex>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_method__top_down(&mut self, env: &Env, elem: &mut Method_<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_method__bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Method_<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_method__ret_top_down(
        &mut self,
        env: &Env,
        elem: &mut TypeHint<Ex>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_method__ret_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut TypeHint<Ex>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_typedef_top_down(&mut self, env: &Env, elem: &mut Typedef<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_typedef_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Typedef<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_typedef_kind_top_down(&mut self, env: &Env, elem: &mut Hint) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_typedef_kind_bottom_up(&mut self, env: &Env, elem: &mut Hint) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_gconst_top_down(&mut self, env: &Env, elem: &mut Gconst<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_gconst_bottom_up(&mut self, env: &Env, elem: &mut Gconst<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_gconst_value_top_down(
        &mut self,
        env: &Env,
        elem: &mut Expr<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_gconst_value_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Expr<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_def_top_down(&mut self, env: &Env, elem: &mut FunDef<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_def_bottom_up(&mut self, env: &Env, elem: &mut FunDef<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_module_def_top_down(
        &mut self,
        env: &Env,
        elem: &mut ModuleDef<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_module_def_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ModuleDef<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_def_top_down(&mut self, env: &Env, elem: &mut Def<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_def_bottom_up(&mut self, env: &Env, elem: &mut Def<Ex, En>) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_child_top_down(&mut self, env: &Env, elem: &mut XhpChild) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_child_bottom_up(&mut self, env: &Env, elem: &mut XhpChild) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint_top_down(&mut self, env: &Env, elem: &mut Hint) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint_bottom_up(&mut self, env: &Env, elem: &mut Hint) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_user_attributes_top_down(
        &mut self,
        env: &Env,
        elem: &mut UserAttributes<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_user_attributes_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut UserAttributes<Ex, En>,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_contexts_top_down(&mut self, env: &Env, elem: &mut Contexts) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_contexts_bottom_up(&mut self, env: &Env, elem: &mut Contexts) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint_fun_top_down(&mut self, env: &Env, elem: &mut HintFun) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint_fun_bottom_up(&mut self, env: &Env, elem: &mut HintFun) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_hint_fun_return_ty_top_down(
        &mut self,
        env: &Env,
        elem: &mut Hint,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_hint_fun_return_ty_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Hint,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint__top_down(&mut self, env: &Env, elem: &mut Hint_) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint__bottom_up(&mut self, env: &Env, elem: &mut Hint_) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_refinement_top_down(&mut self, env: &Env, elem: &mut Refinement) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_refinement_bottom_up(&mut self, env: &Env, elem: &mut Refinement) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_refinement_top_down(
        &mut self,
        env: &Env,
        elem: &mut TypeRefinement,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_refinement_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut TypeRefinement,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_refinement_bounds_top_down(
        &mut self,
        env: &Env,
        elem: &mut TypeRefinementBounds,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_refinement_bounds_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut TypeRefinementBounds,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_top_down(
        &mut self,
        env: &Env,
        elem: &mut CtxRefinement,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut CtxRefinement,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_bounds_top_down(
        &mut self,
        env: &Env,
        elem: &mut CtxRefinementBounds,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_bounds_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut CtxRefinementBounds,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_shape_field_info_top_down(
        &mut self,
        env: &Env,
        elem: &mut ShapeFieldInfo,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_shape_field_info_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ShapeFieldInfo,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_nast_shape_info_top_down(
        &mut self,
        env: &Env,
        elem: &mut NastShapeInfo,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_nast_shape_info_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut NastShapeInfo,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_enum__top_down(&mut self, env: &Env, elem: &mut Enum_) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_enum__bottom_up(&mut self, env: &Env, elem: &mut Enum_) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_where_constraint_hint_top_down(
        &mut self,
        env: &Env,
        elem: &mut WhereConstraintHint,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_where_constraint_hint_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut WhereConstraintHint,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_id_top_down(&mut self, env: &Env, elem: &mut Id) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_id_bottom_up(&mut self, env: &Env, elem: &mut Id) -> ControlFlow<()> {
        Continue(())
    }
}
pub trait PassClone {
    fn clone_box(&self) -> Box<dyn Pass>;
}
impl<T> PassClone for T
where
    T: 'static + Pass + Clone,
{
    fn clone_box(&self) -> Box<dyn Pass> {
        Box::new(self.clone())
    }
}
impl Clone for Box<dyn Pass> {
    fn clone(&self) -> Box<dyn Pass> {
        self.clone_box()
    }
}
pub struct Passes {
    pub passes: Vec<Box<dyn Pass>>,
}
impl Clone for Passes {
    fn clone(&self) -> Self {
        Self {
            passes: self.passes.clone(),
        }
    }
}
impl Pass for Passes {
    #[inline(always)]
    fn on_ty_program_top_down(&mut self, env: &Env, elem: &mut Program<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_program_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_program_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Program<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_program_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_top_down(&mut self, env: &Env, elem: &mut Stmt<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_stmt_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_bottom_up(&mut self, env: &Env, elem: &mut Stmt<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_stmt_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt__top_down(&mut self, env: &Env, elem: &mut Stmt_<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_stmt__top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt__bottom_up(&mut self, env: &Env, elem: &mut Stmt_<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_stmt__bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_using_stmt_top_down(
        &mut self,
        env: &Env,
        elem: &mut UsingStmt<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_using_stmt_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_using_stmt_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut UsingStmt<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_using_stmt_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_as_expr_top_down(&mut self, env: &Env, elem: &mut AsExpr<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_as_expr_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_as_expr_bottom_up(&mut self, env: &Env, elem: &mut AsExpr<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_as_expr_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_block_top_down(&mut self, env: &Env, elem: &mut Block<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_block_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_block_bottom_up(&mut self, env: &Env, elem: &mut Block<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_block_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_finally_block_top_down(
        &mut self,
        env: &Env,
        elem: &mut FinallyBlock<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_finally_block_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_finally_block_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut FinallyBlock<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_finally_block_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_match_top_down(
        &mut self,
        env: &Env,
        elem: &mut StmtMatch<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_stmt_match_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_match_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut StmtMatch<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_stmt_match_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_match_arm_top_down(
        &mut self,
        env: &Env,
        elem: &mut StmtMatchArm<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_stmt_match_arm_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_match_arm_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut StmtMatchArm<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_stmt_match_arm_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_pattern_top_down(&mut self, env: &Env, elem: &mut Pattern) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_pattern_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_pattern_bottom_up(&mut self, env: &Env, elem: &mut Pattern) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_pattern_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_pat_var_top_down(&mut self, env: &Env, elem: &mut PatVar) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_pat_var_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_pat_var_bottom_up(&mut self, env: &Env, elem: &mut PatVar) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_pat_var_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_pat_refinement_top_down(
        &mut self,
        env: &Env,
        elem: &mut PatRefinement,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_pat_refinement_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_pat_refinement_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut PatRefinement,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_pat_refinement_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_id_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassId<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_id_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_id_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassId<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_id_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_id__top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassId_<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_id__top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_id__bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassId_<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_id__bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expr_top_down(&mut self, env: &Env, elem: &mut Expr<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_expr_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expr_bottom_up(&mut self, env: &Env, elem: &mut Expr<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_expr_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_collection_targ_top_down(
        &mut self,
        env: &Env,
        elem: &mut CollectionTarg<Ex>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_collection_targ_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_collection_targ_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut CollectionTarg<Ex>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_collection_targ_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_function_ptr_id_top_down(
        &mut self,
        env: &Env,
        elem: &mut FunctionPtrId<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_function_ptr_id_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_function_ptr_id_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut FunctionPtrId<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_function_ptr_id_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expression_tree_top_down(
        &mut self,
        env: &Env,
        elem: &mut ExpressionTree<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_expression_tree_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expression_tree_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ExpressionTree<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_expression_tree_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expr__top_down(&mut self, env: &Env, elem: &mut Expr_<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_expr__top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expr__bottom_up(&mut self, env: &Env, elem: &mut Expr_<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_expr__bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hole_source_top_down(&mut self, env: &Env, elem: &mut HoleSource) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_hole_source_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hole_source_bottom_up(&mut self, env: &Env, elem: &mut HoleSource) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_hole_source_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_binop_top_down(&mut self, env: &Env, elem: &mut Binop<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_binop_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_binop_bottom_up(&mut self, env: &Env, elem: &mut Binop<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_binop_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_binop_lhs_top_down(&mut self, env: &Env, elem: &mut Expr<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_binop_lhs_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_binop_lhs_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Expr<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_binop_lhs_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_binop_rhs_top_down(&mut self, env: &Env, elem: &mut Expr<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_binop_rhs_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_binop_rhs_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Expr<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_binop_rhs_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_get_expr_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassGetExpr<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_get_expr_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_get_expr_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassGetExpr<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_get_expr_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_case_top_down(&mut self, env: &Env, elem: &mut Case<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_case_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_case_bottom_up(&mut self, env: &Env, elem: &mut Case<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_case_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_default_case_top_down(
        &mut self,
        env: &Env,
        elem: &mut DefaultCase<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_default_case_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_default_case_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut DefaultCase<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_default_case_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_catch_top_down(&mut self, env: &Env, elem: &mut Catch<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_catch_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_catch_bottom_up(&mut self, env: &Env, elem: &mut Catch<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_catch_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_field_top_down(&mut self, env: &Env, elem: &mut Field<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_field_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_field_bottom_up(&mut self, env: &Env, elem: &mut Field<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_field_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_afield_top_down(&mut self, env: &Env, elem: &mut Afield<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_afield_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_afield_bottom_up(&mut self, env: &Env, elem: &mut Afield<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_afield_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_simple_top_down(
        &mut self,
        env: &Env,
        elem: &mut XhpSimple<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_xhp_simple_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_simple_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut XhpSimple<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_xhp_simple_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attribute_top_down(
        &mut self,
        env: &Env,
        elem: &mut XhpAttribute<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_xhp_attribute_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attribute_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut XhpAttribute<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_xhp_attribute_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_param_top_down(
        &mut self,
        env: &Env,
        elem: &mut FunParam<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_fun_param_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_param_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut FunParam<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_fun_param_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun__top_down(&mut self, env: &Env, elem: &mut Fun_<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_fun__top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun__bottom_up(&mut self, env: &Env, elem: &mut Fun_<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_fun__bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_fun__ret_top_down(&mut self, env: &Env, elem: &mut TypeHint<Ex>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_fun__ret_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_fun__ret_bottom_up(&mut self, env: &Env, elem: &mut TypeHint<Ex>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_fun__ret_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_capture_lid_top_down(
        &mut self,
        env: &Env,
        elem: &mut CaptureLid<Ex>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_capture_lid_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_capture_lid_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut CaptureLid<Ex>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_capture_lid_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_efun_top_down(&mut self, env: &Env, elem: &mut Efun<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_efun_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_efun_bottom_up(&mut self, env: &Env, elem: &mut Efun<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_efun_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_func_body_top_down(
        &mut self,
        env: &Env,
        elem: &mut FuncBody<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_func_body_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_func_body_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut FuncBody<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_func_body_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_hint_top_down(&mut self, env: &Env, elem: &mut TypeHint<Ex>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_type_hint_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_hint_bottom_up(&mut self, env: &Env, elem: &mut TypeHint<Ex>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_type_hint_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_targ_top_down(&mut self, env: &Env, elem: &mut Targ<Ex>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_targ_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_targ_bottom_up(&mut self, env: &Env, elem: &mut Targ<Ex>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_targ_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_call_expr_top_down(
        &mut self,
        env: &Env,
        elem: &mut CallExpr<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_call_expr_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_call_expr_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut CallExpr<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_call_expr_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_user_attribute_top_down(
        &mut self,
        env: &Env,
        elem: &mut UserAttribute<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_user_attribute_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_user_attribute_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut UserAttribute<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_user_attribute_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_file_attribute_top_down(
        &mut self,
        env: &Env,
        elem: &mut FileAttribute<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_file_attribute_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_file_attribute_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut FileAttribute<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_file_attribute_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_tparam_top_down(&mut self, env: &Env, elem: &mut Tparam<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_tparam_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_tparam_bottom_up(&mut self, env: &Env, elem: &mut Tparam<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_tparam_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class__top_down(&mut self, env: &Env, elem: &mut Class_<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class__top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class__bottom_up(&mut self, env: &Env, elem: &mut Class_<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class__bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__tparams_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<Tparam<Ex, En>>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__tparams_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__tparams_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<Tparam<Ex, En>>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__tparams_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__extends_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassHint>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__extends_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__extends_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassHint>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__extends_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__uses_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<TraitHint>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__uses_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__uses_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<TraitHint>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__uses_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__xhp_attr_uses_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<XhpAttrHint>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__xhp_attr_uses_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__xhp_attr_uses_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<XhpAttrHint>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__xhp_attr_uses_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__reqs_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassReq>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__reqs_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__reqs_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassReq>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__reqs_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__implements_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassHint>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__implements_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__implements_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassHint>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__implements_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__consts_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassConst<Ex, En>>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__consts_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__consts_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<ClassConst<Ex, En>>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__consts_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__xhp_attrs_top_down(
        &mut self,
        env: &Env,
        elem: &mut Vec<XhpAttr<Ex, En>>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__xhp_attrs_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__xhp_attrs_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Vec<XhpAttr<Ex, En>>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__xhp_attrs_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__user_attributes_top_down(
        &mut self,
        env: &Env,
        elem: &mut UserAttributes<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__user_attributes_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__user_attributes_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut UserAttributes<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class__user_attributes_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_req_top_down(&mut self, env: &Env, elem: &mut ClassReq) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_req_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_req_bottom_up(&mut self, env: &Env, elem: &mut ClassReq) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_req_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr_top_down(
        &mut self,
        env: &Env,
        elem: &mut XhpAttr<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_xhp_attr_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut XhpAttr<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_xhp_attr_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_const_kind_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassConstKind<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_const_kind_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_const_kind_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassConstKind<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_const_kind_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_const_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassConst<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_const_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_const_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassConst<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_const_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_abstract_typeconst_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassAbstractTypeconst,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_abstract_typeconst_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_abstract_typeconst_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassAbstractTypeconst,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_abstract_typeconst_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class_abstract_typeconst_default_top_down(
        &mut self,
        env: &Env,
        elem: &mut Option<Hint>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class_abstract_typeconst_default_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class_abstract_typeconst_default_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Option<Hint>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class_abstract_typeconst_default_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_concrete_typeconst_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassConcreteTypeconst,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_concrete_typeconst_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_concrete_typeconst_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassConcreteTypeconst,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_concrete_typeconst_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassTypeconst,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_typeconst_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassTypeconst,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_typeconst_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst_def_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassTypeconstDef<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_typeconst_def_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst_def_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassTypeconstDef<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_typeconst_def_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr_info_top_down(
        &mut self,
        env: &Env,
        elem: &mut XhpAttrInfo,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_xhp_attr_info_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr_info_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut XhpAttrInfo,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_xhp_attr_info_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_var_top_down(
        &mut self,
        env: &Env,
        elem: &mut ClassVar<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_var_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_var_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ClassVar<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_class_var_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class_var_type__top_down(
        &mut self,
        env: &Env,
        elem: &mut TypeHint<Ex>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class_var_type__top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class_var_type__bottom_up(
        &mut self,
        env: &Env,
        elem: &mut TypeHint<Ex>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_class_var_type__bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_method__top_down(&mut self, env: &Env, elem: &mut Method_<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_method__top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_method__bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Method_<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_method__bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_method__ret_top_down(
        &mut self,
        env: &Env,
        elem: &mut TypeHint<Ex>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_method__ret_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_method__ret_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut TypeHint<Ex>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_method__ret_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_typedef_top_down(&mut self, env: &Env, elem: &mut Typedef<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_typedef_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_typedef_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Typedef<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_typedef_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_typedef_kind_top_down(&mut self, env: &Env, elem: &mut Hint) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_typedef_kind_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_typedef_kind_bottom_up(&mut self, env: &Env, elem: &mut Hint) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_typedef_kind_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_gconst_top_down(&mut self, env: &Env, elem: &mut Gconst<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_gconst_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_gconst_bottom_up(&mut self, env: &Env, elem: &mut Gconst<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_gconst_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_gconst_value_top_down(
        &mut self,
        env: &Env,
        elem: &mut Expr<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_gconst_value_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_gconst_value_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Expr<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_gconst_value_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_def_top_down(&mut self, env: &Env, elem: &mut FunDef<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_fun_def_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_def_bottom_up(&mut self, env: &Env, elem: &mut FunDef<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_fun_def_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_module_def_top_down(
        &mut self,
        env: &Env,
        elem: &mut ModuleDef<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_module_def_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_module_def_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ModuleDef<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_module_def_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_def_top_down(&mut self, env: &Env, elem: &mut Def<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_def_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_def_bottom_up(&mut self, env: &Env, elem: &mut Def<Ex, En>) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_def_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_child_top_down(&mut self, env: &Env, elem: &mut XhpChild) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_xhp_child_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_child_bottom_up(&mut self, env: &Env, elem: &mut XhpChild) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_xhp_child_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint_top_down(&mut self, env: &Env, elem: &mut Hint) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_hint_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint_bottom_up(&mut self, env: &Env, elem: &mut Hint) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_hint_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_user_attributes_top_down(
        &mut self,
        env: &Env,
        elem: &mut UserAttributes<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_user_attributes_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_user_attributes_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut UserAttributes<Ex, En>,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_user_attributes_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_contexts_top_down(&mut self, env: &Env, elem: &mut Contexts) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_contexts_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_contexts_bottom_up(&mut self, env: &Env, elem: &mut Contexts) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_contexts_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint_fun_top_down(&mut self, env: &Env, elem: &mut HintFun) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_hint_fun_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint_fun_bottom_up(&mut self, env: &Env, elem: &mut HintFun) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_hint_fun_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_hint_fun_return_ty_top_down(
        &mut self,
        env: &Env,
        elem: &mut Hint,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_hint_fun_return_ty_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_fld_hint_fun_return_ty_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut Hint,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_fld_hint_fun_return_ty_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint__top_down(&mut self, env: &Env, elem: &mut Hint_) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_hint__top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint__bottom_up(&mut self, env: &Env, elem: &mut Hint_) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_hint__bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_refinement_top_down(&mut self, env: &Env, elem: &mut Refinement) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_refinement_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_refinement_bottom_up(&mut self, env: &Env, elem: &mut Refinement) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_refinement_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_refinement_top_down(
        &mut self,
        env: &Env,
        elem: &mut TypeRefinement,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_type_refinement_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_refinement_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut TypeRefinement,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_type_refinement_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_refinement_bounds_top_down(
        &mut self,
        env: &Env,
        elem: &mut TypeRefinementBounds,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_type_refinement_bounds_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_refinement_bounds_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut TypeRefinementBounds,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_type_refinement_bounds_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_top_down(
        &mut self,
        env: &Env,
        elem: &mut CtxRefinement,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_ctx_refinement_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut CtxRefinement,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_ctx_refinement_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_bounds_top_down(
        &mut self,
        env: &Env,
        elem: &mut CtxRefinementBounds,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_ctx_refinement_bounds_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_bounds_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut CtxRefinementBounds,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_ctx_refinement_bounds_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_shape_field_info_top_down(
        &mut self,
        env: &Env,
        elem: &mut ShapeFieldInfo,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_shape_field_info_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_shape_field_info_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut ShapeFieldInfo,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_shape_field_info_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_nast_shape_info_top_down(
        &mut self,
        env: &Env,
        elem: &mut NastShapeInfo,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_nast_shape_info_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_nast_shape_info_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut NastShapeInfo,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_nast_shape_info_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_enum__top_down(&mut self, env: &Env, elem: &mut Enum_) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_enum__top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_enum__bottom_up(&mut self, env: &Env, elem: &mut Enum_) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_enum__bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_where_constraint_hint_top_down(
        &mut self,
        env: &Env,
        elem: &mut WhereConstraintHint,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_where_constraint_hint_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_where_constraint_hint_bottom_up(
        &mut self,
        env: &Env,
        elem: &mut WhereConstraintHint,
    ) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_where_constraint_hint_bottom_up(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_id_top_down(&mut self, env: &Env, elem: &mut Id) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_id_top_down(env, elem)?;
        }
        Continue(())
    }
    #[inline(always)]
    fn on_ty_id_bottom_up(&mut self, env: &Env, elem: &mut Id) -> ControlFlow<()> {
        for pass in &mut self.passes {
            pass.on_ty_id_bottom_up(env, elem)?;
        }
        Continue(())
    }
}

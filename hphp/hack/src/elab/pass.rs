// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<4d9a1329ace613d8916af30a7d95c0e3>>
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
pub trait Pass {
    #[inline(always)]
    fn on_ty_program_top_down(&mut self, elem: &mut Program<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_program_bottom_up(
        &mut self,
        elem: &mut Program<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_top_down(&mut self, elem: &mut Stmt<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt_bottom_up(&mut self, elem: &mut Stmt<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt__top_down(&mut self, elem: &mut Stmt_<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_stmt__bottom_up(&mut self, elem: &mut Stmt_<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_using_stmt_top_down(
        &mut self,
        elem: &mut UsingStmt<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_using_stmt_bottom_up(
        &mut self,
        elem: &mut UsingStmt<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_as_expr_top_down(&mut self, elem: &mut AsExpr<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_as_expr_bottom_up(&mut self, elem: &mut AsExpr<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_block_top_down(&mut self, elem: &mut Block<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_block_bottom_up(&mut self, elem: &mut Block<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_finally_block_top_down(
        &mut self,
        elem: &mut FinallyBlock<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_finally_block_bottom_up(
        &mut self,
        elem: &mut FinallyBlock<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_id_top_down(
        &mut self,
        elem: &mut ClassId<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_id_bottom_up(
        &mut self,
        elem: &mut ClassId<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_id__top_down(
        &mut self,
        elem: &mut ClassId_<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_id__bottom_up(
        &mut self,
        elem: &mut ClassId_<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expr_top_down(&mut self, elem: &mut Expr<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expr_bottom_up(&mut self, elem: &mut Expr<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_collection_targ_top_down(
        &mut self,
        elem: &mut CollectionTarg<Ex>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_collection_targ_bottom_up(
        &mut self,
        elem: &mut CollectionTarg<Ex>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_function_ptr_id_top_down(
        &mut self,
        elem: &mut FunctionPtrId<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_function_ptr_id_bottom_up(
        &mut self,
        elem: &mut FunctionPtrId<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expression_tree_top_down(
        &mut self,
        elem: &mut ExpressionTree<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expression_tree_bottom_up(
        &mut self,
        elem: &mut ExpressionTree<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expr__top_down(&mut self, elem: &mut Expr_<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_expr__bottom_up(&mut self, elem: &mut Expr_<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hole_source_top_down(&mut self, elem: &mut HoleSource, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hole_source_bottom_up(&mut self, elem: &mut HoleSource, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_get_expr_top_down(
        &mut self,
        elem: &mut ClassGetExpr<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_get_expr_bottom_up(
        &mut self,
        elem: &mut ClassGetExpr<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_case_top_down(&mut self, elem: &mut Case<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_case_bottom_up(&mut self, elem: &mut Case<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_default_case_top_down(
        &mut self,
        elem: &mut DefaultCase<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_default_case_bottom_up(
        &mut self,
        elem: &mut DefaultCase<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_catch_top_down(&mut self, elem: &mut Catch<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_catch_bottom_up(&mut self, elem: &mut Catch<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_field_top_down(&mut self, elem: &mut Field<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_field_bottom_up(&mut self, elem: &mut Field<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_afield_top_down(&mut self, elem: &mut Afield<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_afield_bottom_up(&mut self, elem: &mut Afield<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_simple_top_down(
        &mut self,
        elem: &mut XhpSimple<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_simple_bottom_up(
        &mut self,
        elem: &mut XhpSimple<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attribute_top_down(
        &mut self,
        elem: &mut XhpAttribute<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attribute_bottom_up(
        &mut self,
        elem: &mut XhpAttribute<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_param_top_down(
        &mut self,
        elem: &mut FunParam<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_param_bottom_up(
        &mut self,
        elem: &mut FunParam<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun__top_down(&mut self, elem: &mut Fun_<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun__bottom_up(&mut self, elem: &mut Fun_<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_fun__ret_top_down(&mut self, elem: &mut TypeHint<Ex>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_fun__ret_bottom_up(&mut self, elem: &mut TypeHint<Ex>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_efun_top_down(&mut self, elem: &mut Efun<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_efun_bottom_up(&mut self, elem: &mut Efun<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_func_body_top_down(
        &mut self,
        elem: &mut FuncBody<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_func_body_bottom_up(
        &mut self,
        elem: &mut FuncBody<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_hint_top_down(&mut self, elem: &mut TypeHint<Ex>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_hint_bottom_up(&mut self, elem: &mut TypeHint<Ex>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_targ_top_down(&mut self, elem: &mut Targ<Ex>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_targ_bottom_up(&mut self, elem: &mut Targ<Ex>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_user_attribute_top_down(
        &mut self,
        elem: &mut UserAttribute<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_user_attribute_bottom_up(
        &mut self,
        elem: &mut UserAttribute<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_file_attribute_top_down(
        &mut self,
        elem: &mut FileAttribute<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_file_attribute_bottom_up(
        &mut self,
        elem: &mut FileAttribute<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_tparam_top_down(&mut self, elem: &mut Tparam<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_tparam_bottom_up(&mut self, elem: &mut Tparam<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class__top_down(&mut self, elem: &mut Class_<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class__bottom_up(&mut self, elem: &mut Class_<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__tparams_top_down(
        &mut self,
        elem: &mut Vec<Tparam<Ex, En>>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__tparams_bottom_up(
        &mut self,
        elem: &mut Vec<Tparam<Ex, En>>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__extends_top_down(
        &mut self,
        elem: &mut Vec<ClassHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__extends_bottom_up(
        &mut self,
        elem: &mut Vec<ClassHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__uses_top_down(
        &mut self,
        elem: &mut Vec<TraitHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__uses_bottom_up(
        &mut self,
        elem: &mut Vec<TraitHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__xhp_attr_uses_top_down(
        &mut self,
        elem: &mut Vec<XhpAttrHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__xhp_attr_uses_bottom_up(
        &mut self,
        elem: &mut Vec<XhpAttrHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__reqs_top_down(
        &mut self,
        elem: &mut Vec<ClassReq>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__reqs_bottom_up(
        &mut self,
        elem: &mut Vec<ClassReq>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__implements_top_down(
        &mut self,
        elem: &mut Vec<ClassHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__implements_bottom_up(
        &mut self,
        elem: &mut Vec<ClassHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__consts_top_down(
        &mut self,
        elem: &mut Vec<ClassConst<Ex, En>>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__consts_bottom_up(
        &mut self,
        elem: &mut Vec<ClassConst<Ex, En>>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__xhp_attrs_top_down(
        &mut self,
        elem: &mut Vec<XhpAttr<Ex, En>>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__xhp_attrs_bottom_up(
        &mut self,
        elem: &mut Vec<XhpAttr<Ex, En>>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__user_attributes_top_down(
        &mut self,
        elem: &mut UserAttributes<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class__user_attributes_bottom_up(
        &mut self,
        elem: &mut UserAttributes<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_req_top_down(&mut self, elem: &mut ClassReq, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_req_bottom_up(&mut self, elem: &mut ClassReq, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr_top_down(
        &mut self,
        elem: &mut XhpAttr<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr_bottom_up(
        &mut self,
        elem: &mut XhpAttr<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_const_kind_top_down(
        &mut self,
        elem: &mut ClassConstKind<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_const_kind_bottom_up(
        &mut self,
        elem: &mut ClassConstKind<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_const_top_down(
        &mut self,
        elem: &mut ClassConst<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_const_bottom_up(
        &mut self,
        elem: &mut ClassConst<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_abstract_typeconst_top_down(
        &mut self,
        elem: &mut ClassAbstractTypeconst,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_abstract_typeconst_bottom_up(
        &mut self,
        elem: &mut ClassAbstractTypeconst,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_concrete_typeconst_top_down(
        &mut self,
        elem: &mut ClassConcreteTypeconst,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_concrete_typeconst_bottom_up(
        &mut self,
        elem: &mut ClassConcreteTypeconst,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst_top_down(
        &mut self,
        elem: &mut ClassTypeconst,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst_bottom_up(
        &mut self,
        elem: &mut ClassTypeconst,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst_def_top_down(
        &mut self,
        elem: &mut ClassTypeconstDef<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_typeconst_def_bottom_up(
        &mut self,
        elem: &mut ClassTypeconstDef<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr_info_top_down(
        &mut self,
        elem: &mut XhpAttrInfo,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_attr_info_bottom_up(
        &mut self,
        elem: &mut XhpAttrInfo,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_var_top_down(
        &mut self,
        elem: &mut ClassVar<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_class_var_bottom_up(
        &mut self,
        elem: &mut ClassVar<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class_var_type__top_down(
        &mut self,
        elem: &mut TypeHint<Ex>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_class_var_type__bottom_up(
        &mut self,
        elem: &mut TypeHint<Ex>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_method__top_down(&mut self, elem: &mut Method_<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_method__bottom_up(
        &mut self,
        elem: &mut Method_<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_method__ret_top_down(
        &mut self,
        elem: &mut TypeHint<Ex>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_method__ret_bottom_up(
        &mut self,
        elem: &mut TypeHint<Ex>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_typedef_top_down(&mut self, elem: &mut Typedef<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_typedef_bottom_up(
        &mut self,
        elem: &mut Typedef<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_gconst_top_down(&mut self, elem: &mut Gconst<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_gconst_bottom_up(&mut self, elem: &mut Gconst<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_gconst_value_top_down(
        &mut self,
        elem: &mut Expr<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_gconst_value_bottom_up(
        &mut self,
        elem: &mut Expr<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_def_top_down(&mut self, elem: &mut FunDef<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_fun_def_bottom_up(&mut self, elem: &mut FunDef<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_module_def_top_down(
        &mut self,
        elem: &mut ModuleDef<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_module_def_bottom_up(
        &mut self,
        elem: &mut ModuleDef<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_def_top_down(&mut self, elem: &mut Def<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_def_bottom_up(&mut self, elem: &mut Def<Ex, En>, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_child_top_down(&mut self, elem: &mut XhpChild, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_xhp_child_bottom_up(&mut self, elem: &mut XhpChild, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint_top_down(&mut self, elem: &mut Hint, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint_bottom_up(&mut self, elem: &mut Hint, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_user_attributes_top_down(
        &mut self,
        elem: &mut UserAttributes<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_user_attributes_bottom_up(
        &mut self,
        elem: &mut UserAttributes<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_contexts_top_down(&mut self, elem: &mut Contexts, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_contexts_bottom_up(&mut self, elem: &mut Contexts, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint_fun_top_down(&mut self, elem: &mut HintFun, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint_fun_bottom_up(&mut self, elem: &mut HintFun, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_hint_fun_return_ty_top_down(
        &mut self,
        elem: &mut Hint,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_fld_hint_fun_return_ty_bottom_up(
        &mut self,
        elem: &mut Hint,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint__top_down(&mut self, elem: &mut Hint_, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_hint__bottom_up(&mut self, elem: &mut Hint_, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_refinement_top_down(&mut self, elem: &mut Refinement, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_refinement_bottom_up(&mut self, elem: &mut Refinement, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_refinement_top_down(
        &mut self,
        elem: &mut TypeRefinement,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_refinement_bottom_up(
        &mut self,
        elem: &mut TypeRefinement,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_refinement_bounds_top_down(
        &mut self,
        elem: &mut TypeRefinementBounds,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_type_refinement_bounds_bottom_up(
        &mut self,
        elem: &mut TypeRefinementBounds,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_top_down(
        &mut self,
        elem: &mut CtxRefinement,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_bottom_up(
        &mut self,
        elem: &mut CtxRefinement,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_bounds_top_down(
        &mut self,
        elem: &mut CtxRefinementBounds,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_bounds_bottom_up(
        &mut self,
        elem: &mut CtxRefinementBounds,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_shape_field_info_top_down(
        &mut self,
        elem: &mut ShapeFieldInfo,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_shape_field_info_bottom_up(
        &mut self,
        elem: &mut ShapeFieldInfo,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_nast_shape_info_top_down(
        &mut self,
        elem: &mut NastShapeInfo,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_nast_shape_info_bottom_up(
        &mut self,
        elem: &mut NastShapeInfo,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_enum__top_down(&mut self, elem: &mut Enum_, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_enum__bottom_up(&mut self, elem: &mut Enum_, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_where_constraint_hint_top_down(
        &mut self,
        elem: &mut WhereConstraintHint,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_where_constraint_hint_bottom_up(
        &mut self,
        elem: &mut WhereConstraintHint,
        env: &Env,
    ) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_id_top_down(&mut self, elem: &mut Id, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
    #[inline(always)]
    fn on_ty_id_bottom_up(&mut self, elem: &mut Id, env: &Env) -> ControlFlow<()> {
        Continue(())
    }
}
pub struct Passes<P, Q>
where
    P: Pass,
    Q: Pass,
{
    pub fst: P,
    pub snd: Q,
}
impl<P, Q> Clone for Passes<P, Q>
where
    P: Pass + Clone,
    Q: Pass + Clone,
{
    fn clone(&self) -> Self {
        Passes {
            fst: self.fst.clone(),
            snd: self.snd.clone(),
        }
    }
}
impl<P, Q> Pass for Passes<P, Q>
where
    P: Pass,
    Q: Pass,
{
    #[inline(always)]
    fn on_ty_program_top_down(&mut self, elem: &mut Program<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_program_top_down(elem, env)?;
        self.snd.on_ty_program_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_program_bottom_up(
        &mut self,
        elem: &mut Program<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_program_bottom_up(elem, env)?;
        self.snd.on_ty_program_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_stmt_top_down(&mut self, elem: &mut Stmt<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_stmt_top_down(elem, env)?;
        self.snd.on_ty_stmt_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_stmt_bottom_up(&mut self, elem: &mut Stmt<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_stmt_bottom_up(elem, env)?;
        self.snd.on_ty_stmt_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_stmt__top_down(&mut self, elem: &mut Stmt_<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_stmt__top_down(elem, env)?;
        self.snd.on_ty_stmt__top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_stmt__bottom_up(&mut self, elem: &mut Stmt_<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_stmt__bottom_up(elem, env)?;
        self.snd.on_ty_stmt__bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_using_stmt_top_down(
        &mut self,
        elem: &mut UsingStmt<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_using_stmt_top_down(elem, env)?;
        self.snd.on_ty_using_stmt_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_using_stmt_bottom_up(
        &mut self,
        elem: &mut UsingStmt<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_using_stmt_bottom_up(elem, env)?;
        self.snd.on_ty_using_stmt_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_as_expr_top_down(&mut self, elem: &mut AsExpr<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_as_expr_top_down(elem, env)?;
        self.snd.on_ty_as_expr_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_as_expr_bottom_up(&mut self, elem: &mut AsExpr<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_as_expr_bottom_up(elem, env)?;
        self.snd.on_ty_as_expr_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_block_top_down(&mut self, elem: &mut Block<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_block_top_down(elem, env)?;
        self.snd.on_ty_block_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_block_bottom_up(&mut self, elem: &mut Block<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_block_bottom_up(elem, env)?;
        self.snd.on_ty_block_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_finally_block_top_down(
        &mut self,
        elem: &mut FinallyBlock<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_finally_block_top_down(elem, env)?;
        self.snd.on_ty_finally_block_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_finally_block_bottom_up(
        &mut self,
        elem: &mut FinallyBlock<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_finally_block_bottom_up(elem, env)?;
        self.snd.on_ty_finally_block_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_id_top_down(
        &mut self,
        elem: &mut ClassId<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_id_top_down(elem, env)?;
        self.snd.on_ty_class_id_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_id_bottom_up(
        &mut self,
        elem: &mut ClassId<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_id_bottom_up(elem, env)?;
        self.snd.on_ty_class_id_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_id__top_down(
        &mut self,
        elem: &mut ClassId_<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_id__top_down(elem, env)?;
        self.snd.on_ty_class_id__top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_id__bottom_up(
        &mut self,
        elem: &mut ClassId_<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_id__bottom_up(elem, env)?;
        self.snd.on_ty_class_id__bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_expr_top_down(&mut self, elem: &mut Expr<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_expr_top_down(elem, env)?;
        self.snd.on_ty_expr_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_expr_bottom_up(&mut self, elem: &mut Expr<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_expr_bottom_up(elem, env)?;
        self.snd.on_ty_expr_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_collection_targ_top_down(
        &mut self,
        elem: &mut CollectionTarg<Ex>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_collection_targ_top_down(elem, env)?;
        self.snd.on_ty_collection_targ_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_collection_targ_bottom_up(
        &mut self,
        elem: &mut CollectionTarg<Ex>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_collection_targ_bottom_up(elem, env)?;
        self.snd.on_ty_collection_targ_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_function_ptr_id_top_down(
        &mut self,
        elem: &mut FunctionPtrId<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_function_ptr_id_top_down(elem, env)?;
        self.snd.on_ty_function_ptr_id_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_function_ptr_id_bottom_up(
        &mut self,
        elem: &mut FunctionPtrId<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_function_ptr_id_bottom_up(elem, env)?;
        self.snd.on_ty_function_ptr_id_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_expression_tree_top_down(
        &mut self,
        elem: &mut ExpressionTree<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_expression_tree_top_down(elem, env)?;
        self.snd.on_ty_expression_tree_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_expression_tree_bottom_up(
        &mut self,
        elem: &mut ExpressionTree<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_expression_tree_bottom_up(elem, env)?;
        self.snd.on_ty_expression_tree_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_expr__top_down(&mut self, elem: &mut Expr_<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_expr__top_down(elem, env)?;
        self.snd.on_ty_expr__top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_expr__bottom_up(&mut self, elem: &mut Expr_<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_expr__bottom_up(elem, env)?;
        self.snd.on_ty_expr__bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_hole_source_top_down(&mut self, elem: &mut HoleSource, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_hole_source_top_down(elem, env)?;
        self.snd.on_ty_hole_source_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_hole_source_bottom_up(&mut self, elem: &mut HoleSource, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_hole_source_bottom_up(elem, env)?;
        self.snd.on_ty_hole_source_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_get_expr_top_down(
        &mut self,
        elem: &mut ClassGetExpr<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_get_expr_top_down(elem, env)?;
        self.snd.on_ty_class_get_expr_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_get_expr_bottom_up(
        &mut self,
        elem: &mut ClassGetExpr<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_get_expr_bottom_up(elem, env)?;
        self.snd.on_ty_class_get_expr_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_case_top_down(&mut self, elem: &mut Case<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_case_top_down(elem, env)?;
        self.snd.on_ty_case_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_case_bottom_up(&mut self, elem: &mut Case<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_case_bottom_up(elem, env)?;
        self.snd.on_ty_case_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_default_case_top_down(
        &mut self,
        elem: &mut DefaultCase<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_default_case_top_down(elem, env)?;
        self.snd.on_ty_default_case_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_default_case_bottom_up(
        &mut self,
        elem: &mut DefaultCase<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_default_case_bottom_up(elem, env)?;
        self.snd.on_ty_default_case_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_catch_top_down(&mut self, elem: &mut Catch<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_catch_top_down(elem, env)?;
        self.snd.on_ty_catch_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_catch_bottom_up(&mut self, elem: &mut Catch<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_catch_bottom_up(elem, env)?;
        self.snd.on_ty_catch_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_field_top_down(&mut self, elem: &mut Field<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_field_top_down(elem, env)?;
        self.snd.on_ty_field_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_field_bottom_up(&mut self, elem: &mut Field<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_field_bottom_up(elem, env)?;
        self.snd.on_ty_field_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_afield_top_down(&mut self, elem: &mut Afield<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_afield_top_down(elem, env)?;
        self.snd.on_ty_afield_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_afield_bottom_up(&mut self, elem: &mut Afield<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_afield_bottom_up(elem, env)?;
        self.snd.on_ty_afield_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_xhp_simple_top_down(
        &mut self,
        elem: &mut XhpSimple<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_xhp_simple_top_down(elem, env)?;
        self.snd.on_ty_xhp_simple_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_xhp_simple_bottom_up(
        &mut self,
        elem: &mut XhpSimple<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_xhp_simple_bottom_up(elem, env)?;
        self.snd.on_ty_xhp_simple_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_xhp_attribute_top_down(
        &mut self,
        elem: &mut XhpAttribute<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_xhp_attribute_top_down(elem, env)?;
        self.snd.on_ty_xhp_attribute_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_xhp_attribute_bottom_up(
        &mut self,
        elem: &mut XhpAttribute<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_xhp_attribute_bottom_up(elem, env)?;
        self.snd.on_ty_xhp_attribute_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_fun_param_top_down(
        &mut self,
        elem: &mut FunParam<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_fun_param_top_down(elem, env)?;
        self.snd.on_ty_fun_param_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_fun_param_bottom_up(
        &mut self,
        elem: &mut FunParam<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_fun_param_bottom_up(elem, env)?;
        self.snd.on_ty_fun_param_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_fun__top_down(&mut self, elem: &mut Fun_<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_fun__top_down(elem, env)?;
        self.snd.on_ty_fun__top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_fun__bottom_up(&mut self, elem: &mut Fun_<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_fun__bottom_up(elem, env)?;
        self.snd.on_ty_fun__bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_fld_fun__ret_top_down(&mut self, elem: &mut TypeHint<Ex>, env: &Env) -> ControlFlow<()> {
        self.fst.on_fld_fun__ret_top_down(elem, env)?;
        self.snd.on_fld_fun__ret_top_down(elem, env)
    }
    #[inline(always)]
    fn on_fld_fun__ret_bottom_up(&mut self, elem: &mut TypeHint<Ex>, env: &Env) -> ControlFlow<()> {
        self.fst.on_fld_fun__ret_bottom_up(elem, env)?;
        self.snd.on_fld_fun__ret_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_efun_top_down(&mut self, elem: &mut Efun<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_efun_top_down(elem, env)?;
        self.snd.on_ty_efun_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_efun_bottom_up(&mut self, elem: &mut Efun<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_efun_bottom_up(elem, env)?;
        self.snd.on_ty_efun_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_func_body_top_down(
        &mut self,
        elem: &mut FuncBody<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_func_body_top_down(elem, env)?;
        self.snd.on_ty_func_body_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_func_body_bottom_up(
        &mut self,
        elem: &mut FuncBody<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_func_body_bottom_up(elem, env)?;
        self.snd.on_ty_func_body_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_type_hint_top_down(&mut self, elem: &mut TypeHint<Ex>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_type_hint_top_down(elem, env)?;
        self.snd.on_ty_type_hint_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_type_hint_bottom_up(&mut self, elem: &mut TypeHint<Ex>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_type_hint_bottom_up(elem, env)?;
        self.snd.on_ty_type_hint_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_targ_top_down(&mut self, elem: &mut Targ<Ex>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_targ_top_down(elem, env)?;
        self.snd.on_ty_targ_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_targ_bottom_up(&mut self, elem: &mut Targ<Ex>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_targ_bottom_up(elem, env)?;
        self.snd.on_ty_targ_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_user_attribute_top_down(
        &mut self,
        elem: &mut UserAttribute<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_user_attribute_top_down(elem, env)?;
        self.snd.on_ty_user_attribute_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_user_attribute_bottom_up(
        &mut self,
        elem: &mut UserAttribute<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_user_attribute_bottom_up(elem, env)?;
        self.snd.on_ty_user_attribute_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_file_attribute_top_down(
        &mut self,
        elem: &mut FileAttribute<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_file_attribute_top_down(elem, env)?;
        self.snd.on_ty_file_attribute_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_file_attribute_bottom_up(
        &mut self,
        elem: &mut FileAttribute<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_file_attribute_bottom_up(elem, env)?;
        self.snd.on_ty_file_attribute_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_tparam_top_down(&mut self, elem: &mut Tparam<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_tparam_top_down(elem, env)?;
        self.snd.on_ty_tparam_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_tparam_bottom_up(&mut self, elem: &mut Tparam<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_tparam_bottom_up(elem, env)?;
        self.snd.on_ty_tparam_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_class__top_down(&mut self, elem: &mut Class_<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_class__top_down(elem, env)?;
        self.snd.on_ty_class__top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_class__bottom_up(&mut self, elem: &mut Class_<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_class__bottom_up(elem, env)?;
        self.snd.on_ty_class__bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__tparams_top_down(
        &mut self,
        elem: &mut Vec<Tparam<Ex, En>>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__tparams_top_down(elem, env)?;
        self.snd.on_fld_class__tparams_top_down(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__tparams_bottom_up(
        &mut self,
        elem: &mut Vec<Tparam<Ex, En>>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__tparams_bottom_up(elem, env)?;
        self.snd.on_fld_class__tparams_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__extends_top_down(
        &mut self,
        elem: &mut Vec<ClassHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__extends_top_down(elem, env)?;
        self.snd.on_fld_class__extends_top_down(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__extends_bottom_up(
        &mut self,
        elem: &mut Vec<ClassHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__extends_bottom_up(elem, env)?;
        self.snd.on_fld_class__extends_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__uses_top_down(
        &mut self,
        elem: &mut Vec<TraitHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__uses_top_down(elem, env)?;
        self.snd.on_fld_class__uses_top_down(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__uses_bottom_up(
        &mut self,
        elem: &mut Vec<TraitHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__uses_bottom_up(elem, env)?;
        self.snd.on_fld_class__uses_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__xhp_attr_uses_top_down(
        &mut self,
        elem: &mut Vec<XhpAttrHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__xhp_attr_uses_top_down(elem, env)?;
        self.snd.on_fld_class__xhp_attr_uses_top_down(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__xhp_attr_uses_bottom_up(
        &mut self,
        elem: &mut Vec<XhpAttrHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__xhp_attr_uses_bottom_up(elem, env)?;
        self.snd.on_fld_class__xhp_attr_uses_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__reqs_top_down(
        &mut self,
        elem: &mut Vec<ClassReq>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__reqs_top_down(elem, env)?;
        self.snd.on_fld_class__reqs_top_down(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__reqs_bottom_up(
        &mut self,
        elem: &mut Vec<ClassReq>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__reqs_bottom_up(elem, env)?;
        self.snd.on_fld_class__reqs_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__implements_top_down(
        &mut self,
        elem: &mut Vec<ClassHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__implements_top_down(elem, env)?;
        self.snd.on_fld_class__implements_top_down(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__implements_bottom_up(
        &mut self,
        elem: &mut Vec<ClassHint>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__implements_bottom_up(elem, env)?;
        self.snd.on_fld_class__implements_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__consts_top_down(
        &mut self,
        elem: &mut Vec<ClassConst<Ex, En>>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__consts_top_down(elem, env)?;
        self.snd.on_fld_class__consts_top_down(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__consts_bottom_up(
        &mut self,
        elem: &mut Vec<ClassConst<Ex, En>>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__consts_bottom_up(elem, env)?;
        self.snd.on_fld_class__consts_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__xhp_attrs_top_down(
        &mut self,
        elem: &mut Vec<XhpAttr<Ex, En>>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__xhp_attrs_top_down(elem, env)?;
        self.snd.on_fld_class__xhp_attrs_top_down(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__xhp_attrs_bottom_up(
        &mut self,
        elem: &mut Vec<XhpAttr<Ex, En>>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__xhp_attrs_bottom_up(elem, env)?;
        self.snd.on_fld_class__xhp_attrs_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__user_attributes_top_down(
        &mut self,
        elem: &mut UserAttributes<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class__user_attributes_top_down(elem, env)?;
        self.snd.on_fld_class__user_attributes_top_down(elem, env)
    }
    #[inline(always)]
    fn on_fld_class__user_attributes_bottom_up(
        &mut self,
        elem: &mut UserAttributes<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst
            .on_fld_class__user_attributes_bottom_up(elem, env)?;
        self.snd.on_fld_class__user_attributes_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_req_top_down(&mut self, elem: &mut ClassReq, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_class_req_top_down(elem, env)?;
        self.snd.on_ty_class_req_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_req_bottom_up(&mut self, elem: &mut ClassReq, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_class_req_bottom_up(elem, env)?;
        self.snd.on_ty_class_req_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_xhp_attr_top_down(
        &mut self,
        elem: &mut XhpAttr<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_xhp_attr_top_down(elem, env)?;
        self.snd.on_ty_xhp_attr_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_xhp_attr_bottom_up(
        &mut self,
        elem: &mut XhpAttr<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_xhp_attr_bottom_up(elem, env)?;
        self.snd.on_ty_xhp_attr_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_const_kind_top_down(
        &mut self,
        elem: &mut ClassConstKind<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_const_kind_top_down(elem, env)?;
        self.snd.on_ty_class_const_kind_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_const_kind_bottom_up(
        &mut self,
        elem: &mut ClassConstKind<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_const_kind_bottom_up(elem, env)?;
        self.snd.on_ty_class_const_kind_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_const_top_down(
        &mut self,
        elem: &mut ClassConst<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_const_top_down(elem, env)?;
        self.snd.on_ty_class_const_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_const_bottom_up(
        &mut self,
        elem: &mut ClassConst<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_const_bottom_up(elem, env)?;
        self.snd.on_ty_class_const_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_abstract_typeconst_top_down(
        &mut self,
        elem: &mut ClassAbstractTypeconst,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst
            .on_ty_class_abstract_typeconst_top_down(elem, env)?;
        self.snd.on_ty_class_abstract_typeconst_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_abstract_typeconst_bottom_up(
        &mut self,
        elem: &mut ClassAbstractTypeconst,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst
            .on_ty_class_abstract_typeconst_bottom_up(elem, env)?;
        self.snd.on_ty_class_abstract_typeconst_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_concrete_typeconst_top_down(
        &mut self,
        elem: &mut ClassConcreteTypeconst,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst
            .on_ty_class_concrete_typeconst_top_down(elem, env)?;
        self.snd.on_ty_class_concrete_typeconst_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_concrete_typeconst_bottom_up(
        &mut self,
        elem: &mut ClassConcreteTypeconst,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst
            .on_ty_class_concrete_typeconst_bottom_up(elem, env)?;
        self.snd.on_ty_class_concrete_typeconst_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_typeconst_top_down(
        &mut self,
        elem: &mut ClassTypeconst,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_typeconst_top_down(elem, env)?;
        self.snd.on_ty_class_typeconst_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_typeconst_bottom_up(
        &mut self,
        elem: &mut ClassTypeconst,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_typeconst_bottom_up(elem, env)?;
        self.snd.on_ty_class_typeconst_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_typeconst_def_top_down(
        &mut self,
        elem: &mut ClassTypeconstDef<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_typeconst_def_top_down(elem, env)?;
        self.snd.on_ty_class_typeconst_def_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_typeconst_def_bottom_up(
        &mut self,
        elem: &mut ClassTypeconstDef<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_typeconst_def_bottom_up(elem, env)?;
        self.snd.on_ty_class_typeconst_def_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_xhp_attr_info_top_down(
        &mut self,
        elem: &mut XhpAttrInfo,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_xhp_attr_info_top_down(elem, env)?;
        self.snd.on_ty_xhp_attr_info_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_xhp_attr_info_bottom_up(
        &mut self,
        elem: &mut XhpAttrInfo,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_xhp_attr_info_bottom_up(elem, env)?;
        self.snd.on_ty_xhp_attr_info_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_var_top_down(
        &mut self,
        elem: &mut ClassVar<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_var_top_down(elem, env)?;
        self.snd.on_ty_class_var_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_class_var_bottom_up(
        &mut self,
        elem: &mut ClassVar<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_class_var_bottom_up(elem, env)?;
        self.snd.on_ty_class_var_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_fld_class_var_type__top_down(
        &mut self,
        elem: &mut TypeHint<Ex>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class_var_type__top_down(elem, env)?;
        self.snd.on_fld_class_var_type__top_down(elem, env)
    }
    #[inline(always)]
    fn on_fld_class_var_type__bottom_up(
        &mut self,
        elem: &mut TypeHint<Ex>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_class_var_type__bottom_up(elem, env)?;
        self.snd.on_fld_class_var_type__bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_method__top_down(&mut self, elem: &mut Method_<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_method__top_down(elem, env)?;
        self.snd.on_ty_method__top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_method__bottom_up(
        &mut self,
        elem: &mut Method_<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_method__bottom_up(elem, env)?;
        self.snd.on_ty_method__bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_fld_method__ret_top_down(
        &mut self,
        elem: &mut TypeHint<Ex>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_method__ret_top_down(elem, env)?;
        self.snd.on_fld_method__ret_top_down(elem, env)
    }
    #[inline(always)]
    fn on_fld_method__ret_bottom_up(
        &mut self,
        elem: &mut TypeHint<Ex>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_method__ret_bottom_up(elem, env)?;
        self.snd.on_fld_method__ret_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_typedef_top_down(&mut self, elem: &mut Typedef<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_typedef_top_down(elem, env)?;
        self.snd.on_ty_typedef_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_typedef_bottom_up(
        &mut self,
        elem: &mut Typedef<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_typedef_bottom_up(elem, env)?;
        self.snd.on_ty_typedef_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_gconst_top_down(&mut self, elem: &mut Gconst<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_gconst_top_down(elem, env)?;
        self.snd.on_ty_gconst_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_gconst_bottom_up(&mut self, elem: &mut Gconst<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_gconst_bottom_up(elem, env)?;
        self.snd.on_ty_gconst_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_fld_gconst_value_top_down(
        &mut self,
        elem: &mut Expr<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_gconst_value_top_down(elem, env)?;
        self.snd.on_fld_gconst_value_top_down(elem, env)
    }
    #[inline(always)]
    fn on_fld_gconst_value_bottom_up(
        &mut self,
        elem: &mut Expr<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_gconst_value_bottom_up(elem, env)?;
        self.snd.on_fld_gconst_value_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_fun_def_top_down(&mut self, elem: &mut FunDef<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_fun_def_top_down(elem, env)?;
        self.snd.on_ty_fun_def_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_fun_def_bottom_up(&mut self, elem: &mut FunDef<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_fun_def_bottom_up(elem, env)?;
        self.snd.on_ty_fun_def_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_module_def_top_down(
        &mut self,
        elem: &mut ModuleDef<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_module_def_top_down(elem, env)?;
        self.snd.on_ty_module_def_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_module_def_bottom_up(
        &mut self,
        elem: &mut ModuleDef<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_module_def_bottom_up(elem, env)?;
        self.snd.on_ty_module_def_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_def_top_down(&mut self, elem: &mut Def<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_def_top_down(elem, env)?;
        self.snd.on_ty_def_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_def_bottom_up(&mut self, elem: &mut Def<Ex, En>, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_def_bottom_up(elem, env)?;
        self.snd.on_ty_def_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_xhp_child_top_down(&mut self, elem: &mut XhpChild, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_xhp_child_top_down(elem, env)?;
        self.snd.on_ty_xhp_child_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_xhp_child_bottom_up(&mut self, elem: &mut XhpChild, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_xhp_child_bottom_up(elem, env)?;
        self.snd.on_ty_xhp_child_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_hint_top_down(&mut self, elem: &mut Hint, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_hint_top_down(elem, env)?;
        self.snd.on_ty_hint_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_hint_bottom_up(&mut self, elem: &mut Hint, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_hint_bottom_up(elem, env)?;
        self.snd.on_ty_hint_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_user_attributes_top_down(
        &mut self,
        elem: &mut UserAttributes<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_user_attributes_top_down(elem, env)?;
        self.snd.on_ty_user_attributes_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_user_attributes_bottom_up(
        &mut self,
        elem: &mut UserAttributes<Ex, En>,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_user_attributes_bottom_up(elem, env)?;
        self.snd.on_ty_user_attributes_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_contexts_top_down(&mut self, elem: &mut Contexts, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_contexts_top_down(elem, env)?;
        self.snd.on_ty_contexts_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_contexts_bottom_up(&mut self, elem: &mut Contexts, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_contexts_bottom_up(elem, env)?;
        self.snd.on_ty_contexts_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_hint_fun_top_down(&mut self, elem: &mut HintFun, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_hint_fun_top_down(elem, env)?;
        self.snd.on_ty_hint_fun_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_hint_fun_bottom_up(&mut self, elem: &mut HintFun, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_hint_fun_bottom_up(elem, env)?;
        self.snd.on_ty_hint_fun_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_fld_hint_fun_return_ty_top_down(
        &mut self,
        elem: &mut Hint,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_hint_fun_return_ty_top_down(elem, env)?;
        self.snd.on_fld_hint_fun_return_ty_top_down(elem, env)
    }
    #[inline(always)]
    fn on_fld_hint_fun_return_ty_bottom_up(
        &mut self,
        elem: &mut Hint,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_fld_hint_fun_return_ty_bottom_up(elem, env)?;
        self.snd.on_fld_hint_fun_return_ty_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_hint__top_down(&mut self, elem: &mut Hint_, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_hint__top_down(elem, env)?;
        self.snd.on_ty_hint__top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_hint__bottom_up(&mut self, elem: &mut Hint_, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_hint__bottom_up(elem, env)?;
        self.snd.on_ty_hint__bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_refinement_top_down(&mut self, elem: &mut Refinement, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_refinement_top_down(elem, env)?;
        self.snd.on_ty_refinement_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_refinement_bottom_up(&mut self, elem: &mut Refinement, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_refinement_bottom_up(elem, env)?;
        self.snd.on_ty_refinement_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_type_refinement_top_down(
        &mut self,
        elem: &mut TypeRefinement,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_type_refinement_top_down(elem, env)?;
        self.snd.on_ty_type_refinement_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_type_refinement_bottom_up(
        &mut self,
        elem: &mut TypeRefinement,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_type_refinement_bottom_up(elem, env)?;
        self.snd.on_ty_type_refinement_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_type_refinement_bounds_top_down(
        &mut self,
        elem: &mut TypeRefinementBounds,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_type_refinement_bounds_top_down(elem, env)?;
        self.snd.on_ty_type_refinement_bounds_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_type_refinement_bounds_bottom_up(
        &mut self,
        elem: &mut TypeRefinementBounds,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_type_refinement_bounds_bottom_up(elem, env)?;
        self.snd.on_ty_type_refinement_bounds_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_top_down(
        &mut self,
        elem: &mut CtxRefinement,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_ctx_refinement_top_down(elem, env)?;
        self.snd.on_ty_ctx_refinement_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_bottom_up(
        &mut self,
        elem: &mut CtxRefinement,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_ctx_refinement_bottom_up(elem, env)?;
        self.snd.on_ty_ctx_refinement_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_bounds_top_down(
        &mut self,
        elem: &mut CtxRefinementBounds,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_ctx_refinement_bounds_top_down(elem, env)?;
        self.snd.on_ty_ctx_refinement_bounds_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_ctx_refinement_bounds_bottom_up(
        &mut self,
        elem: &mut CtxRefinementBounds,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_ctx_refinement_bounds_bottom_up(elem, env)?;
        self.snd.on_ty_ctx_refinement_bounds_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_shape_field_info_top_down(
        &mut self,
        elem: &mut ShapeFieldInfo,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_shape_field_info_top_down(elem, env)?;
        self.snd.on_ty_shape_field_info_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_shape_field_info_bottom_up(
        &mut self,
        elem: &mut ShapeFieldInfo,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_shape_field_info_bottom_up(elem, env)?;
        self.snd.on_ty_shape_field_info_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_nast_shape_info_top_down(
        &mut self,
        elem: &mut NastShapeInfo,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_nast_shape_info_top_down(elem, env)?;
        self.snd.on_ty_nast_shape_info_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_nast_shape_info_bottom_up(
        &mut self,
        elem: &mut NastShapeInfo,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_nast_shape_info_bottom_up(elem, env)?;
        self.snd.on_ty_nast_shape_info_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_enum__top_down(&mut self, elem: &mut Enum_, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_enum__top_down(elem, env)?;
        self.snd.on_ty_enum__top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_enum__bottom_up(&mut self, elem: &mut Enum_, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_enum__bottom_up(elem, env)?;
        self.snd.on_ty_enum__bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_where_constraint_hint_top_down(
        &mut self,
        elem: &mut WhereConstraintHint,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_where_constraint_hint_top_down(elem, env)?;
        self.snd.on_ty_where_constraint_hint_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_where_constraint_hint_bottom_up(
        &mut self,
        elem: &mut WhereConstraintHint,
        env: &Env,
    ) -> ControlFlow<()> {
        self.fst.on_ty_where_constraint_hint_bottom_up(elem, env)?;
        self.snd.on_ty_where_constraint_hint_bottom_up(elem, env)
    }
    #[inline(always)]
    fn on_ty_id_top_down(&mut self, elem: &mut Id, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_id_top_down(elem, env)?;
        self.snd.on_ty_id_top_down(elem, env)
    }
    #[inline(always)]
    fn on_ty_id_bottom_up(&mut self, elem: &mut Id, env: &Env) -> ControlFlow<()> {
        self.fst.on_ty_id_bottom_up(elem, env)?;
        self.snd.on_ty_id_bottom_up(elem, env)
    }
}

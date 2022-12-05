(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast

type ('env, 'err) t

module Cont : sig
  type 'a t

  val next : 'a -> 'a t

  val finish : 'a -> 'a t
end

type 'a transform = 'a -> 'a Cont.t

type ('env, 'err) pass = {
  on_program: ('env * (unit, unit) Aast.program * 'err) transform option;
  on_class_: ('env * (unit, unit) Aast.class_ * 'err) transform option;
  on_class_c_tparams:
    ('env * (unit, unit) Aast.tparam list * 'err) transform option;
  on_class_c_extends: ('env * Aast.hint list * 'err) transform option;
  on_class_c_uses: ('env * Aast.hint list * 'err) transform option;
  on_class_c_xhp_attrs:
    ('env * (unit, unit) Aast.xhp_attr list * 'err) transform option;
  on_class_c_xhp_attr_uses: ('env * Aast.hint list * 'err) transform option;
  on_class_c_req:
    ('env * (Aast.hint * Aast.require_kind) * 'err) transform option;
  on_class_c_reqs:
    ('env * (Aast.hint * Aast.require_kind) list * 'err) transform option;
  on_class_c_implements: ('env * Aast.hint list * 'err) transform option;
  on_class_c_where_constraints:
    ('env * Aast.where_constraint_hint list * 'err) transform option;
  on_class_c_consts:
    ('env * (unit, unit) Aast.class_const list * 'err) transform option;
  on_class_c_typeconsts:
    ('env * (unit, unit) Aast.class_typeconst_def list * 'err) transform option;
  on_class_c_vars:
    ('env * (unit, unit) Aast.class_var list * 'err) transform option;
  on_class_c_enum: ('env * Aast.enum_ option * 'err) transform option;
  on_class_c_methods:
    ('env * (unit, unit) Aast.method_ list * 'err) transform option;
  on_class_c_user_attributes:
    ('env * (unit, unit) Aast.user_attribute list * 'err) transform option;
  on_class_c_file_attributes:
    ('env * (unit, unit) Aast.file_attribute list * 'err) transform option;
  on_class_var: ('env * (unit, unit) Aast.class_var * 'err) transform option;
  on_class_var_cv_user_attributes:
    ('env * (unit, unit) Aast.user_attribute list * 'err) transform option;
  on_class_var_cv_expr:
    ('env * (unit, unit) Aast.expr option * 'err) transform option;
  on_class_var_cv_type: ('env * unit Aast.type_hint * 'err) transform option;
  on_class_const_kind:
    ('env * (unit, unit) Aast.class_const_kind * 'err) transform option;
  on_typedef: ('env * (unit, unit) Aast.typedef * 'err) transform option;
  on_gconst: ('env * (unit, unit) Aast.gconst * 'err) transform option;
  on_gconst_cst_type: ('env * Aast.hint option * 'err) transform option;
  on_gconst_cst_value: ('env * (unit, unit) Aast.expr * 'err) transform option;
  on_fun_def: ('env * (unit, unit) Aast.fun_def * 'err) transform option;
  on_module_def: ('env * (unit, unit) Aast.module_def * 'err) transform option;
  on_stmt: ('env * (unit, unit) Aast.stmt * 'err) transform option;
  on_stmt_: ('env * (unit, unit) Aast.stmt_ * 'err) transform option;
  on_block: ('env * (unit, unit) Aast.block * 'err) transform option;
  on_using_stmt: ('env * (unit, unit) Aast.using_stmt * 'err) transform option;
  on_hint: ('env * Aast.hint * 'err) transform option;
  on_hint_: ('env * Aast.hint_ * 'err) transform option;
  on_hint_fun: ('env * Aast.hint_fun * 'err) transform option;
  on_hint_fun_hf_param_tys: ('env * Aast.hint list * 'err) transform option;
  on_hint_fun_hf_variadic_ty:
    ('env * Aast.variadic_hint * 'err) transform option;
  on_hint_fun_hf_ctxs: ('env * Aast.contexts option * 'err) transform option;
  on_hint_fun_hf_return_ty: ('env * Aast.hint * 'err) transform option;
  on_nast_shape_info: ('env * Aast.nast_shape_info * 'err) transform option;
  on_shape_field_info: ('env * Aast.shape_field_info * 'err) transform option;
  on_expr: ('env * (unit, unit) Aast.expr * 'err) transform option;
  on_expr_: ('env * (unit, unit) Aast.expr_ * 'err) transform option;
  on_fun_: ('env * (unit, unit) Aast.fun_ * 'err) transform option;
  on_fun_f_ret: ('env * unit Aast.type_hint * 'err) transform option;
  on_fun_f_tparams:
    ('env * (unit, unit) Aast.tparam list * 'err) transform option;
  on_fun_f_where_constraints:
    ('env * Aast.where_constraint_hint list * 'err) transform option;
  on_fun_f_params:
    ('env * (unit, unit) Aast.fun_param list * 'err) transform option;
  on_fun_f_ctxs: ('env * Aast.contexts option * 'err) transform option;
  on_fun_f_unsafe_ctxs: ('env * Aast.contexts option * 'err) transform option;
  on_fun_f_body: ('env * (unit, unit) Aast.func_body * 'err) transform option;
  on_fun_f_user_attributes:
    ('env * (unit, unit) Aast.user_attribute list * 'err) transform option;
  on_method_: ('env * (unit, unit) Aast.method_ * 'err) transform option;
  on_method_m_ret: ('env * unit Aast.type_hint * 'err) transform option;
  on_method_m_tparams:
    ('env * (unit, unit) Aast.tparam list * 'err) transform option;
  on_method_m_where_constraints:
    ('env * Aast.where_constraint_hint list * 'err) transform option;
  on_method_m_params:
    ('env * (unit, unit) Aast.fun_param list * 'err) transform option;
  on_method_m_ctxs: ('env * Aast.contexts option * 'err) transform option;
  on_method_m_unsafe_ctxs:
    ('env * Aast.contexts option * 'err) transform option;
  on_method_m_body:
    ('env * (unit, unit) Aast.func_body * 'err) transform option;
  on_method_m_user_attributes:
    ('env * (unit, unit) Aast.user_attribute list * 'err) transform option;
  on_class_id: ('env * (unit, unit) Aast.class_id * 'err) transform option;
  on_class_id_: ('env * (unit, unit) Aast.class_id_ * 'err) transform option;
  on_func_body: ('env * (unit, unit) Aast.func_body * 'err) transform option;
  on_enum_: ('env * Aast.enum_ * 'err) transform option;
  on_tparam: ('env * (unit, unit) Aast.tparam * 'err) transform option;
  on_user_attributes:
    ('env * (unit, unit) Aast.user_attribute list * 'err) transform option;
  on_where_constraint_hint:
    ('env * Aast.where_constraint_hint * 'err) transform option;
  on_contexts: ('env * Aast.contexts * 'err) transform option;
  on_context: ('env * Aast.hint * 'err) transform option;
  on_targ: ('env * unit Aast.targ * 'err) transform option;
  on_as_expr: ('env * (unit, unit) Aast.as_expr * 'err) transform option;
}

val identity : ('env, 'err) pass

val top_down : ('env, 'err) pass -> ('env, 'err) t

val bottom_up : ('env, 'err) pass -> ('env, 'err) t

val mk_visitor :
  ('a, Naming_phase_error.err Naming_phase_error.Free_monoid.t) t list ->
  < on_'en :
      'a ->
      unit ->
      unit * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_'ex :
      'a ->
      unit ->
      unit * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_AFkvalue :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) afield
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_AFvalue :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) afield
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Abstract :
      'a ->
      Ast_defs.abstraction
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Amp :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Ampamp :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Anonymous :
      'a -> emit_id * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Array_get :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr option ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_As :
      'a ->
      (unit, unit) Aast_defs.expr ->
      Aast_defs.hint ->
      is_variadic ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_As_kv :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) as_expr
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_As_v :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) as_expr
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_AssertEnv :
      'a ->
      env_annot ->
      (pos * unit) local_id_map ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Await :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Await_as_kv :
      'a ->
      pos ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) as_expr
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Await_as_v :
      'a ->
      pos ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) as_expr
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Awaitall :
      'a ->
      (Aast_defs.lid option * (unit, unit) Aast_defs.expr) list ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Bar :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Barbar :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Binop :
      'a ->
      Ast_defs.bop ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Block :
      'a ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Break :
      'a ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_CCAbstract :
      'a ->
      (unit, unit) Aast_defs.expr option ->
      (unit, unit) class_const_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_CCConcrete :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) class_const_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_CGexpr :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) class_get_expr
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_CGstring :
      'a ->
      pstring ->
      (unit, unit) class_get_expr
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_CI :
      'a ->
      sid ->
      (unit, unit) class_id_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_CIexpr :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) class_id_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_CIparent :
      'a ->
      (unit, unit) class_id_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_CIself :
      'a ->
      (unit, unit) class_id_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_CIstatic :
      'a ->
      (unit, unit) class_id_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_CRexact :
      'a ->
      Aast_defs.hint ->
      ctx_refinement * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_CRloose :
      'a ->
      ctx_refinement_bounds ->
      ctx_refinement * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Call :
      'a ->
      (unit, unit) Aast_defs.expr ->
      unit Aast_defs.targ list ->
      (Ast_defs.param_kind * (unit, unit) Aast_defs.expr) list ->
      (unit, unit) Aast_defs.expr option ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Case :
      'a ->
      (unit, unit) Aast_defs.case ->
      (unit, unit) gen_case
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Cast :
      'a ->
      Aast_defs.hint ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Cclass :
      'a ->
      Ast_defs.abstraction ->
      Ast_defs.classish_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Cenum :
      'a ->
      Ast_defs.classish_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Cenum_class :
      'a ->
      Ast_defs.abstraction ->
      Ast_defs.classish_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ChildBinary :
      'a ->
      xhp_child ->
      xhp_child ->
      xhp_child * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ChildList :
      'a ->
      xhp_child list ->
      xhp_child * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ChildName :
      'a ->
      sid ->
      xhp_child * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ChildPlus :
      'a ->
      xhp_child_op * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ChildQuestion :
      'a ->
      xhp_child_op * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ChildStar :
      'a ->
      xhp_child_op * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ChildUnary :
      'a ->
      xhp_child ->
      xhp_child_op ->
      xhp_child * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Cinterface :
      'a ->
      Ast_defs.classish_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Class :
      'a ->
      (unit, unit) class_ ->
      (unit, unit) def * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Class_const :
      'a ->
      (unit, unit) Aast_defs.class_id ->
      pstring ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Class_get :
      'a ->
      (unit, unit) Aast_defs.class_id ->
      (unit, unit) class_get_expr ->
      prop_or_method ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Clone :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Cmp :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Collection :
      'a ->
      sid ->
      unit collection_targ option ->
      (unit, unit) afield list ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_CollectionTKV :
      'a ->
      unit Aast_defs.targ ->
      unit Aast_defs.targ ->
      unit collection_targ
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_CollectionTV :
      'a ->
      unit Aast_defs.targ ->
      unit collection_targ
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Concrete :
      'a ->
      Ast_defs.abstraction
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Constant :
      'a ->
      (unit, unit) gconst ->
      (unit, unit) def * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Constraint_as :
      'a ->
      Ast_defs.constraint_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Constraint_eq :
      'a ->
      Ast_defs.constraint_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Constraint_super :
      'a ->
      Ast_defs.constraint_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Continue :
      'a ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Contravariant :
      'a ->
      Ast_defs.variance
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Covariant :
      'a ->
      Ast_defs.variance
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Ctrait :
      'a ->
      Ast_defs.classish_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Darray :
      'a ->
      (unit Aast_defs.targ * unit Aast_defs.targ) option ->
      ((unit, unit) Aast_defs.expr * (unit, unit) Aast_defs.expr) list ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Default :
      'a ->
      (unit, unit) Aast_defs.default_case ->
      (unit, unit) gen_case
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Dict :
      'a -> kvc_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Diff :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Diff2 :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Do :
      'a ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Dollardollar :
      'a ->
      Aast_defs.lid ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Dot :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ET_Splice :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Efun :
      'a ->
      (unit, unit) fun_ ->
      Aast_defs.lid list ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Eif :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr option ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Emit_id :
      'a ->
      int ->
      emit_id * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_EnforcedCast :
      'a ->
      Aast_defs.hint list ->
      hole_source * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_EnumClassLabel :
      'a ->
      sid option ->
      byte_string ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Eq :
      'a ->
      Ast_defs.bop option ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Eqeq :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Eqeqeq :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Erased :
      'a -> reify_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Expr :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ExpressionTree :
      'a ->
      (unit, unit) expression_tree ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_FAsync :
      'a ->
      Ast_defs.fun_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_FAsyncGenerator :
      'a ->
      Ast_defs.fun_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_FGenerator :
      'a ->
      Ast_defs.fun_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_FP_class_const :
      'a ->
      (unit, unit) Aast_defs.class_id ->
      pstring ->
      (unit, unit) function_ptr_id
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_FP_id :
      'a ->
      sid ->
      (unit, unit) function_ptr_id
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_FSync :
      'a ->
      Ast_defs.fun_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Fallthrough :
      'a ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_False :
      'a ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_FileAttributes :
      'a ->
      (unit, unit) file_attribute ->
      (unit, unit) def * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Float :
      'a ->
      byte_string ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_For :
      'a ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) Aast_defs.expr option ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Foreach :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) as_expr ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Fun :
      'a ->
      (unit, unit) fun_def ->
      (unit, unit) def * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Fun_id :
      'a ->
      sid ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_FunctionPointer :
      'a ->
      (unit, unit) function_ptr_id ->
      unit Aast_defs.targ list ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Gt :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Gte :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Gtgt :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Habstr :
      'a ->
      byte_string ->
      Aast_defs.hint list ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Haccess :
      'a ->
      Aast_defs.hint ->
      sid list ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hany :
      'a -> hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Happly :
      'a ->
      sid ->
      Aast_defs.hint list ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hdynamic :
      'a -> hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Herr :
      'a -> hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hfun :
      'a ->
      hint_fun ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hfun_context :
      'a ->
      byte_string ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hintersection :
      'a ->
      Aast_defs.hint list ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hlike :
      'a ->
      Aast_defs.hint ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hmixed :
      'a -> hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hnonnull :
      'a -> hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hnothing :
      'a -> hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hole :
      'a ->
      (unit, unit) Aast_defs.expr ->
      unit ->
      unit ->
      hole_source ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hoption :
      'a ->
      Aast_defs.hint ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hprim :
      'a ->
      tprim ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hrefinement :
      'a ->
      Aast_defs.hint ->
      refinement list ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hshape :
      'a ->
      nast_shape_info ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hsoft :
      'a ->
      Aast_defs.hint ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hthis :
      'a -> hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Htuple :
      'a ->
      Aast_defs.hint list ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hunion :
      'a ->
      Aast_defs.hint list ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hvar :
      'a ->
      byte_string ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Hvec_or_dict :
      'a ->
      Aast_defs.hint option ->
      Aast_defs.hint ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Id :
      'a ->
      sid ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_If :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ImmMap :
      'a -> kvc_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ImmSet :
      'a -> vc_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ImmVector :
      'a -> vc_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Import :
      'a ->
      import_flavor ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Include :
      'a ->
      import_flavor * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_IncludeOnce :
      'a ->
      import_flavor * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Int :
      'a ->
      byte_string ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Invariant :
      'a ->
      Ast_defs.variance
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Is :
      'a ->
      (unit, unit) Aast_defs.expr ->
      Aast_defs.hint ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Is_method :
      'a ->
      prop_or_method * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Is_prop :
      'a ->
      prop_or_method * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Join :
      'a -> env_annot * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_KeyValCollection :
      'a ->
      kvc_kind ->
      (unit Aast_defs.targ * unit Aast_defs.targ) option ->
      (unit, unit) Aast_defs.field list ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Keyset :
      'a -> vc_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_LateInit :
      'a ->
      xhp_attr_tag * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Lfun :
      'a ->
      (unit, unit) fun_ ->
      Aast_defs.lid list ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_List :
      'a ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Lplaceholder :
      'a ->
      pos ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Lt :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Lte :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Ltlt :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Lvar :
      'a ->
      Aast_defs.lid ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_MDNameExact :
      'a ->
      sid ->
      md_name_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_MDNameGlobal :
      'a ->
      pos ->
      md_name_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_MDNamePrefix :
      'a ->
      sid ->
      md_name_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Map :
      'a -> kvc_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Markup :
      'a ->
      pstring ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Method_caller :
      'a ->
      sid ->
      pstring ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Method_id :
      'a ->
      (unit, unit) Aast_defs.expr ->
      pstring ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Minus :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Module :
      'a ->
      (unit, unit) module_def ->
      (unit, unit) def * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_NSClass :
      'a -> ns_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_NSClassAndNamespace :
      'a -> ns_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_NSConst :
      'a -> ns_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_NSFun :
      'a -> ns_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_NSNamespace :
      'a -> ns_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Namespace :
      'a ->
      sid ->
      (unit, unit) def list ->
      (unit, unit) def * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_NamespaceUse :
      'a ->
      (ns_kind * sid * sid) list ->
      (unit, unit) def * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_New :
      'a ->
      (unit, unit) Aast_defs.class_id ->
      unit Aast_defs.targ list ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) Aast_defs.expr option ->
      unit ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Noop :
      'a ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Null :
      'a ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_OG_nullsafe :
      'a ->
      og_null_flavor * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_OG_nullthrows :
      'a ->
      og_null_flavor * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Obj_get :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr ->
      og_null_flavor ->
      prop_or_method ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Omitted :
      'a ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Opaque :
      'a ->
      typedef_visibility
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_OpaqueModule :
      'a ->
      typedef_visibility
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Pair :
      'a ->
      (unit Aast_defs.targ * unit Aast_defs.targ) option ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Percent :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Pinout :
      'a ->
      pos ->
      Ast_defs.param_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Pipe :
      'a ->
      Aast_defs.lid ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Plus :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Pnormal :
      'a ->
      Ast_defs.param_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_PrefixedString :
      'a ->
      byte_string ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_QuestionQuestion :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Rctx :
      'a ->
      sid ->
      ctx_refinement ->
      refinement * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Readonly :
      'a ->
      Ast_defs.readonly_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ReadonlyExpr :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Refinement :
      'a -> env_annot * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Reified :
      'a -> reify_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Require :
      'a ->
      import_flavor * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_RequireClass :
      'a ->
      require_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_RequireExtends :
      'a ->
      require_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_RequireImplements :
      'a ->
      require_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_RequireOnce :
      'a ->
      import_flavor * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Required :
      'a ->
      xhp_attr_tag * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Return :
      'a ->
      (unit, unit) Aast_defs.expr option ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Rtype :
      'a ->
      sid ->
      type_refinement ->
      refinement * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_SFclass_const :
      'a ->
      sid ->
      pstring ->
      Ast_defs.shape_field_name
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_SFlit_int :
      'a ->
      pstring ->
      Ast_defs.shape_field_name
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_SFlit_str :
      'a ->
      positioned_byte_string ->
      Ast_defs.shape_field_name
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Set :
      'a -> vc_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_SetModule :
      'a ->
      sid ->
      (unit, unit) def * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_SetNamespaceEnv :
      'a ->
      nsenv ->
      (unit, unit) def * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Shape :
      'a ->
      (Ast_defs.shape_field_name * (unit, unit) Aast_defs.expr) list ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Slash :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Smethod_id :
      'a ->
      (unit, unit) Aast_defs.class_id ->
      pstring ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_SoftReified :
      'a -> reify_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Star :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Starstar :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Stmt :
      'a ->
      (unit, unit) Aast_defs.stmt ->
      (unit, unit) def * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_String :
      'a ->
      byte_string ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_String2 :
      'a ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Switch :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.case list ->
      (unit, unit) Aast_defs.default_case option ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_TCAbstract :
      'a ->
      class_abstract_typeconst ->
      class_typeconst * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_TCConcrete :
      'a ->
      class_concrete_typeconst ->
      class_typeconst * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_TRexact :
      'a ->
      Aast_defs.hint ->
      type_refinement * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_TRloose :
      'a ->
      type_refinement_bounds ->
      type_refinement * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Tarraykey :
      'a -> tprim * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Tbool :
      'a -> tprim * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Tfloat :
      'a -> tprim * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_This :
      'a ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Throw :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Tint :
      'a -> tprim * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Tnoreturn :
      'a -> tprim * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Tnull :
      'a -> tprim * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Tnum :
      'a -> tprim * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Transparent :
      'a ->
      typedef_visibility
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Tresource :
      'a -> tprim * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_True :
      'a ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Try :
      'a ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) Aast_defs.catch list ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Tstring :
      'a -> tprim * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Tuple :
      'a ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Tvoid :
      'a -> tprim * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Typedef :
      'a ->
      (unit, unit) typedef ->
      (unit, unit) def * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Typing :
      'a ->
      hole_source * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Udecr :
      'a ->
      Ast_defs.uop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Uincr :
      'a ->
      Ast_defs.uop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Uminus :
      'a ->
      Ast_defs.uop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Unop :
      'a ->
      Ast_defs.uop ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Unot :
      'a ->
      Ast_defs.uop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_UnsafeCast :
      'a ->
      Aast_defs.hint list ->
      hole_source * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_UnsafeNonnullCast :
      'a ->
      hole_source * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Upcast :
      'a ->
      (unit, unit) Aast_defs.expr ->
      Aast_defs.hint ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Updecr :
      'a ->
      Ast_defs.uop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Upincr :
      'a ->
      Ast_defs.uop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Uplus :
      'a ->
      Ast_defs.uop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Usilence :
      'a ->
      Ast_defs.uop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Using :
      'a ->
      (unit, unit) using_stmt ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Utild :
      'a ->
      Ast_defs.uop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ValCollection :
      'a ->
      vc_kind ->
      unit Aast_defs.targ option ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Varray :
      'a ->
      unit Aast_defs.targ option ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Vec :
      'a -> vc_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Vector :
      'a -> vc_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_While :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_XEV_Int :
      'a ->
      int ->
      Ast_defs.xhp_enum_value
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_XEV_String :
      'a ->
      byte_string ->
      Ast_defs.xhp_enum_value
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Xhp_simple :
      'a ->
      (unit, unit) xhp_simple ->
      (unit, unit) xhp_attribute
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Xhp_spread :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit, unit) xhp_attribute
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Xml :
      'a ->
      sid ->
      (unit, unit) xhp_attribute list ->
      (unit, unit) Aast_defs.expr list ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Xor :
      'a ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Yield :
      'a ->
      (unit, unit) afield ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_Yield_break :
      'a ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_abstraction :
      'a ->
      Ast_defs.abstraction ->
      Ast_defs.abstraction
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_afield :
      'a ->
      (unit, unit) afield ->
      (unit, unit) afield
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_as_expr :
      'a ->
      (unit, unit) as_expr ->
      (unit, unit) as_expr
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_block :
      'a ->
      (unit, unit) Aast_defs.block ->
      (unit, unit) Aast_defs.block
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_bop :
      'a ->
      Ast_defs.bop ->
      Ast_defs.bop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_byte_string :
      'a ->
      byte_string ->
      byte_string * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_case :
      'a ->
      (unit, unit) Aast_defs.case ->
      ((unit, unit) Aast_defs.expr * (unit, unit) Aast_defs.block)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_catch :
      'a ->
      (unit, unit) Aast_defs.catch ->
      (sid * Aast_defs.lid * (unit, unit) Aast_defs.block)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_ :
      'a ->
      (unit, unit) class_ ->
      (unit, unit) class_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_abstract_typeconst :
      'a ->
      class_abstract_typeconst ->
      class_abstract_typeconst
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_consts :
      'a ->
      (unit, unit) class_const list ->
      (unit, unit) class_const list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_enum :
      'a ->
      enum_ option ->
      enum_ option * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_extends :
      'a ->
      Aast_defs.hint list ->
      xhp_attr_hint list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_file_attributes :
      'a ->
      (unit, unit) file_attribute list ->
      (unit, unit) file_attribute list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_implements :
      'a ->
      Aast_defs.hint list ->
      xhp_attr_hint list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_methods :
      'a ->
      (unit, unit) method_ list ->
      (unit, unit) method_ list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_req :
      'a ->
      Aast_defs.hint * require_kind ->
      (Aast_defs.hint * require_kind)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_reqs :
      'a ->
      (Aast_defs.hint * require_kind) list ->
      (xhp_attr_hint * require_kind) list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_tparams :
      'a ->
      (unit, unit) tparam list ->
      (unit, unit) tparam list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_typeconsts :
      'a ->
      (unit, unit) class_typeconst_def list ->
      (unit, unit) class_typeconst_def list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_user_attributes :
      'a ->
      (unit, unit) user_attribute list ->
      (unit, unit) user_attribute list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_uses :
      'a ->
      Aast_defs.hint list ->
      xhp_attr_hint list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_vars :
      'a ->
      (unit, unit) class_var list ->
      (unit, unit) class_var list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_where_constraints :
      'a ->
      Aast_defs.where_constraint_hint list ->
      where_constraint_hint list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_xhp_attr_uses :
      'a ->
      Aast_defs.hint list ->
      xhp_attr_hint list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_c_xhp_attrs :
      'a ->
      (unit, unit) Aast_defs.xhp_attr list ->
      (unit, unit) xhp_attr list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_concrete_typeconst :
      'a ->
      class_concrete_typeconst ->
      class_concrete_typeconst
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_const :
      'a ->
      (unit, unit) class_const ->
      (unit, unit) class_const
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_const_kind :
      'a ->
      (unit, unit) class_const_kind ->
      (unit, unit) class_const_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_get_expr :
      'a ->
      (unit, unit) class_get_expr ->
      (unit, unit) class_get_expr
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_hint :
      'a ->
      Aast_defs.hint ->
      Aast_defs.hint * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_id :
      'a ->
      (unit, unit) Aast_defs.class_id ->
      (unit * pos * (unit, unit) class_id_)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_id_ :
      'a ->
      (unit, unit) class_id_ ->
      (unit, unit) class_id_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_name :
      'a -> sid -> sid * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_typeconst :
      'a ->
      class_typeconst ->
      class_typeconst * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_typeconst_def :
      'a ->
      (unit, unit) class_typeconst_def ->
      (unit, unit) class_typeconst_def
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_var :
      'a ->
      (unit, unit) class_var ->
      (unit, unit) class_var
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_var_cv_expr :
      'a ->
      (unit, unit) Aast_defs.expr option ->
      (unit, unit) expr option
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_var_cv_type :
      'a ->
      unit type_hint ->
      unit type_hint * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_class_var_cv_user_attributes :
      'a ->
      (unit, unit) user_attribute list ->
      (unit, unit) user_attribute list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_classish_kind :
      'a ->
      Ast_defs.classish_kind ->
      Ast_defs.classish_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_collection_targ :
      'a ->
      unit collection_targ ->
      unit collection_targ
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_constraint_kind :
      'a ->
      Ast_defs.constraint_kind ->
      Ast_defs.constraint_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_context :
      'a ->
      Aast_defs.hint ->
      Aast_defs.hint * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_contexts :
      'a ->
      Aast_defs.contexts ->
      (pos * Aast_defs.hint list)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ctx_refinement :
      'a ->
      ctx_refinement ->
      ctx_refinement * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ctx_refinement_bounds :
      'a ->
      ctx_refinement_bounds ->
      ctx_refinement_bounds
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_def :
      'a ->
      (unit, unit) def ->
      (unit, unit) def * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_default_case :
      'a ->
      (unit, unit) Aast_defs.default_case ->
      (pos * (unit, unit) Aast_defs.block)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_doc_comment :
      'a ->
      pstring ->
      pstring * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_emit_id :
      'a ->
      emit_id ->
      emit_id * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_enum_ :
      'a ->
      enum_ ->
      enum_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_env_annot :
      'a ->
      env_annot ->
      env_annot * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_expr :
      'a ->
      (unit, unit) Aast_defs.expr ->
      (unit * pos * (unit, unit) expr_)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_expr_ :
      'a ->
      (unit, unit) expr_ ->
      (unit, unit) expr_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_expression_tree :
      'a ->
      (unit, unit) expression_tree ->
      (unit, unit) expression_tree
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_field :
      'a ->
      (unit, unit) Aast_defs.field ->
      ((unit, unit) Aast_defs.expr * (unit, unit) Aast_defs.expr)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_file_attribute :
      'a ->
      (unit, unit) file_attribute ->
      (unit, unit) file_attribute
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_file_attributes :
      'a ->
      (unit, unit) file_attribute list ->
      (unit, unit) file_attribute list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_fun_ :
      'a ->
      (unit, unit) fun_ ->
      (unit, unit) fun_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_fun_def :
      'a ->
      (unit, unit) fun_def ->
      (unit, unit) fun_def
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_fun_f_body :
      'a ->
      (unit, unit) func_body ->
      (unit, unit) func_body
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_fun_f_ctxs :
      'a ->
      Aast_defs.contexts option ->
      contexts option * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_fun_f_params :
      'a ->
      (unit, unit) fun_param list ->
      (unit, unit) fun_param list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_fun_f_ret :
      'a ->
      unit type_hint ->
      unit type_hint * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_fun_f_tparams :
      'a ->
      (unit, unit) tparam list ->
      (unit, unit) tparam list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_fun_f_unsafe_ctxs :
      'a ->
      Aast_defs.contexts option ->
      contexts option * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_fun_f_user_attributes :
      'a ->
      (unit, unit) user_attribute list ->
      (unit, unit) user_attribute list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_fun_f_where_constraints :
      'a ->
      Aast_defs.where_constraint_hint list ->
      where_constraint_hint list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_fun_kind :
      'a ->
      Ast_defs.fun_kind ->
      Ast_defs.fun_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_fun_param :
      'a ->
      (unit, unit) fun_param ->
      (unit, unit) fun_param
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_func_body :
      'a ->
      (unit, unit) func_body ->
      (unit, unit) func_body
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_function_ptr_id :
      'a ->
      (unit, unit) function_ptr_id ->
      (unit, unit) function_ptr_id
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_gconst :
      'a ->
      (unit, unit) gconst ->
      (unit, unit) gconst
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_gconst_cst_type :
      'a ->
      Aast_defs.hint option ->
      xhp_attr_hint option
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_gconst_cst_value :
      'a ->
      (unit, unit) expr ->
      (unit, unit) expr
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_gen_case :
      'a ->
      (unit, unit) gen_case ->
      (unit, unit) gen_case
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_hf_param_info :
      'a ->
      hf_param_info ->
      hf_param_info * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_hint :
      'a ->
      Aast_defs.hint ->
      (pos * hint_) * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_hint_ :
      'a ->
      hint_ ->
      hint_ * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_hint_fun :
      'a ->
      hint_fun ->
      hint_fun * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_hint_fun_hf_ctxs :
      'a ->
      Aast_defs.contexts option ->
      contexts option * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_hint_fun_hf_param_tys :
      'a ->
      Aast_defs.hint list ->
      xhp_attr_hint list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_hint_fun_hf_return_ty :
      'a ->
      xhp_attr_hint ->
      xhp_attr_hint * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_hint_fun_hf_variadic_ty :
      'a ->
      variadic_hint ->
      variadic_hint * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_hole_source :
      'a ->
      hole_source ->
      hole_source * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_id :
      'a ->
      sid ->
      (pos * byte_string)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_id_ :
      'a ->
      byte_string ->
      byte_string * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_import_flavor :
      'a ->
      import_flavor ->
      import_flavor * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_is_variadic :
      'a ->
      is_variadic ->
      is_variadic * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_kvc_kind :
      'a ->
      kvc_kind ->
      kvc_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_lid :
      'a ->
      Aast_defs.lid ->
      (pos * local_id) * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_local_id :
      'a ->
      local_id ->
      local_id * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_md_name_kind :
      'a ->
      md_name_kind ->
      md_name_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_method_ :
      'a ->
      (unit, unit) method_ ->
      (unit, unit) method_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_method_m_body :
      'a ->
      (unit, unit) func_body ->
      (unit, unit) func_body
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_method_m_ctxs :
      'a ->
      Aast_defs.contexts option ->
      contexts option * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_method_m_params :
      'a ->
      (unit, unit) fun_param list ->
      (unit, unit) fun_param list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_method_m_ret :
      'a ->
      unit type_hint ->
      unit type_hint * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_method_m_tparams :
      'a ->
      (unit, unit) tparam list ->
      (unit, unit) tparam list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_method_m_unsafe_ctxs :
      'a ->
      Aast_defs.contexts option ->
      contexts option * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_method_m_user_attributes :
      'a ->
      (unit, unit) user_attribute list ->
      (unit, unit) user_attribute list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_method_m_where_constraints :
      'a ->
      Aast_defs.where_constraint_hint list ->
      where_constraint_hint list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_module_def :
      'a ->
      (unit, unit) module_def ->
      (unit, unit) module_def
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_nast_shape_info :
      'a ->
      nast_shape_info ->
      nast_shape_info * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_ns_kind :
      'a ->
      ns_kind ->
      ns_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_nsenv :
      'a ->
      nsenv ->
      nsenv * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_og_null_flavor :
      'a ->
      og_null_flavor ->
      og_null_flavor * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_param_kind :
      'a ->
      Ast_defs.param_kind ->
      Ast_defs.param_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_pos :
      'a -> pos -> pos * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_positioned_byte_string :
      'a ->
      positioned_byte_string ->
      (pos * byte_string)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_program :
      'a ->
      (unit, unit) def list ->
      (unit, unit) def list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_prop_or_method :
      'a ->
      prop_or_method ->
      prop_or_method * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_pstring :
      'a ->
      pstring ->
      (pos * byte_string)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_readonly_kind :
      'a ->
      Ast_defs.readonly_kind ->
      Ast_defs.readonly_kind
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_refinement :
      'a ->
      refinement ->
      refinement * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_reified :
      'a ->
      is_variadic ->
      is_variadic * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_reify_kind :
      'a ->
      reify_kind ->
      reify_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_require_kind :
      'a ->
      require_kind ->
      require_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_shape_field_info :
      'a ->
      shape_field_info ->
      shape_field_info * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_shape_field_name :
      'a ->
      Ast_defs.shape_field_name ->
      Ast_defs.shape_field_name
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_sid :
      'a -> sid -> sid * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_stmt :
      'a ->
      (unit, unit) Aast_defs.stmt ->
      (pos * (unit, unit) stmt_)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_stmt_ :
      'a ->
      (unit, unit) stmt_ ->
      (unit, unit) stmt_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_targ :
      'a ->
      unit Aast_defs.targ ->
      (unit * Aast_defs.hint)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_tparam :
      'a ->
      (unit, unit) tparam ->
      (unit, unit) tparam
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_tprim :
      'a ->
      tprim ->
      tprim * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_trait_hint :
      'a ->
      Aast_defs.hint ->
      Aast_defs.hint * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_type_hint :
      'a ->
      unit Aast_defs.type_hint ->
      (unit * Aast_defs.type_hint_)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_type_hint_ :
      'a ->
      Aast_defs.type_hint_ ->
      Aast_defs.type_hint_
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_type_refinement :
      'a ->
      type_refinement ->
      type_refinement * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_type_refinement_bounds :
      'a ->
      type_refinement_bounds ->
      type_refinement_bounds
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_typedef :
      'a ->
      (unit, unit) typedef ->
      (unit, unit) typedef
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_typedef_visibility :
      'a ->
      typedef_visibility ->
      typedef_visibility
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_uop :
      'a ->
      Ast_defs.uop ->
      Ast_defs.uop * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_user_attribute :
      'a ->
      (unit, unit) user_attribute ->
      (unit, unit) user_attribute
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_user_attributes :
      'a ->
      (unit, unit) user_attribute list ->
      (unit, unit) user_attribute list
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_using_stmt :
      'a ->
      (unit, unit) using_stmt ->
      (unit, unit) using_stmt
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_variadic_hint :
      'a ->
      Aast_defs.variadic_hint ->
      Aast_defs.variadic_hint
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_variance :
      'a ->
      Ast_defs.variance ->
      Ast_defs.variance
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_vc_kind :
      'a ->
      vc_kind ->
      vc_kind * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_visibility :
      'a ->
      visibility ->
      visibility * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_visibility_Internal :
      'a -> visibility * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_visibility_Private :
      'a -> visibility * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_visibility_Protected :
      'a -> visibility * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_visibility_Public :
      'a -> visibility * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_where_constraint_hint :
      'a ->
      Aast_defs.where_constraint_hint ->
      (Aast_defs.hint * Ast_defs.constraint_kind * Aast_defs.hint)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_xhp_attr :
      'a ->
      (unit, unit) Aast_defs.xhp_attr ->
      (unit Aast_defs.type_hint
      * (unit, unit) class_var
      * xhp_attr_tag option
      * (pos * (unit, unit) Aast_defs.expr list) option)
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_xhp_attr_hint :
      'a ->
      Aast_defs.hint ->
      Aast_defs.hint * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_xhp_attr_info :
      'a ->
      xhp_attr_info ->
      xhp_attr_info * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_xhp_attr_tag :
      'a ->
      xhp_attr_tag ->
      xhp_attr_tag * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_xhp_attribute :
      'a ->
      (unit, unit) xhp_attribute ->
      (unit, unit) xhp_attribute
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_xhp_child :
      'a ->
      xhp_child ->
      xhp_child * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_xhp_child_op :
      'a ->
      xhp_child_op ->
      xhp_child_op * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_xhp_enum_value :
      'a ->
      Ast_defs.xhp_enum_value ->
      Ast_defs.xhp_enum_value
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t
  ; on_xhp_simple :
      'a ->
      (unit, unit) xhp_simple ->
      (unit, unit) xhp_simple
      * Naming_phase_error.err Naming_phase_error.Free_monoid.t >

(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast

type 'env t

val mk_visitor :
  'env t list ->
  ('env -> (unit, unit) program -> (unit, unit) program)
  * ('env -> (unit, unit) class_ -> (unit, unit) class_)
  * ('env -> (unit, unit) fun_def -> (unit, unit) fun_def)
  * ('env -> (unit, unit) module_def -> (unit, unit) module_def)
  * ('env -> (unit, unit) gconst -> (unit, unit) gconst)
  * ('env -> (unit, unit) typedef -> (unit, unit) typedef)

module Ast_transform : sig
  type 'a transform = 'a -> ('a, 'a) Result.t

  type 'env t = {
    on_program: ('env * (unit, unit) program) transform option;
    (* -- Class and fields -------------------------------------------------- *)
    on_class_: ('env * (unit, unit) class_) transform option;
    on_class_c_tparams: ('env * (unit, unit) tparam list) transform option;
    on_class_c_extends: ('env * hint list) transform option;
    on_class_c_uses: ('env * hint list) transform option;
    on_class_c_xhp_attrs: ('env * (unit, unit) xhp_attr list) transform option;
    on_class_c_xhp_attr_uses: ('env * hint list) transform option;
    on_class_c_req: ('env * (hint * require_kind)) transform option;
    on_class_c_reqs: ('env * (hint * require_kind) list) transform option;
    on_class_c_implements: ('env * hint list) transform option;
    on_class_c_where_constraints:
      ('env * where_constraint_hint list) transform option;
    on_class_c_consts: ('env * (unit, unit) class_const list) transform option;
    on_class_c_typeconsts:
      ('env * (unit, unit) class_typeconst_def list) transform option;
    on_class_c_vars: ('env * (unit, unit) class_var list) transform option;
    on_class_c_enum: ('env * enum_ option) transform option;
    on_class_c_methods: ('env * (unit, unit) method_ list) transform option;
    on_class_c_user_attributes:
      ('env * (unit, unit) user_attribute list) transform option;
    on_class_c_file_attributes:
      ('env * (unit, unit) file_attribute list) transform option;
    (* -- Class vars -------------------------------------------------------- *)
    on_class_var: ('env * (unit, unit) class_var) transform option;
    on_class_var_cv_user_attributes:
      ('env * (unit, unit) user_attribute list) transform option;
    on_class_var_cv_expr: ('env * (unit, unit) expr option) transform option;
    on_class_var_cv_type: ('env * unit type_hint) transform option;
    on_class_const_kind:
      ('env * (unit, unit) class_const_kind) transform option;
    (* -- Type defs --------------------------------------------------------- *)
    on_typedef: ('env * (unit, unit) typedef) transform option;
    (* -- Global constants -------------------------------------------------- *)
    on_gconst: ('env * (unit, unit) gconst) transform option;
    on_gconst_cst_type: ('env * hint option) transform option;
    on_gconst_cst_value: ('env * (unit, unit) expr) transform option;
    (* -- Function defs ----------------------------------------------------- *)
    on_fun_def: ('env * (unit, unit) fun_def) transform option;
    (* -- Module defs ------------------------------------------------------- *)
    on_module_def: ('env * (unit, unit) module_def) transform option;
    (* -- Statements -------------------------------------------------------- *)
    on_stmt: ('env * (unit, unit) stmt) transform option;
    on_stmt_: ('env * (unit, unit) stmt_) transform option;
    on_block: ('env * (unit, unit) block) transform option;
    on_using_stmt: ('env * (unit, unit) using_stmt) transform option;
    (* -- Hints ------------------------------------------------------------- *)
    on_hint: ('env * hint) transform option;
    on_hint_: ('env * hint_) transform option;
    (* -- hint_fun & fields ------------------------------------------------- *)
    on_hint_fun: ('env * hint_fun) transform option;
    on_hint_fun_hf_param_tys: ('env * hint list) transform option;
    on_hint_fun_hf_variadic_ty: ('env * variadic_hint) transform option;
    on_hint_fun_hf_ctxs: ('env * contexts option) transform option;
    on_hint_fun_hf_return_ty: ('env * hint) transform option;
    on_refinement: ('env * refinement) transform option;
    on_type_refinement: ('env * type_refinement) transform option;
    on_type_refinement_bounds: ('env * type_refinement_bounds) transform option;
    on_ctx_refinement: ('env * ctx_refinement) transform option;
    on_ctx_refinement_bounds: ('env * ctx_refinement_bounds) transform option;
    (* -- shape fields ------------------------------------------------------- *)
    on_nast_shape_info: ('env * nast_shape_info) transform option;
    on_shape_field_info: ('env * shape_field_info) transform option;
    on_nsi_field_map: ('env * shape_field_info list) transform option;
    on_shape_field_name: ('env * Ast_defs.shape_field_name) transform option;
    (* -- Expressions ------------------------------------------------------- *)
    on_expr: ('env * (unit, unit) expr) transform option;
    on_expr_: ('env * (unit, unit) expr_) transform option;
    (* -- Functions --------------------------------------------------------- *)
    on_fun_: ('env * (unit, unit) fun_) transform option;
    on_fun_f_ret: ('env * unit type_hint) transform option;
    on_fun_f_tparams: ('env * (unit, unit) tparam list) transform option;
    on_fun_f_where_constraints:
      ('env * where_constraint_hint list) transform option;
    on_fun_f_params: ('env * (unit, unit) fun_param list) transform option;
    on_fun_f_ctxs: ('env * contexts option) transform option;
    on_fun_f_unsafe_ctxs: ('env * contexts option) transform option;
    on_fun_f_body: ('env * (unit, unit) func_body) transform option;
    on_fun_f_user_attributes:
      ('env * (unit, unit) user_attribute list) transform option;
    (* -- Methods ----------------------------------------------------------- *)
    on_method_: ('env * (unit, unit) method_) transform option;
    on_method_m_ret: ('env * unit type_hint) transform option;
    on_method_m_tparams: ('env * (unit, unit) tparam list) transform option;
    on_method_m_where_constraints:
      ('env * where_constraint_hint list) transform option;
    on_method_m_params: ('env * (unit, unit) fun_param list) transform option;
    on_method_m_ctxs: ('env * contexts option) transform option;
    on_method_m_unsafe_ctxs: ('env * contexts option) transform option;
    on_method_m_body: ('env * (unit, unit) func_body) transform option;
    on_method_m_user_attributes:
      ('env * (unit, unit) user_attribute list) transform option;
    (* -- Class ID ---------------------------------------------------------- *)
    on_class_id: ('env * (unit, unit) class_id) transform option;
    on_class_id_: ('env * (unit, unit) class_id_) transform option;
    (* -- Common ------------------------------------------------------------ *)
    on_func_body: ('env * (unit, unit) func_body) transform option;
    on_enum_: ('env * enum_) transform option;
    on_tparam: ('env * (unit, unit) tparam) transform option;
    on_user_attributes:
      ('env * (unit, unit) user_attribute list) transform option;
    on_user_attribute: ('env * (unit, unit) user_attribute) transform option;
    on_file_attributes:
      ('env * (unit, unit) file_attribute list) transform option;
    on_file_attribute: ('env * (unit, unit) file_attribute) transform option;
    on_where_constraint_hint: ('env * where_constraint_hint) transform option;
    on_where_constraint_hints:
      ('env * where_constraint_hint list) transform option;
    on_contexts: ('env * contexts) transform option;
    on_context: ('env * hint) transform option;
    on_targ: ('env * unit targ) transform option;
    on_as_expr: ('env * (unit, unit) as_expr) transform option;
    on_afield: ('env * (unit, unit) afield) transform option;
    on_collection_targ: ('env * unit collection_targ) transform option;
    on_class_get_expr: ('env * (unit, unit) class_get_expr) transform option;
    on_xhp_attribute: ('env * (unit, unit) xhp_attribute) transform option;
    on_xhp_simple: ('env * (unit, unit) xhp_simple) transform option;
    on_function_ptr_id: ('env * (unit, unit) function_ptr_id) transform option;
    on_tparams: ('env * (unit, unit) tparam list) transform option;
    on_tparam_tparams: ('env * (unit, unit) tparam list) transform option;
    on_tparam_tp_constraints:
      ('env * (Ast_defs.constraint_kind * hint) list) transform option;
    on_tp_constraint:
      ('env * (Ast_defs.constraint_kind * hint)) transform option;
    on_tparam_tp_user_attributes:
      ('env * (unit, unit) user_attribute list) transform option;
    on_catch: ('env * (unit, unit) catch) transform option;
    on_case: ('env * (unit, unit) case) transform option;
    on_default_case: ('env * (unit, unit) default_case) transform option;
    on_fun_param_user_attributes:
      ('env * (unit, unit) user_attribute list) transform option;
    on_fun_param: ('env * (unit, unit) fun_param) transform option;
    on_fun_params: ('env * (unit, unit) fun_param list) transform option;
    on_xhp_attr: ('env * (unit, unit) xhp_attr) transform option;
    on_class_const: ('env * (unit, unit) class_const) transform option;
    on_class_typeconst_def:
      ('env * (unit, unit) class_typeconst_def) transform option;
    on_class_concrete_typeconst:
      ('env * class_concrete_typeconst) transform option;
    on_class_abstract_typeconst:
      ('env * class_abstract_typeconst) transform option;
    on_class_typeconst: ('env * class_typeconst) transform option;
    on_def: ('env * (unit, unit) def) transform option;
    on_expression_tree: ('env * (unit, unit) expression_tree) transform option;
  }

  val identity : 'env t
end

val top_down : 'env Ast_transform.t -> 'env t

val bottom_up : 'env Ast_transform.t -> 'env t

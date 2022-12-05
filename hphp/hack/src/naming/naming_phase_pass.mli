(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast

type ('env, 'err) t

type 'a transform = 'a -> ('a, 'a) Result.t

type ('env, 'err) pass = {
  on_program: ('env * (unit, unit) program * 'err) transform option;
  (* -- Class and fields -------------------------------------------------- *)
  on_class_: ('env * (unit, unit) class_ * 'err) transform option;
  on_class_c_tparams: ('env * (unit, unit) tparam list * 'err) transform option;
  on_class_c_extends: ('env * hint list * 'err) transform option;
  on_class_c_uses: ('env * hint list * 'err) transform option;
  on_class_c_xhp_attrs:
    ('env * (unit, unit) xhp_attr list * 'err) transform option;
  on_class_c_xhp_attr_uses: ('env * hint list * 'err) transform option;
  on_class_c_req: ('env * (hint * require_kind) * 'err) transform option;
  on_class_c_reqs: ('env * (hint * require_kind) list * 'err) transform option;
  on_class_c_implements: ('env * hint list * 'err) transform option;
  on_class_c_where_constraints:
    ('env * where_constraint_hint list * 'err) transform option;
  on_class_c_consts:
    ('env * (unit, unit) class_const list * 'err) transform option;
  on_class_c_typeconsts:
    ('env * (unit, unit) class_typeconst_def list * 'err) transform option;
  on_class_c_vars: ('env * (unit, unit) class_var list * 'err) transform option;
  on_class_c_enum: ('env * enum_ option * 'err) transform option;
  on_class_c_methods:
    ('env * (unit, unit) method_ list * 'err) transform option;
  on_class_c_user_attributes:
    ('env * (unit, unit) user_attribute list * 'err) transform option;
  on_class_c_file_attributes:
    ('env * (unit, unit) file_attribute list * 'err) transform option;
  (* -- Class vars -------------------------------------------------------- *)
  on_class_var: ('env * (unit, unit) class_var * 'err) transform option;
  on_class_var_cv_user_attributes:
    ('env * (unit, unit) user_attribute list * 'err) transform option;
  on_class_var_cv_expr:
    ('env * (unit, unit) expr option * 'err) transform option;
  on_class_var_cv_type: ('env * unit type_hint * 'err) transform option;
  on_class_const_kind:
    ('env * (unit, unit) class_const_kind * 'err) transform option;
  (* -- Type defs --------------------------------------------------------- *)
  on_typedef: ('env * (unit, unit) typedef * 'err) transform option;
  (* -- Global constants -------------------------------------------------- *)
  on_gconst: ('env * (unit, unit) gconst * 'err) transform option;
  on_gconst_cst_type: ('env * hint option * 'err) transform option;
  on_gconst_cst_value: ('env * (unit, unit) expr * 'err) transform option;
  (* -- Function defs ----------------------------------------------------- *)
  on_fun_def: ('env * (unit, unit) fun_def * 'err) transform option;
  (* -- Module defs ------------------------------------------------------- *)
  on_module_def: ('env * (unit, unit) module_def * 'err) transform option;
  (* -- Statements -------------------------------------------------------- *)
  on_stmt: ('env * (unit, unit) stmt * 'err) transform option;
  on_stmt_: ('env * (unit, unit) stmt_ * 'err) transform option;
  on_block: ('env * (unit, unit) block * 'err) transform option;
  on_using_stmt: ('env * (unit, unit) using_stmt * 'err) transform option;
  (* -- Hints ------------------------------------------------------------- *)
  on_hint: ('env * hint * 'err) transform option;
  on_hint_: ('env * hint_ * 'err) transform option;
  (* -- hint_fun & fields ------------------------------------------------- *)
  on_hint_fun: ('env * hint_fun * 'err) transform option;
  on_hint_fun_hf_param_tys: ('env * hint list * 'err) transform option;
  on_hint_fun_hf_variadic_ty: ('env * variadic_hint * 'err) transform option;
  on_hint_fun_hf_ctxs: ('env * contexts option * 'err) transform option;
  on_hint_fun_hf_return_ty: ('env * hint * 'err) transform option;
  on_refinement: ('env * refinement * 'err) transform option;
  on_type_refinement: ('env * type_refinement * 'err) transform option;
  on_type_refinement_bounds:
    ('env * type_refinement_bounds * 'err) transform option;
  on_ctx_refinement: ('env * ctx_refinement * 'err) transform option;
  on_ctx_refinement_bounds:
    ('env * ctx_refinement_bounds * 'err) transform option;
  (* -- shape fields ------------------------------------------------------- *)
  on_nast_shape_info: ('env * nast_shape_info * 'err) transform option;
  on_shape_field_info: ('env * shape_field_info * 'err) transform option;
  on_nsi_field_map: ('env * shape_field_info list * 'err) transform option;
  on_shape_field_name:
    ('env * Ast_defs.shape_field_name * 'err) transform option;
  (* -- Expressions ------------------------------------------------------- *)
  on_expr: ('env * (unit, unit) expr * 'err) transform option;
  on_expr_: ('env * (unit, unit) expr_ * 'err) transform option;
  (* -- Functions --------------------------------------------------------- *)
  on_fun_: ('env * (unit, unit) fun_ * 'err) transform option;
  on_fun_f_ret: ('env * unit type_hint * 'err) transform option;
  on_fun_f_tparams: ('env * (unit, unit) tparam list * 'err) transform option;
  on_fun_f_where_constraints:
    ('env * where_constraint_hint list * 'err) transform option;
  on_fun_f_params: ('env * (unit, unit) fun_param list * 'err) transform option;
  on_fun_f_ctxs: ('env * contexts option * 'err) transform option;
  on_fun_f_unsafe_ctxs: ('env * contexts option * 'err) transform option;
  on_fun_f_body: ('env * (unit, unit) func_body * 'err) transform option;
  on_fun_f_user_attributes:
    ('env * (unit, unit) user_attribute list * 'err) transform option;
  (* -- Methods ----------------------------------------------------------- *)
  on_method_: ('env * (unit, unit) method_ * 'err) transform option;
  on_method_m_ret: ('env * unit type_hint * 'err) transform option;
  on_method_m_tparams:
    ('env * (unit, unit) tparam list * 'err) transform option;
  on_method_m_where_constraints:
    ('env * where_constraint_hint list * 'err) transform option;
  on_method_m_params:
    ('env * (unit, unit) fun_param list * 'err) transform option;
  on_method_m_ctxs: ('env * contexts option * 'err) transform option;
  on_method_m_unsafe_ctxs: ('env * contexts option * 'err) transform option;
  on_method_m_body: ('env * (unit, unit) func_body * 'err) transform option;
  on_method_m_user_attributes:
    ('env * (unit, unit) user_attribute list * 'err) transform option;
  (* -- Class ID ---------------------------------------------------------- *)
  on_class_id: ('env * (unit, unit) class_id * 'err) transform option;
  on_class_id_: ('env * (unit, unit) class_id_ * 'err) transform option;
  (* -- Common ------------------------------------------------------------ *)
  on_func_body: ('env * (unit, unit) func_body * 'err) transform option;
  on_enum_: ('env * enum_ * 'err) transform option;
  on_tparam: ('env * (unit, unit) tparam * 'err) transform option;
  on_user_attributes:
    ('env * (unit, unit) user_attribute list * 'err) transform option;
  on_user_attribute:
    ('env * (unit, unit) user_attribute * 'err) transform option;
  on_file_attributes:
    ('env * (unit, unit) file_attribute list * 'err) transform option;
  on_file_attribute:
    ('env * (unit, unit) file_attribute * 'err) transform option;
  on_where_constraint_hint:
    ('env * where_constraint_hint * 'err) transform option;
  on_where_constraint_hints:
    ('env * where_constraint_hint list * 'err) transform option;
  on_contexts: ('env * contexts * 'err) transform option;
  on_context: ('env * hint * 'err) transform option;
  on_targ: ('env * unit targ * 'err) transform option;
  on_as_expr: ('env * (unit, unit) as_expr * 'err) transform option;
  on_afield: ('env * (unit, unit) afield * 'err) transform option;
  on_collection_targ: ('env * unit collection_targ * 'err) transform option;
  on_class_get_expr:
    ('env * (unit, unit) class_get_expr * 'err) transform option;
  on_xhp_attribute: ('env * (unit, unit) xhp_attribute * 'err) transform option;
  on_xhp_simple: ('env * (unit, unit) xhp_simple * 'err) transform option;
  on_function_ptr_id:
    ('env * (unit, unit) function_ptr_id * 'err) transform option;
  on_tparams: ('env * (unit, unit) tparam list * 'err) transform option;
  on_tparam_tparams: ('env * (unit, unit) tparam list * 'err) transform option;
  on_tparam_tp_constraints:
    ('env * (Ast_defs.constraint_kind * hint) list * 'err) transform option;
  on_tp_constraint:
    ('env * (Ast_defs.constraint_kind * hint) * 'err) transform option;
  on_tparam_tp_user_attributes:
    ('env * (unit, unit) user_attribute list * 'err) transform option;
  on_catch: ('env * (unit, unit) catch * 'err) transform option;
  on_case: ('env * (unit, unit) case * 'err) transform option;
  on_default_case: ('env * (unit, unit) default_case * 'err) transform option;
  on_fun_param_user_attributes:
    ('env * (unit, unit) user_attribute list * 'err) transform option;
  on_fun_param: ('env * (unit, unit) fun_param * 'err) transform option;
  on_fun_params: ('env * (unit, unit) fun_param list * 'err) transform option;
  on_xhp_attr: ('env * (unit, unit) xhp_attr * 'err) transform option;
  on_class_const: ('env * (unit, unit) class_const * 'err) transform option;
  on_class_typeconst_def:
    ('env * (unit, unit) class_typeconst_def * 'err) transform option;
  on_class_concrete_typeconst:
    ('env * class_concrete_typeconst * 'err) transform option;
  on_class_abstract_typeconst:
    ('env * class_abstract_typeconst * 'err) transform option;
  on_class_typeconst: ('env * class_typeconst * 'err) transform option;
  on_def: ('env * (unit, unit) def * 'err) transform option;
  on_expression_tree:
    ('env * (unit, unit) expression_tree * 'err) transform option;
}

val identity : ('env, 'err) pass

val top_down : ('env, 'err) pass -> ('env, 'err) t

val bottom_up : ('env, 'err) pass -> ('env, 'err) t

val mk_visitor :
  (Naming_phase_env.t, Naming_phase_error.t list) t list ->
  on_error:(Naming_phase_error.t -> unit) ->
  (Naming_phase_env.t -> (unit, unit) program -> (unit, unit) program)
  * (Naming_phase_env.t -> (unit, unit) class_ -> (unit, unit) class_)
  * (Naming_phase_env.t -> (unit, unit) fun_def -> (unit, unit) fun_def)
  * (Naming_phase_env.t -> (unit, unit) module_def -> (unit, unit) module_def)
  * (Naming_phase_env.t -> (unit, unit) gconst -> (unit, unit) gconst)
  * (Naming_phase_env.t -> (unit, unit) typedef -> (unit, unit) typedef)

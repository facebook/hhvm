(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Common
open Aast

module Ast_transform = struct
  (* Each of our transforms can return either `Ok`, indicating that we
     should continue to apply subsequent transforms, or `Error` indicating
     that we should short-circuit the pass and stop. This is useful when
     we introduce an error marker like `Herr` or `invalid_expr_` where
     we know all child elements are already canonical and valid *)
  type 'a transform = 'a -> ('a, 'a) Result.t

  type ('env, 'err) t = {
    on_program: ('env * (unit, unit) program * 'err) transform option;
    (* -- Class and fields -------------------------------------------------- *)
    on_class_: ('env * (unit, unit) class_ * 'err) transform option;
    on_class_c_tparams:
      ('env * (unit, unit) tparam list * 'err) transform option;
    on_class_c_extends: ('env * hint list * 'err) transform option;
    on_class_c_uses: ('env * hint list * 'err) transform option;
    on_class_c_xhp_attrs:
      ('env * (unit, unit) xhp_attr list * 'err) transform option;
    on_class_c_xhp_attr_uses: ('env * hint list * 'err) transform option;
    on_class_c_req: ('env * (hint * require_kind) * 'err) transform option;
    on_class_c_reqs:
      ('env * (hint * require_kind) list * 'err) transform option;
    on_class_c_implements: ('env * hint list * 'err) transform option;
    on_class_c_where_constraints:
      ('env * where_constraint_hint list * 'err) transform option;
    on_class_c_consts:
      ('env * (unit, unit) class_const list * 'err) transform option;
    on_class_c_typeconsts:
      ('env * (unit, unit) class_typeconst_def list * 'err) transform option;
    on_class_c_vars:
      ('env * (unit, unit) class_var list * 'err) transform option;
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
    on_fun_f_params:
      ('env * (unit, unit) fun_param list * 'err) transform option;
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
    on_xhp_attribute:
      ('env * (unit, unit) xhp_attribute * 'err) transform option;
    on_xhp_simple: ('env * (unit, unit) xhp_simple * 'err) transform option;
    on_function_ptr_id:
      ('env * (unit, unit) function_ptr_id * 'err) transform option;
    on_tparams: ('env * (unit, unit) tparam list * 'err) transform option;
    on_tparam_tparams:
      ('env * (unit, unit) tparam list * 'err) transform option;
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

  let combine_transform ps ~select =
    match List.filter_map ps ~f:select with
    | [] -> None
    | ts -> Some (fun init -> List.fold_result ts ~init ~f:( |> ))

  let combine ps =
    {
      on_program = combine_transform ps ~select:(fun t -> t.on_program);
      on_class_ = combine_transform ps ~select:(fun t -> t.on_class_);
      on_class_c_tparams =
        combine_transform ps ~select:(fun t -> t.on_class_c_tparams);
      on_class_c_extends =
        combine_transform ps ~select:(fun t -> t.on_class_c_extends);
      on_class_c_uses =
        combine_transform ps ~select:(fun t -> t.on_class_c_uses);
      on_class_c_xhp_attrs =
        combine_transform ps ~select:(fun t -> t.on_class_c_xhp_attrs);
      on_class_c_xhp_attr_uses =
        combine_transform ps ~select:(fun t -> t.on_class_c_xhp_attr_uses);
      on_class_c_req = combine_transform ps ~select:(fun t -> t.on_class_c_req);
      on_class_c_reqs =
        combine_transform ps ~select:(fun t -> t.on_class_c_reqs);
      on_class_c_implements =
        combine_transform ps ~select:(fun t -> t.on_class_c_implements);
      on_class_c_where_constraints =
        combine_transform ps ~select:(fun t -> t.on_class_c_where_constraints);
      on_class_c_consts =
        combine_transform ps ~select:(fun t -> t.on_class_c_consts);
      on_class_c_typeconsts =
        combine_transform ps ~select:(fun t -> t.on_class_c_typeconsts);
      on_class_c_vars =
        combine_transform ps ~select:(fun t -> t.on_class_c_vars);
      on_class_c_enum =
        combine_transform ps ~select:(fun t -> t.on_class_c_enum);
      on_class_c_methods =
        combine_transform ps ~select:(fun t -> t.on_class_c_methods);
      on_class_c_user_attributes =
        combine_transform ps ~select:(fun t -> t.on_class_c_user_attributes);
      on_class_c_file_attributes =
        combine_transform ps ~select:(fun t -> t.on_class_c_file_attributes);
      on_class_const_kind =
        combine_transform ps ~select:(fun t -> t.on_class_const_kind);
      on_class_var = combine_transform ps ~select:(fun t -> t.on_class_var);
      on_class_var_cv_user_attributes =
        combine_transform ps ~select:(fun t ->
            t.on_class_var_cv_user_attributes);
      on_class_var_cv_expr =
        combine_transform ps ~select:(fun t -> t.on_class_var_cv_expr);
      on_class_var_cv_type =
        combine_transform ps ~select:(fun t -> t.on_class_var_cv_type);
      on_typedef = combine_transform ps ~select:(fun t -> t.on_typedef);
      on_gconst = combine_transform ps ~select:(fun t -> t.on_gconst);
      on_gconst_cst_type =
        combine_transform ps ~select:(fun t -> t.on_gconst_cst_type);
      on_gconst_cst_value =
        combine_transform ps ~select:(fun t -> t.on_gconst_cst_value);
      on_fun_def = combine_transform ps ~select:(fun t -> t.on_fun_def);
      on_module_def = combine_transform ps ~select:(fun t -> t.on_module_def);
      on_stmt = combine_transform ps ~select:(fun t -> t.on_stmt);
      on_stmt_ = combine_transform ps ~select:(fun t -> t.on_stmt_);
      on_block = combine_transform ps ~select:(fun t -> t.on_block);
      on_using_stmt = combine_transform ps ~select:(fun t -> t.on_using_stmt);
      on_hint = combine_transform ps ~select:(fun t -> t.on_hint);
      on_hint_ = combine_transform ps ~select:(fun t -> t.on_hint_);
      on_hint_fun = combine_transform ps ~select:(fun t -> t.on_hint_fun);
      on_hint_fun_hf_param_tys =
        combine_transform ps ~select:(fun t -> t.on_hint_fun_hf_param_tys);
      on_hint_fun_hf_variadic_ty =
        combine_transform ps ~select:(fun t -> t.on_hint_fun_hf_variadic_ty);
      on_hint_fun_hf_ctxs =
        combine_transform ps ~select:(fun t -> t.on_hint_fun_hf_ctxs);
      on_hint_fun_hf_return_ty =
        combine_transform ps ~select:(fun t -> t.on_hint_fun_hf_return_ty);
      on_nast_shape_info =
        combine_transform ps ~select:(fun t -> t.on_nast_shape_info);
      on_shape_field_info =
        combine_transform ps ~select:(fun t -> t.on_shape_field_info);
      on_nsi_field_map =
        combine_transform ps ~select:(fun t -> t.on_nsi_field_map);
      on_shape_field_name =
        combine_transform ps ~select:(fun t -> t.on_shape_field_name);
      on_refinement = combine_transform ps ~select:(fun t -> t.on_refinement);
      on_type_refinement =
        combine_transform ps ~select:(fun t -> t.on_type_refinement);
      on_type_refinement_bounds =
        combine_transform ps ~select:(fun t -> t.on_type_refinement_bounds);
      on_ctx_refinement =
        combine_transform ps ~select:(fun t -> t.on_ctx_refinement);
      on_ctx_refinement_bounds =
        combine_transform ps ~select:(fun t -> t.on_ctx_refinement_bounds);
      on_expr = combine_transform ps ~select:(fun t -> t.on_expr);
      on_expr_ = combine_transform ps ~select:(fun t -> t.on_expr_);
      on_fun_ = combine_transform ps ~select:(fun t -> t.on_fun_);
      on_fun_f_ret = combine_transform ps ~select:(fun t -> t.on_fun_f_ret);
      on_fun_f_tparams =
        combine_transform ps ~select:(fun t -> t.on_fun_f_tparams);
      on_fun_f_where_constraints =
        combine_transform ps ~select:(fun t -> t.on_fun_f_where_constraints);
      on_fun_f_params =
        combine_transform ps ~select:(fun t -> t.on_fun_f_params);
      on_fun_f_ctxs = combine_transform ps ~select:(fun t -> t.on_fun_f_ctxs);
      on_fun_f_unsafe_ctxs =
        combine_transform ps ~select:(fun t -> t.on_fun_f_unsafe_ctxs);
      on_fun_f_body = combine_transform ps ~select:(fun t -> t.on_fun_f_body);
      on_fun_f_user_attributes =
        combine_transform ps ~select:(fun t -> t.on_fun_f_user_attributes);
      on_method_ = combine_transform ps ~select:(fun t -> t.on_method_);
      on_method_m_ret =
        combine_transform ps ~select:(fun t -> t.on_method_m_ret);
      on_method_m_tparams =
        combine_transform ps ~select:(fun t -> t.on_method_m_tparams);
      on_method_m_where_constraints =
        combine_transform ps ~select:(fun t -> t.on_method_m_where_constraints);
      on_method_m_params =
        combine_transform ps ~select:(fun t -> t.on_method_m_params);
      on_method_m_ctxs =
        combine_transform ps ~select:(fun t -> t.on_method_m_ctxs);
      on_method_m_unsafe_ctxs =
        combine_transform ps ~select:(fun t -> t.on_method_m_unsafe_ctxs);
      on_method_m_body =
        combine_transform ps ~select:(fun t -> t.on_method_m_body);
      on_method_m_user_attributes =
        combine_transform ps ~select:(fun t -> t.on_method_m_user_attributes);
      on_class_id = combine_transform ps ~select:(fun t -> t.on_class_id);
      on_class_id_ = combine_transform ps ~select:(fun t -> t.on_class_id_);
      on_func_body = combine_transform ps ~select:(fun t -> t.on_func_body);
      on_enum_ = combine_transform ps ~select:(fun t -> t.on_enum_);
      on_tparam = combine_transform ps ~select:(fun t -> t.on_tparam);
      on_user_attributes =
        combine_transform ps ~select:(fun t -> t.on_user_attributes);
      on_user_attribute =
        combine_transform ps ~select:(fun t -> t.on_user_attribute);
      on_file_attributes =
        combine_transform ps ~select:(fun t -> t.on_file_attributes);
      on_file_attribute =
        combine_transform ps ~select:(fun t -> t.on_file_attribute);
      on_where_constraint_hints =
        combine_transform ps ~select:(fun t -> t.on_where_constraint_hints);
      on_where_constraint_hint =
        combine_transform ps ~select:(fun t -> t.on_where_constraint_hint);
      on_contexts = combine_transform ps ~select:(fun t -> t.on_contexts);
      on_context = combine_transform ps ~select:(fun t -> t.on_context);
      on_targ = combine_transform ps ~select:(fun t -> t.on_targ);
      on_as_expr = combine_transform ps ~select:(fun t -> t.on_as_expr);
      on_class_get_expr =
        combine_transform ps ~select:(fun t -> t.on_class_get_expr);
      on_afield = combine_transform ps ~select:(fun t -> t.on_afield);
      on_collection_targ =
        combine_transform ps ~select:(fun t -> t.on_collection_targ);
      on_xhp_attribute =
        combine_transform ps ~select:(fun t -> t.on_xhp_attribute);
      on_xhp_simple = combine_transform ps ~select:(fun t -> t.on_xhp_simple);
      on_function_ptr_id =
        combine_transform ps ~select:(fun t -> t.on_function_ptr_id);
      on_tparams = combine_transform ps ~select:(fun t -> t.on_tparams);
      on_tparam_tparams =
        combine_transform ps ~select:(fun t -> t.on_tparam_tparams);
      on_tparam_tp_constraints =
        combine_transform ps ~select:(fun t -> t.on_tparam_tp_constraints);
      on_tp_constraint =
        combine_transform ps ~select:(fun t -> t.on_tp_constraint);
      on_tparam_tp_user_attributes =
        combine_transform ps ~select:(fun t -> t.on_tparam_tp_user_attributes);
      on_catch = combine_transform ps ~select:(fun t -> t.on_catch);
      on_case = combine_transform ps ~select:(fun t -> t.on_case);
      on_default_case =
        combine_transform ps ~select:(fun t -> t.on_default_case);
      on_fun_param_user_attributes =
        combine_transform ps ~select:(fun t -> t.on_fun_param_user_attributes);
      on_fun_param = combine_transform ps ~select:(fun t -> t.on_fun_param);
      on_fun_params = combine_transform ps ~select:(fun t -> t.on_fun_params);
      on_xhp_attr = combine_transform ps ~select:(fun t -> t.on_xhp_attr);
      on_class_const = combine_transform ps ~select:(fun t -> t.on_class_const);
      on_class_typeconst_def =
        combine_transform ps ~select:(fun t -> t.on_class_typeconst_def);
      on_class_concrete_typeconst =
        combine_transform ps ~select:(fun t -> t.on_class_concrete_typeconst);
      on_class_abstract_typeconst =
        combine_transform ps ~select:(fun t -> t.on_class_abstract_typeconst);
      on_class_typeconst =
        combine_transform ps ~select:(fun t -> t.on_class_typeconst);
      on_def = combine_transform ps ~select:(fun t -> t.on_def);
      on_expression_tree =
        combine_transform ps ~select:(fun t -> t.on_expression_tree);
    }

  let identity =
    {
      on_program = None;
      on_class_ = None;
      on_class_c_tparams = None;
      on_class_c_extends = None;
      on_class_c_uses = None;
      on_class_c_xhp_attrs = None;
      on_class_c_xhp_attr_uses = None;
      on_class_c_req = None;
      on_class_c_reqs = None;
      on_class_c_implements = None;
      on_class_c_where_constraints = None;
      on_class_c_consts = None;
      on_class_c_typeconsts = None;
      on_class_c_vars = None;
      on_class_c_enum = None;
      on_class_c_methods = None;
      on_class_c_user_attributes = None;
      on_class_c_file_attributes = None;
      on_class_const_kind = None;
      on_class_var = None;
      on_class_var_cv_user_attributes = None;
      on_class_var_cv_expr = None;
      on_class_var_cv_type = None;
      on_typedef = None;
      on_gconst = None;
      on_gconst_cst_type = None;
      on_gconst_cst_value = None;
      on_fun_def = None;
      on_module_def = None;
      on_stmt = None;
      on_stmt_ = None;
      on_block = None;
      on_using_stmt = None;
      on_hint = None;
      on_hint_ = None;
      on_hint_fun = None;
      on_hint_fun_hf_param_tys = None;
      on_hint_fun_hf_variadic_ty = None;
      on_hint_fun_hf_ctxs = None;
      on_hint_fun_hf_return_ty = None;
      on_nast_shape_info = None;
      on_shape_field_info = None;
      on_nsi_field_map = None;
      on_shape_field_name = None;
      on_refinement = None;
      on_type_refinement = None;
      on_type_refinement_bounds = None;
      on_ctx_refinement = None;
      on_ctx_refinement_bounds = None;
      on_expr = None;
      on_expr_ = None;
      on_fun_ = None;
      on_fun_f_ret = None;
      on_fun_f_tparams = None;
      on_fun_f_where_constraints = None;
      on_fun_f_params = None;
      on_fun_f_ctxs = None;
      on_fun_f_unsafe_ctxs = None;
      on_fun_f_body = None;
      on_fun_f_user_attributes = None;
      on_method_ = None;
      on_method_m_ret = None;
      on_method_m_tparams = None;
      on_method_m_where_constraints = None;
      on_method_m_params = None;
      on_method_m_ctxs = None;
      on_method_m_unsafe_ctxs = None;
      on_method_m_body = None;
      on_method_m_user_attributes = None;
      on_class_id = None;
      on_class_id_ = None;
      on_func_body = None;
      on_enum_ = None;
      on_tparam = None;
      on_user_attributes = None;
      on_user_attribute = None;
      on_file_attributes = None;
      on_file_attribute = None;
      on_where_constraint_hints = None;
      on_where_constraint_hint = None;
      on_contexts = None;
      on_context = None;
      on_targ = None;
      on_as_expr = None;
      on_class_get_expr = None;
      on_afield = None;
      on_collection_targ = None;
      on_xhp_attribute = None;
      on_xhp_simple = None;
      on_function_ptr_id = None;
      on_tparams = None;
      on_tparam_tparams = None;
      on_tparam_tp_constraints = None;
      on_tp_constraint = None;
      on_tparam_tp_user_attributes = None;
      on_catch = None;
      on_case = None;
      on_default_case = None;
      on_fun_param_user_attributes = None;
      on_fun_param = None;
      on_fun_params = None;
      on_xhp_attr = None;
      on_class_const = None;
      on_class_typeconst_def = None;
      on_class_concrete_typeconst = None;
      on_class_abstract_typeconst = None;
      on_class_typeconst = None;
      on_def = None;
      on_expression_tree = None;
    }

  module Traversal = struct
    type ('env, 'err) handler = {
      top_down: ('env, 'err list) t;
      bottom_up: ('env, 'err list) t;
      on_error: 'err -> unit;
    }

    let on_tuple2 f g (fst, snd) ~env ~handler =
      let fst = f fst ~env ~handler in
      let snd = g snd ~env ~handler in
      (fst, snd)

    let on_snd f (fst, snd) ~env ~handler =
      let snd = f snd ~env ~handler in
      (fst, snd)

    let on_fst f (fst, snd) ~env ~handler =
      let fst = f fst ~env ~handler in
      (fst, snd)

    let on_list f elems ~env ~handler =
      List.map elems ~f:(fun elem -> f elem ~env ~handler)

    let on_option f elem_opt ~env ~handler =
      Option.map elem_opt ~f:(fun elem -> f elem ~env ~handler)

    let or_ok t_opt = Option.value ~default:(fun x -> Ok x) t_opt

    let traverse_id elem ~env:_ ~handler:_ = elem

    (* -- Hints ----------------------------------------------------------------- *)

    let rec on_hint (elem : hint) ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_hint) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_snd on_hint_) elem ~env ~handler in
              (or_ok handler.bottom_up.on_hint) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_hint_ elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_hint_) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_hint_ elem ~env ~handler in
              (or_ok handler.bottom_up.on_hint_) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_hint_ elem ~env ~handler =
      match elem with
      | Aast.Hoption hint -> Aast.Hoption (on_hint hint ~env ~handler)
      | Aast.Hlike hint -> Aast.Hlike (on_hint hint ~env ~handler)
      | Aast.Hfun hint_fun -> Aast.Hfun (on_hint_fun hint_fun ~env ~handler)
      | Aast.Htuple hints -> Aast.Htuple (on_list on_hint hints ~env ~handler)
      | Aast.Happly (class_id, hints) ->
        Aast.Happly (class_id, on_list on_hint hints ~env ~handler)
      | Aast.Hshape nast_shape_info ->
        Aast.Hshape (on_nast_shape_info nast_shape_info ~env ~handler)
      | Aast.Haccess (hint, id) -> Aast.Haccess (on_hint hint ~env ~handler, id)
      | Aast.Hsoft hint -> Aast.Hsoft (on_hint hint ~env ~handler)
      | Aast.Hrefinement (hint, refinements) ->
        Aast.Hrefinement
          ( on_hint hint ~env ~handler,
            on_list on_refinement refinements ~env ~handler )
      | Aast.Hvec_or_dict (hint_opt, hint) ->
        Aast.Hvec_or_dict
          (on_option on_hint hint_opt ~env ~handler, on_hint hint ~env ~handler)
      | Aast.Habstr (name, hints) ->
        Aast.Habstr (name, on_list on_hint hints ~env ~handler)
      | Aast.Hunion hints -> Aast.Hunion (on_list on_hint hints ~env ~handler)
      | Aast.Hintersection hints ->
        Aast.Hintersection (on_list on_hint hints ~env ~handler)
      | Aast.(
          ( Hany | Herr | Hmixed | Hnonnull | Hprim _ | Hthis | Hdynamic
          | Hnothing | Hvar _ | Hfun_context _ )) ->
        elem

    and on_hint_fun elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_hint_fun) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_hint_fun elem ~env ~handler in
              (or_ok handler.bottom_up.on_hint_fun) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_hint_fun hfun ~env ~handler =
      let hf_param_tys =
        on_hint_fun_hf_param_tys hfun.Aast.hf_param_tys ~env ~handler
      and hf_variadic_ty =
        on_hint_fun_hf_variadic_ty hfun.Aast.hf_variadic_ty ~env ~handler
      and hf_ctxs = on_hint_fun_hf_ctxs hfun.Aast.hf_ctxs ~env ~handler
      and hf_return_ty =
        on_hint_fun_hf_return_ty hfun.Aast.hf_return_ty ~env ~handler
      in
      Aast.{ hfun with hf_param_tys; hf_variadic_ty; hf_ctxs; hf_return_ty }

    and on_hint_fun_hf_return_ty elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_hint_fun_hf_return_ty) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_hint elem ~env ~handler in
              (or_ok handler.bottom_up.on_hint_fun_hf_return_ty)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_hint_fun_hf_param_tys elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_hint_fun_hf_param_tys) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_hint) elem ~env ~handler in
              (or_ok handler.bottom_up.on_hint_fun_hf_param_tys)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_hint_fun_hf_variadic_ty elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_hint_fun_hf_variadic_ty) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_option on_hint) elem ~env ~handler in
              (or_ok handler.bottom_up.on_hint_fun_hf_variadic_ty)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_hint_fun_hf_ctxs elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_hint_fun_hf_ctxs) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_option on_contexts) elem ~env ~handler in
              (or_ok handler.bottom_up.on_hint_fun_hf_ctxs) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_contexts elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_contexts) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_snd (on_list on_context) elem ~env ~handler in
              (or_ok handler.bottom_up.on_contexts) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_context elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_context) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_hint elem ~env ~handler in
              (or_ok handler.bottom_up.on_context) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_nast_shape_info elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_nast_shape_info) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_nast_shape_info elem ~env ~handler in
              (or_ok handler.bottom_up.on_nast_shape_info) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_nast_shape_info nast_shape_info ~env ~handler =
      let nsi_field_map =
        on_nsi_field_map nast_shape_info.Aast.nsi_field_map ~env ~handler
      in
      Aast.{ nast_shape_info with nsi_field_map }

    and on_nsi_field_map elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_nsi_field_map) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_shape_field_info) elem ~env ~handler in
              (or_ok handler.bottom_up.on_nsi_field_map) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_shape_field_info elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_shape_field_info) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_shape_field_info elem ~env ~handler in
              (or_ok handler.bottom_up.on_shape_field_info) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_shape_field_info elem ~env ~handler =
      let sfi_hint = on_hint elem.sfi_hint ~env ~handler
      and sfi_name = on_shape_field_name elem.sfi_name ~env ~handler in
      { elem with sfi_hint; sfi_name }

    and on_shape_field_name elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_shape_field_name) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_id elem ~env ~handler in
              (or_ok handler.bottom_up.on_shape_field_name) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_refinement elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_refinement) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_refinement elem ~env ~handler in
              (or_ok handler.bottom_up.on_refinement) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_refinement refinement ~env ~handler =
      match refinement with
      | Aast.Rctx (sid, ctx_refinement) ->
        Aast.Rctx (sid, on_ctx_refinement ctx_refinement ~env ~handler)
      | Aast.Rtype (sid, type_refinement) ->
        Aast.Rtype (sid, on_type_refinement type_refinement ~env ~handler)

    and on_type_refinement elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_type_refinement) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_type_refinement elem ~env ~handler in
              (or_ok handler.bottom_up.on_type_refinement) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_type_refinement type_refinement ~env ~handler =
      match type_refinement with
      | TRexact hint -> TRexact (on_hint hint ~env ~handler)
      | TRloose type_refinement_bounds ->
        TRloose (on_type_refinement_bounds type_refinement_bounds ~env ~handler)

    and on_type_refinement_bounds elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_type_refinement_bounds) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_type_refinement_bounds elem ~env ~handler in
              (or_ok handler.bottom_up.on_type_refinement_bounds)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_type_refinement_bounds
        Aast.{ tr_lower; tr_upper } ~env ~handler =
      Aast.
        {
          tr_lower = List.map ~f:(on_hint ~env ~handler) tr_lower;
          tr_upper = List.map ~f:(on_hint ~env ~handler) tr_upper;
        }

    and on_ctx_refinement elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_ctx_refinement) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_ctx_refinement elem ~env ~handler in
              (or_ok handler.bottom_up.on_ctx_refinement) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_ctx_refinement ctx_refinement ~env ~handler =
      match ctx_refinement with
      | Aast.CRexact hint -> Aast.CRexact (on_hint hint ~env ~handler)
      | Aast.CRloose ctx_refinement_bounds ->
        Aast.CRloose
          (on_ctx_refinement_bounds ctx_refinement_bounds ~env ~handler)

    and on_ctx_refinement_bounds elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_ctx_refinement_bounds) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_ctx_refinement_bounds elem ~env ~handler in
              (or_ok handler.bottom_up.on_ctx_refinement_bounds)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_ctx_refinement_bounds Aast.{ cr_lower; cr_upper } ~env ~handler
        =
      Aast.
        {
          cr_lower = Option.map ~f:(on_hint ~env ~handler) cr_lower;
          cr_upper = Option.map ~f:(on_hint ~env ~handler) cr_upper;
        }

    (* -- Class type constants -------------------------------------------------- *)

    let rec on_class_typeconst elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_typeconst) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_class_typeconst elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_typeconst) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_class_typeconst elem ~env ~handler =
      match elem with
      | TCAbstract class_abstract_typeconst ->
        TCAbstract
          (on_class_abstract_typeconst class_abstract_typeconst ~env ~handler)
      | TCConcrete class_abstract_typeconst ->
        TCConcrete
          (on_class_concrete_typeconst class_abstract_typeconst ~env ~handler)

    and on_class_abstract_typeconst elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_abstract_typeconst)
                (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_class_abstract_typeconst elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_abstract_typeconst)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_class_abstract_typeconst elem ~env ~handler =
      let c_atc_as_constraint =
        on_option on_hint elem.c_atc_as_constraint ~env ~handler
      and c_atc_super_constraint =
        on_option on_hint elem.c_atc_super_constraint ~env ~handler
      and c_atc_default = on_option on_hint elem.c_atc_default ~env ~handler in
      { c_atc_as_constraint; c_atc_super_constraint; c_atc_default }

    and on_class_concrete_typeconst elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_concrete_typeconst)
                (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_class_concrete_typeconst elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_concrete_typeconst)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_class_concrete_typeconst { c_tc_type } ~env ~handler =
      let c_tc_type = on_hint c_tc_type ~env ~handler in
      { c_tc_type }

    let traverse_enum_ { e_base; e_constraint; e_includes } ~env ~handler =
      let e_base = on_hint e_base ~env ~handler
      and e_constraint = on_option on_hint e_constraint ~env ~handler
      and e_includes = on_list on_hint e_includes ~env ~handler in
      { e_base; e_constraint; e_includes }

    let on_enum_ elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_enum_) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_enum_ elem ~env ~handler in
              (or_ok handler.bottom_up.on_enum_) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    let on_targ elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_targ) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_snd on_hint) elem ~env ~handler in
              (or_ok handler.bottom_up.on_targ) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    let rec on_expr elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_expr) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_expr elem ~env ~handler in
              (or_ok handler.bottom_up.on_expr) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_expr (annot, pos, expr_) ~env ~handler =
      let expr_ = on_expr_ expr_ ~env ~handler in
      (annot, pos, expr_)

    and on_expr_ elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_expr_) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_expr_ elem ~env ~handler in
              (or_ok handler.bottom_up.on_expr_) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_expr_ expr_ ~env ~handler =
      match expr_ with
      | Darray (kv_targs_opt, kvs) ->
        Darray
          ( on_option (on_tuple2 on_targ on_targ) kv_targs_opt ~env ~handler,
            on_list (on_tuple2 on_expr on_expr) kvs ~env ~handler )
      | Varray (targ_opt, exprs) ->
        Varray
          ( on_option on_targ targ_opt ~env ~handler,
            on_list on_expr exprs ~env ~handler )
      | Shape flds ->
        Shape
          (on_list (on_tuple2 on_shape_field_name on_expr) flds ~env ~handler)
      | ValCollection (vc_kind, targ_opt, exprs) ->
        ValCollection
          ( vc_kind,
            on_option on_targ targ_opt ~env ~handler,
            on_list on_expr exprs ~env ~handler )
      | KeyValCollection (kvc_kind, kv_targs_opt, fields) ->
        KeyValCollection
          ( kvc_kind,
            on_option (on_tuple2 on_targ on_targ) kv_targs_opt ~env ~handler,
            on_list (on_tuple2 on_expr on_expr) fields ~env ~handler )
      | Clone expr -> Clone (on_expr expr ~env ~handler)
      | Array_get (expr, expr_opt) ->
        Array_get
          (on_expr expr ~env ~handler, on_option on_expr expr_opt ~env ~handler)
      | Obj_get (rcvr, member, null_flavor, prop_or_meth) ->
        Obj_get
          ( on_expr rcvr ~env ~handler,
            on_expr member ~env ~handler,
            null_flavor,
            prop_or_meth )
      | Class_get (class_id, class_get_expr, prop_or_meth) ->
        Class_get
          ( on_class_id class_id ~env ~handler,
            on_class_get_expr class_get_expr ~env ~handler,
            prop_or_meth )
      | Class_const (class_id, name) ->
        Class_const (on_class_id class_id ~env ~handler, name)
      | Call (fn_expr, targs, args, unpacked_arg) ->
        Call
          ( on_expr fn_expr ~env ~handler,
            on_list on_targ targs ~env ~handler,
            on_list (on_snd on_expr) args ~env ~handler,
            on_option on_expr unpacked_arg ~env ~handler )
      | FunctionPointer (id, targs) ->
        FunctionPointer
          ( on_function_ptr_id id ~env ~handler,
            on_list on_targ targs ~env ~handler )
      | String2 exprs -> String2 (on_list on_expr exprs ~env ~handler)
      | PrefixedString (str, expr) ->
        PrefixedString (str, on_expr expr ~env ~handler)
      | Yield afield -> Yield (on_afield afield ~env ~handler)
      | Await expr -> Await (on_expr expr ~env ~handler)
      | ReadonlyExpr expr -> ReadonlyExpr (on_expr expr ~env ~handler)
      | Tuple exprs -> Tuple (on_list on_expr exprs ~env ~handler)
      | List exprs -> List (on_list on_expr exprs ~env ~handler)
      | Cast (hint, expr) ->
        Cast (on_hint hint ~env ~handler, on_expr expr ~env ~handler)
      | Unop (op, expr) -> Unop (op, on_expr expr ~env ~handler)
      | Binop (op, le, re) ->
        Binop (op, on_expr le ~env ~handler, on_expr re ~env ~handler)
      | Pipe (lid, e1, e2) ->
        Pipe (lid, on_expr e1 ~env ~handler, on_expr e2 ~env ~handler)
      | Eif (e1, e2_opt, e3) ->
        Eif
          ( on_expr e1 ~env ~handler,
            on_option on_expr e2_opt ~env ~handler,
            on_expr e3 ~env ~handler )
      | Is (expr, hint) ->
        Is (on_expr expr ~env ~handler, on_hint hint ~env ~handler)
      | As (expr, hint, nullable) ->
        As (on_expr expr ~env ~handler, on_hint hint ~env ~handler, nullable)
      | Upcast (expr, hint) ->
        Upcast (on_expr expr ~env ~handler, on_hint hint ~env ~handler)
      | New (class_id, targs, exprs, expr_opt, ex) ->
        New
          ( on_class_id class_id ~env ~handler,
            on_list on_targ targs ~env ~handler,
            on_list on_expr exprs ~env ~handler,
            on_option on_expr expr_opt ~env ~handler,
            ex )
      | Efun efun ->
        Efun { efun with ef_fun = on_fun_ efun.ef_fun ~env ~handler }
      | Lfun (fun_, lids) -> Lfun (on_fun_ fun_ ~env ~handler, lids)
      | Xml (class_name, xhp_attrs, exprs) ->
        Xml
          ( class_name,
            on_list on_xhp_attribute xhp_attrs ~env ~handler,
            on_list on_expr exprs ~env ~handler )
      | Import (flav, expr) -> Import (flav, on_expr expr ~env ~handler)
      | Collection (class_name, ctarg_opt, afields) ->
        Collection
          ( class_name,
            on_option on_collection_targ ctarg_opt ~env ~handler,
            on_list on_afield afields ~env ~handler )
      | Method_id (expr, id) -> Method_id (on_expr expr ~env ~handler, id)
      | Smethod_id (class_id, id) ->
        Smethod_id (on_class_id class_id ~env ~handler, id)
      | Pair (targs_opt, e1, e2) ->
        Pair
          ( on_option (on_tuple2 on_targ on_targ) targs_opt ~env ~handler,
            on_expr e1 ~env ~handler,
            on_expr e2 ~env ~handler )
      | Hole (expr, ex1, ex2, src) ->
        Hole (on_expr expr ~env ~handler, ex1, ex2, src)
      | ET_Splice expr -> ET_Splice (on_expr expr ~env ~handler)
      | ExpressionTree expr_tree ->
        ExpressionTree (on_expression_tree expr_tree ~env ~handler)
      | EnumClassLabel _
      | Null
      | This
      | True
      | False
      | Omitted
      | Id _
      | Lvar _
      | Dollardollar _
      | Int _
      | Float _
      | String _
      | Lplaceholder _
      | Fun_id _
      | Method_caller _ ->
        expr_

    and on_expression_tree elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_expression_tree) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_expression_tree elem ~env ~handler in
              (or_ok handler.bottom_up.on_expression_tree) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_expression_tree elem ~env ~handler =
      let et_hint = on_hint elem.et_hint ~env ~handler
      and et_splices = on_list on_stmt elem.et_splices ~env ~handler
      and et_function_pointers =
        on_list on_stmt elem.et_function_pointers ~env ~handler
      and et_virtualized_expr = on_expr elem.et_virtualized_expr ~env ~handler
      and et_runtime_expr = on_expr elem.et_runtime_expr ~env ~handler in
      {
        elem with
        et_hint;
        et_splices;
        et_function_pointers;
        et_virtualized_expr;
        et_runtime_expr;
      }

    and on_class_get_expr elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_get_expr) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_class_get_expr elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_get_expr) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_class_get_expr elem ~env ~handler =
      match elem with
      | CGstring _ -> elem
      | CGexpr expr -> CGexpr (on_expr expr ~env ~handler)

    and on_afield elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_afield) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_afield elem ~env ~handler in
              (or_ok handler.bottom_up.on_afield) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_afield elem ~env ~handler =
      match elem with
      | AFvalue expr -> AFvalue (on_expr expr ~env ~handler)
      | AFkvalue (e1, e2) ->
        AFkvalue (on_expr e1 ~env ~handler, on_expr e2 ~env ~handler)

    and on_class_id elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_id) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_class_id elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_id) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_class_id (ex, pos, class_id_) ~env ~handler =
      let class_id_ = on_class_id_ class_id_ ~env ~handler in
      (ex, pos, class_id_)

    and on_class_id_ elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_id_) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_class_id_ elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_id_) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_class_id_ class_id_ ~env ~handler =
      match class_id_ with
      | CIparent
      | CIself
      | CIstatic
      | CI _ ->
        class_id_
      | CIexpr expr -> CIexpr (on_expr expr ~env ~handler)

    and on_collection_targ elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_collection_targ) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_collection_targ elem ~env ~handler in
              (or_ok handler.bottom_up.on_collection_targ) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_collection_targ elem ~env ~handler =
      match elem with
      | CollectionTV targ -> CollectionTV (on_targ targ ~env ~handler)
      | CollectionTKV (ktarg, vtarg) ->
        CollectionTKV (on_targ ktarg ~env ~handler, on_targ vtarg ~env ~handler)

    and on_xhp_attribute elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_xhp_attribute) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_xhp_attribute elem ~env ~handler in
              (or_ok handler.bottom_up.on_xhp_attribute) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_xhp_attribute elem ~env ~handler =
      match elem with
      | Xhp_simple xhp_simple ->
        Xhp_simple (on_xhp_simple xhp_simple ~env ~handler)
      | Xhp_spread expr -> Xhp_spread (on_expr expr ~env ~handler)

    and on_xhp_simple elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_xhp_simple) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_xhp_simple elem ~env ~handler in
              (or_ok handler.bottom_up.on_xhp_simple) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_xhp_simple elem ~env ~handler =
      let xs_expr = on_expr elem.xs_expr ~env ~handler in
      { elem with xs_expr }

    and on_function_ptr_id elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_function_ptr_id) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_function_ptr_id elem ~env ~handler in
              (or_ok handler.bottom_up.on_function_ptr_id) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_function_ptr_id elem ~env ~handler =
      match elem with
      | FP_id _ -> elem
      | FP_class_const (class_id, str) ->
        FP_class_const (on_class_id class_id ~env ~handler, str)

    and on_as_expr elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_as_expr) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_as_expr elem ~env ~handler in
              (or_ok handler.bottom_up.on_as_expr) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_as_expr elem ~env ~handler =
      match elem with
      | As_v expr -> As_v (on_expr expr ~env ~handler)
      | As_kv (e1, e2) ->
        As_kv (on_expr e1 ~env ~handler, on_expr e2 ~env ~handler)
      | Await_as_v (pos, expr) -> Await_as_v (pos, on_expr expr ~env ~handler)
      | Await_as_kv (pos, e1, e2) ->
        Await_as_kv (pos, on_expr e1 ~env ~handler, on_expr e2 ~env ~handler)

    and on_fun_ elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_fun_) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_fun_ elem ~env ~handler in
              (or_ok handler.bottom_up.on_fun_) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_fun_ f ~env ~handler =
      let f_ret = on_fun_f_ret f.Aast.f_ret ~env ~handler
      and f_tparams = on_fun_f_tparams f.Aast.f_tparams ~env ~handler
      and f_where_constraints =
        on_fun_f_where_constraints f.Aast.f_where_constraints ~env ~handler
      and f_params = on_fun_f_params f.Aast.f_params ~env ~handler
      and f_ctxs = on_fun_f_ctxs f.Aast.f_ctxs ~env ~handler
      and f_unsafe_ctxs =
        on_fun_f_unsafe_ctxs f.Aast.f_unsafe_ctxs ~env ~handler
      and f_body = on_fun_f_body f.Aast.f_body ~env ~handler
      and f_user_attributes =
        on_fun_f_user_attributes f.Aast.f_user_attributes ~env ~handler
      in
      {
        f with
        f_ret;
        f_tparams;
        f_where_constraints;
        f_params;
        f_ctxs;
        f_unsafe_ctxs;
        f_body;
        f_user_attributes;
      }

    and on_fun_f_ret elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_fun_f_ret) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_snd (on_option on_hint)) elem ~env ~handler in
              (or_ok handler.bottom_up.on_fun_f_ret) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_fun_f_tparams elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_fun_f_tparams) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_tparams elem ~env ~handler in
              (or_ok handler.bottom_up.on_fun_f_tparams) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_fun_f_where_constraints elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_fun_f_where_constraints) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_where_constraint_hints elem ~env ~handler in
              (or_ok handler.bottom_up.on_fun_f_where_constraints)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_fun_f_params elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_fun_f_params) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_fun_params elem ~env ~handler in
              (or_ok handler.bottom_up.on_fun_f_params) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_fun_f_ctxs elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_fun_f_ctxs) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_option on_contexts) elem ~env ~handler in
              (or_ok handler.bottom_up.on_fun_f_ctxs) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_fun_f_unsafe_ctxs elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_fun_f_unsafe_ctxs) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_option on_contexts) elem ~env ~handler in
              (or_ok handler.bottom_up.on_fun_f_unsafe_ctxs) (env, elem, errs)
            ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_fun_f_body elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_fun_f_body) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_func_body elem ~env ~handler in
              (or_ok handler.bottom_up.on_fun_f_body) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_fun_f_user_attributes elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_fun_f_user_attributes) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_user_attributes elem ~env ~handler in
              (or_ok handler.bottom_up.on_fun_f_user_attributes)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_fun_params elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_fun_params) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_fun_param) elem ~env ~handler in
              (or_ok handler.bottom_up.on_fun_params) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_fun_param elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_fun_param) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_fun_param elem ~env ~handler in
              (or_ok handler.bottom_up.on_fun_param) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_fun_param elem ~env ~handler =
      let param_type_hint =
        on_snd (on_option on_hint) elem.param_type_hint ~env ~handler
      and param_expr = on_option on_expr elem.param_expr ~env ~handler
      and param_user_attributes =
        on_fun_param_user_attributes elem.param_user_attributes ~env ~handler
      in
      { elem with param_type_hint; param_expr; param_user_attributes }

    and on_fun_param_user_attributes elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_fun_param_user_attributes)
                (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_user_attributes elem ~env ~handler in
              (or_ok handler.bottom_up.on_fun_param_user_attributes)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_func_body elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_func_body) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_func_body elem ~env ~handler in
              (or_ok handler.bottom_up.on_func_body) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_func_body { fb_ast } ~env ~handler =
      let fb_ast = on_block fb_ast ~env ~handler in
      { fb_ast }

    and on_tparams elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_tparams) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_tparam) elem ~env ~handler in
              (or_ok handler.bottom_up.on_tparams) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_tparam elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_tparam) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_tparam elem ~env ~handler in
              (or_ok handler.bottom_up.on_tparam) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_tparam tparam ~env ~handler =
      let tp_parameters = on_tparams tparam.tp_parameters ~env ~handler
      and tp_constraints =
        on_list (on_snd on_hint) tparam.tp_constraints ~env ~handler
      and tp_user_attributes =
        on_user_attributes tparam.tp_user_attributes ~env ~handler
      in
      { tparam with tp_parameters; tp_constraints; tp_user_attributes }

    and on_user_attributes elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_user_attributes) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_user_attribute) elem ~env ~handler in
              (or_ok handler.bottom_up.on_user_attributes) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_user_attribute elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_user_attribute) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_user_attribute elem ~env ~handler in
              (or_ok handler.bottom_up.on_user_attribute) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_user_attribute user_attribute ~env ~handler =
      let ua_params = on_list on_expr user_attribute.ua_params ~env ~handler in
      { user_attribute with ua_params }

    and on_file_attributes elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_file_attributes) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_file_attribute) elem ~env ~handler in
              (or_ok handler.bottom_up.on_file_attributes) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_file_attribute elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_file_attribute) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_file_attribute elem ~env ~handler in
              (or_ok handler.bottom_up.on_file_attribute) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_file_attribute file_attribute ~env ~handler =
      let fa_user_attributes =
        on_user_attributes file_attribute.fa_user_attributes ~env ~handler
      in
      { file_attribute with fa_user_attributes }

    and on_where_constraint_hints elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_where_constraint_hints) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem =
                (on_list on_where_constraint_hint) elem ~env ~handler
              in
              (or_ok handler.bottom_up.on_where_constraint_hints)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_where_constraint_hint elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_where_constraint_hint) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_where_constraint_hint elem ~env ~handler in
              (or_ok handler.bottom_up.on_where_constraint_hint)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_where_constraint_hint (h1, cstr, h2) ~env ~handler =
      let h1 = on_hint h1 ~env ~handler and h2 = on_hint h2 ~env ~handler in
      (h1, cstr, h2)

    and on_stmt elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_stmt) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_snd on_stmt_) elem ~env ~handler in
              (or_ok handler.bottom_up.on_stmt) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_stmt_ elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_stmt_) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_stmt_ elem ~env ~handler in
              (or_ok handler.bottom_up.on_stmt_) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_stmt_ elem ~env ~handler =
      match elem with
      | Expr expr -> Expr (on_expr expr ~env ~handler)
      | Throw expr -> Throw (on_expr expr ~env ~handler)
      | Return expr_opt -> Return (on_option on_expr expr_opt ~env ~handler)
      | Awaitall (temps, block) ->
        Awaitall
          ( on_list (on_snd on_expr) temps ~env ~handler,
            on_block block ~env ~handler )
      | If (expr, b1, b2) ->
        If
          ( on_expr expr ~env ~handler,
            on_block b1 ~env ~handler,
            on_block b2 ~env ~handler )
      | Do (block, expr) ->
        Do (on_block block ~env ~handler, on_expr expr ~env ~handler)
      | While (expr, block) ->
        While (on_expr expr ~env ~handler, on_block block ~env ~handler)
      | Switch (expr, cases, default_case_opt) ->
        Switch
          ( on_expr expr ~env ~handler,
            on_list on_case cases ~env ~handler,
            on_option on_default_case default_case_opt ~env ~handler )
      | Using using -> Using (on_using_stmt using ~env ~handler)
      | For (vexprs, cexpr, mexprs, block) ->
        For
          ( on_list on_expr vexprs ~env ~handler,
            on_option on_expr cexpr ~env ~handler,
            on_list on_expr mexprs ~env ~handler,
            on_block block ~env ~handler )
      | Foreach (expr, as_expr, block) ->
        Foreach
          ( on_expr expr ~env ~handler,
            on_as_expr as_expr ~env ~handler,
            on_block block ~env ~handler )
      | Try (block, catches, finally) ->
        Try
          ( on_block block ~env ~handler,
            on_list on_catch catches ~env ~handler,
            on_block finally ~env ~handler )
      | Block block -> Block (on_block block ~env ~handler)
      | AssertEnv _
      | Markup _
      | Noop
      | Fallthrough
      | Break
      | Yield_break
      | Continue ->
        elem

    and on_block elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_block) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_stmt) elem ~env ~handler in
              (or_ok handler.bottom_up.on_block) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_using_stmt elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_using_stmt) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_using_stmt elem ~env ~handler in
              (or_ok handler.bottom_up.on_using_stmt) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_using_stmt elem ~env ~handler =
      let us_exprs = on_snd (on_list on_expr) elem.us_exprs ~env ~handler
      and us_block = on_block elem.us_block ~env ~handler in
      { elem with us_exprs; us_block }

    and on_catch elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_catch) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_catch elem ~env ~handler in
              (or_ok handler.bottom_up.on_catch) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_catch (class_name, lid, block) ~env ~handler =
      let block = on_block block ~env ~handler in
      (class_name, lid, block)

    and on_case elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_case) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_tuple2 on_expr on_block) elem ~env ~handler in
              (or_ok handler.bottom_up.on_case) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_default_case elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_default_case) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_snd on_block) elem ~env ~handler in
              (or_ok handler.bottom_up.on_default_case) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_ elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_class elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_class c ~env ~handler =
      let c_tparams = on_class_c_tparams c.c_tparams ~env ~handler
      and c_extends = on_class_c_extends c.c_extends ~env ~handler
      and c_uses = on_class_c_uses c.c_uses ~env ~handler
      and c_xhp_attrs = on_class_c_xhp_attrs c.c_xhp_attrs ~env ~handler
      and c_xhp_attr_uses =
        on_class_c_xhp_attr_uses c.c_xhp_attr_uses ~env ~handler
      and c_reqs = on_class_c_reqs c.c_reqs ~env ~handler
      and c_implements = on_class_c_implements c.c_implements ~env ~handler
      and c_where_constraints =
        on_class_c_where_constraints c.c_where_constraints ~env ~handler
      and c_consts = on_class_c_consts c.c_consts ~env ~handler
      and c_typeconsts = on_class_c_typeconsts c.c_typeconsts ~env ~handler
      and c_vars = on_class_c_vars c.c_vars ~env ~handler
      and c_enum = on_class_c_enum c.c_enum ~env ~handler
      and c_methods = on_class_c_methods c.c_methods ~env ~handler
      and c_user_attributes =
        on_class_c_user_attributes c.c_user_attributes ~env ~handler
      and c_file_attributes =
        on_class_c_file_attributes c.c_file_attributes ~env ~handler
      in
      {
        c with
        c_tparams;
        c_extends;
        c_uses;
        c_xhp_attr_uses;
        c_reqs;
        c_implements;
        c_where_constraints;
        c_consts;
        c_typeconsts;
        c_vars;
        c_enum;
        c_methods;
        c_xhp_attrs;
        c_user_attributes;
        c_file_attributes;
      }

    and on_class_c_tparams elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_tparams) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_tparams elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_tparams) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_c_extends elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_extends) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_hint) elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_extends) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_c_uses elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_uses) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_hint) elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_uses) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_c_xhp_attrs elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_xhp_attrs) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_xhp_attr) elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_xhp_attrs) (env, elem, errs)
            ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_xhp_attr elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_xhp_attr) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_xhp_attr elem ~env ~handler in
              (or_ok handler.bottom_up.on_xhp_attr) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_xhp_attr
        (type_hint, class_var, tag_opt, pos_exprs_opt) ~env ~handler =
      let type_hint = on_snd (on_option on_hint) type_hint ~env ~handler
      and class_var = on_class_var class_var ~env ~handler
      and pos_exprs_opt =
        on_option (on_snd @@ on_list on_expr) pos_exprs_opt ~env ~handler
      in
      (type_hint, class_var, tag_opt, pos_exprs_opt)

    and on_class_c_xhp_attr_uses elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_xhp_attr_uses) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_hint) elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_xhp_attr_uses)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_c_req elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_req) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_fst on_hint) elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_req) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_c_reqs elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_reqs) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_class_c_req) elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_reqs) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_c_implements elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_implements) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_hint) elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_implements) (env, elem, errs)
            ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_c_where_constraints elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_where_constraints)
                (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_where_constraint_hints elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_where_constraints)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_c_consts elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_consts) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_class_const) elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_consts) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_c_typeconsts elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_typeconsts) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_class_typeconst_def) elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_typeconsts) (env, elem, errs)
            ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_c_vars elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_vars) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_class_var) elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_vars) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_c_enum elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_enum) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_option on_enum_) elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_enum) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_c_methods elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_methods) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_list on_method_) elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_methods) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_c_user_attributes elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_user_attributes) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_user_attributes elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_user_attributes)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_c_file_attributes elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_c_file_attributes) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_file_attributes elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_c_file_attributes)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_const elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_const) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_class_const elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_const) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_class_const elem ~env ~handler =
      let cc_user_attributes =
        on_user_attributes elem.cc_user_attributes ~env ~handler
      and cc_type = on_option on_hint elem.cc_type ~env ~handler
      and cc_kind = on_class_const_kind elem.cc_kind ~env ~handler in
      { elem with cc_user_attributes; cc_type; cc_kind }

    and on_class_const_kind elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_const_kind) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_class_const_kind elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_const_kind) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_class_const_kind elem ~env ~handler =
      match elem with
      | CCAbstract expr_opt ->
        CCAbstract (on_option on_expr expr_opt ~env ~handler)
      | CCConcrete expr -> CCConcrete (on_expr expr ~env ~handler)

    and on_class_typeconst_def elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_typeconst_def) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_class_typeconst_def elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_typeconst_def) (env, elem, errs)
            ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_class_typeconst_def elem ~env ~handler =
      let c_tconst_user_attributes =
        on_user_attributes elem.c_tconst_user_attributes ~env ~handler
      and c_tconst_kind = on_class_typeconst elem.c_tconst_kind ~env ~handler in
      { elem with c_tconst_user_attributes; c_tconst_kind }

    and on_class_var elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_var) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_class_var elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_var) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_class_var cv ~env ~handler =
      let cv_user_attributes =
        on_class_var_cv_user_attributes cv.cv_user_attributes ~env ~handler
      and cv_expr = on_class_var_cv_expr cv.cv_expr ~env ~handler
      and cv_type = on_class_var_cv_type cv.cv_type ~env ~handler in
      Aast.{ cv with cv_user_attributes; cv_type; cv_expr }

    and on_class_var_cv_user_attributes elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_var_cv_user_attributes)
                (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_user_attributes elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_var_cv_user_attributes)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_var_cv_expr elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_var_cv_expr) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_option on_expr) elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_var_cv_expr) (env, elem, errs)
            ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_class_var_cv_type elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_class_var_cv_type) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_snd (on_option on_hint)) elem ~env ~handler in
              (or_ok handler.bottom_up.on_class_var_cv_type) (env, elem, errs)
            ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_method_ elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_method_) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_method_ elem ~env ~handler in
              (or_ok handler.bottom_up.on_method_) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and traverse_method_ m ~env ~handler =
      let m_ret = on_method_m_ret m.m_ret ~env ~handler
      and m_tparams = on_method_m_tparams m.m_tparams ~env ~handler
      and m_where_constraints =
        on_method_m_where_constraints m.m_where_constraints ~env ~handler
      and m_params = on_method_m_params m.m_params ~env ~handler
      and m_ctxs = on_method_m_ctxs m.m_ctxs ~env ~handler
      and m_unsafe_ctxs = on_method_m_unsafe_ctxs m.m_unsafe_ctxs ~env ~handler
      and m_body = on_method_m_body m.m_body ~env ~handler
      and m_user_attributes =
        on_method_m_user_attributes m.m_user_attributes ~env ~handler
      in
      {
        m with
        m_ret;
        m_tparams;
        m_where_constraints;
        m_params;
        m_ctxs;
        m_unsafe_ctxs;
        m_body;
        m_user_attributes;
      }

    and on_method_m_ret elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_method_m_ret) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_snd (on_option on_hint)) elem ~env ~handler in
              (or_ok handler.bottom_up.on_method_m_ret) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_method_m_tparams elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_method_m_tparams) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_tparams elem ~env ~handler in
              (or_ok handler.bottom_up.on_method_m_tparams) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_method_m_where_constraints elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_method_m_where_constraints)
                (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_where_constraint_hints elem ~env ~handler in
              (or_ok handler.bottom_up.on_method_m_where_constraints)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_method_m_params elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_fun_f_params) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_fun_params elem ~env ~handler in
              (or_ok handler.bottom_up.on_fun_f_params) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_method_m_ctxs elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_method_m_ctxs) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_option on_contexts) elem ~env ~handler in
              (or_ok handler.bottom_up.on_method_m_ctxs) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_method_m_unsafe_ctxs elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_method_m_unsafe_ctxs) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_option on_contexts) elem ~env ~handler in
              (or_ok handler.bottom_up.on_method_m_unsafe_ctxs) (env, elem, errs)
            ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_method_m_body elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_method_m_body) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_func_body elem ~env ~handler in
              (or_ok handler.bottom_up.on_method_m_body) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    and on_method_m_user_attributes elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_method_m_user_attributes)
                (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_user_attributes elem ~env ~handler in
              (or_ok handler.bottom_up.on_method_m_user_attributes)
                (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    let on_gconst_cst_type elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_gconst_cst_type) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = (on_option on_hint) elem ~env ~handler in
              (or_ok handler.bottom_up.on_gconst_cst_type) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    let on_gconst_cst_value elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_gconst_cst_value) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_expr elem ~env ~handler in
              (or_ok handler.bottom_up.on_gconst_cst_value) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    let traverse_gconst cst ~env ~handler =
      let cst_type = on_gconst_cst_type cst.Aast.cst_type ~env ~handler
      and cst_value = on_gconst_cst_value cst.Aast.cst_value ~env ~handler in
      Aast.{ cst with cst_type; cst_value }

    let on_gconst elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_gconst) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_gconst elem ~env ~handler in
              (or_ok handler.bottom_up.on_gconst) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    let traverse_fun_def elem ~env ~handler =
      let fd_file_attributes =
        on_file_attributes elem.fd_file_attributes ~env ~handler
      and fd_fun = on_fun_ elem.fd_fun ~env ~handler in
      { elem with fd_fun; fd_file_attributes }

    let on_fun_def elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_fun_def) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_fun_def elem ~env ~handler in
              (or_ok handler.bottom_up.on_fun_def) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    let traverse_module_def elem ~env ~handler =
      let md_user_attributes =
        on_user_attributes elem.md_user_attributes ~env ~handler
      in
      { elem with md_user_attributes }

    let on_module_def elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_module_def) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_module_def elem ~env ~handler in
              (or_ok handler.bottom_up.on_module_def) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    let traverse_typedef elem ~env ~handler =
      let t_user_attributes =
        on_user_attributes elem.t_user_attributes ~env ~handler
      and t_file_attributes =
        on_file_attributes elem.t_file_attributes ~env ~handler
      and t_tparams = on_tparams elem.t_tparams ~env ~handler
      and t_as_constraint = on_option on_hint elem.t_as_constraint ~env ~handler
      and t_super_constraint =
        on_option on_hint elem.t_super_constraint ~env ~handler
      and t_kind = on_hint elem.t_kind ~env ~handler in
      {
        elem with
        t_user_attributes;
        t_file_attributes;
        t_tparams;
        t_as_constraint;
        t_super_constraint;
        t_kind;
      }

    let on_typedef elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_typedef) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_typedef elem ~env ~handler in
              (or_ok handler.bottom_up.on_typedef) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    let rec traverse_def def ~env ~handler =
      match def with
      | Aast.Class class_ -> Aast.Class (on_class_ class_ ~env ~handler)
      | Aast.Fun fn -> Aast.Fun (on_fun_def fn ~env ~handler)
      | Aast.Stmt stmt -> Aast.Stmt (on_stmt stmt ~env ~handler)
      | Aast.Typedef td -> Aast.Typedef (on_typedef td ~env ~handler)
      | Aast.Constant gconst -> Aast.Constant (on_gconst gconst ~env ~handler)
      | Aast.Module md -> Aast.Module (on_module_def md ~env ~handler)
      | Aast.Namespace (sid, defs) ->
        Aast.Namespace (sid, on_list on_def defs ~env ~handler)
      | _ -> def

    and on_def elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_def) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = traverse_def elem ~env ~handler in
              (or_ok handler.bottom_up.on_def) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem

    let on_program elem ~env ~handler =
      let (_, elem, errs) =
        Result.(
          fold
            ~ok:Fn.id
            ~error:Fn.id
            ( (or_ok handler.top_down.on_program) (env, elem, [])
            >>= fun (env, elem, errs) ->
              let elem = on_list on_def elem ~env ~handler in
              (or_ok handler.bottom_up.on_program) (env, elem, errs) ))
      in
      List.iter ~f:handler.on_error errs;
      elem
  end
end

type ('env, 'err) t =
  | Top_down of ('env, 'err) Ast_transform.t
  | Bottom_up of ('env, 'err) Ast_transform.t

let partition ts =
  List.fold_right ts ~init:([], []) ~f:(fun t (tds, bus) ->
      match t with
      | Top_down td -> (td :: tds, bus)
      | Bottom_up bu -> (tds, bu :: bus))

let combine ts =
  let (top_downs, bottom_ups) = partition ts in
  (Ast_transform.combine top_downs, Ast_transform.combine bottom_ups)

let top_down pass = Top_down pass

let bottom_up pass = Bottom_up pass

let mk_visitor passes ~on_error =
  let (top_down, bottom_up) = combine passes in
  let open Ast_transform.Traversal in
  let handler = { top_down; bottom_up; on_error } in
  ( (fun env elem -> on_program elem ~env ~handler),
    (fun env elem -> on_class_ elem ~env ~handler),
    (fun env elem -> on_fun_def elem ~env ~handler),
    (fun env elem -> on_module_def elem ~env ~handler),
    (fun env elem -> on_gconst elem ~env ~handler),
    (fun env elem -> on_typedef elem ~env ~handler) )

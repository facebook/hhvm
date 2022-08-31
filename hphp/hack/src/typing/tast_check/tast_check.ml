(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

let visitor ctx =
  let makers =
    [Xhp_required_check.make_handler; Redundant_generics_check.make_handler]
  in
  let handlers = List.map makers ~f:(( |> ) ctx) |> List.filter_opt in
  (* Handlers in effect only when a typechecker option is set and is not part
     of standard typechecker operation. *)
  let irregular_handlers =
    let tco = Provider_context.get_tcopt ctx in
    let log_levels = TypecheckerOptions.log_levels tco in
    let add_handler handlers (key, handler) =
      match SMap.find_opt key log_levels with
      | Some level when level > 0 -> handler :: handlers
      | _ -> handlers
    in
    let key_handler_pairs =
      [
        ("tany", Bad_type_logger.handler);
        ("shape_analysis", Shape_analysis_logger.handler);
      ]
    in
    List.fold ~init:[] ~f:add_handler key_handler_pairs
  in
  let skip_hierarchy_checks =
    TypecheckerOptions.skip_hierarchy_checks (Provider_context.get_tcopt ctx)
  in
  let is_global_write_check_enabled =
    0
    <> List.length
         (TypecheckerOptions.global_write_check_enabled
            (Provider_context.get_tcopt ctx))
    || 0
       <> SSet.cardinal
            (TypecheckerOptions.global_write_check_functions_enabled
               (Provider_context.get_tcopt ctx))
  in
  let handlers =
    handlers
    @ irregular_handlers
    @ List.filter_map
        ~f:Fn.id
        [
          Some Shape_field_check.handler;
          Some String_cast_check.handler;
          Some Tautology_check.handler;
          Some Enforceable_hint_check.handler;
          Some Const_write_check.handler;
          Some Switch_check.handler;
          Some Void_return_check.handler;
          Some Rvalue_check.handler;
          Some Callconv_check.handler;
          Some Xhp_check.handler;
          Some Discarded_awaitable_check.handler;
          Some Invalid_index_check.handler;
          Some Pseudofunctions_check.handler;
          Some Reified_check.handler;
          Some Instantiability_check.handler;
          Some Static_memoized_check.handler;
          Some Abstract_class_check.handler;
          (if skip_hierarchy_checks then
            None
          else
            Some Class_parent_check.handler);
          Some Method_type_param_check.handler;
          Some Obj_get_check.handler;
          Some This_hint_check.handler;
          Some Unresolved_type_variable_check.handler;
          Some Type_const_check.handler;
          Some Static_method_generics_check.handler;
          (if skip_hierarchy_checks then
            None
          else
            Some Class_inherited_member_case_check.handler);
          Some Ifc_tast_check.handler;
          Some Readonly_check.handler;
          Some Meth_caller_check.handler;
          Some Expression_tree_check.handler;
          (if skip_hierarchy_checks then
            None
          else
            Some Class_const_origin_check.handler);
          Some Enum_classes_check.handler;
          (if is_global_write_check_enabled then
            Some Global_write_check.handler
          else
            None);
          (if skip_hierarchy_checks then
            None
          else
            Some Enum_check.handler);
        ]
  in
  let handlers =
    if TypecheckerOptions.skip_tast_checks (Provider_context.get_tcopt ctx) then
      []
    else
      handlers
  in
  Tast_visitor.iter_with handlers

let program ctx = (visitor ctx)#go ctx

let def ctx = (visitor ctx)#go_def ctx

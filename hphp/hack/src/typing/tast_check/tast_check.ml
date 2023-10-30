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

(* Handlers that are enabled through 'log_levels' configuration. *)
let logger_handlers ctx =
  let tco = Provider_context.get_tcopt ctx in
  let log_levels = TypecheckerOptions.log_levels tco in
  let add_handler handlers (key, handler) =
    match SMap.find_opt key log_levels with
    | Some level when level > 0 -> handler ctx :: handlers
    | _ -> handlers
  in
  let key_handler_pairs =
    [
      ("tany", Tany_logger.create_handler);
      ("shape_analysis", Shape_analysis_logger.create_handler);
      ("sdt_analysis", Sdt_analysis_logger.create_handler);
      ("nothing_property", Nothing_property_logger.create_handler);
    ]
  in
  List.fold ~init:[] ~f:add_handler key_handler_pairs

let visitor ctx =
  (* Handlers that are not TAST checks to produce errors, but are used for
     telemetry that processes TASTs. *)
  let irregular_handlers = logger_handlers ctx in
  let tcopt = Provider_context.get_tcopt ctx in
  let hierarchy_check handler =
    if TypecheckerOptions.skip_hierarchy_checks tcopt then
      None
    else
      Some handler
  in
  let handlers =
    irregular_handlers
    @ List.filter_map
        ~f:Fn.id
        [
          Xhp_required_check.create_handler ctx;
          Redundant_generics_check.create_handler ctx;
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
          hierarchy_check Class_parent_check.handler;
          Some Method_type_param_check.handler;
          Some Obj_get_check.handler;
          Some This_hint_check.handler;
          Some Unresolved_type_variable_check.handler;
          Some Type_const_check.handler;
          Some Static_method_generics_check.handler;
          hierarchy_check Class_inherited_member_case_check.handler;
          Some Readonly_check.handler;
          Some Meth_caller_check.handler;
          Some Expression_tree_check.handler;
          hierarchy_check Class_const_origin_check.handler;
          (if TypecheckerOptions.global_access_check_enabled tcopt then
            Some Global_access_check.handler
          else
            None);
          hierarchy_check Enum_check.handler;
          (if TypecheckerOptions.populate_dead_unsafe_cast_heap tcopt then
            Some Remove_dead_unsafe_casts.patch_location_collection_handler
          else
            None);
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

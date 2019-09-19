(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Core_kernel

[@@@warning "+33"]

let visitor opts =
  let makers = [Xhp_required_check.make_handler] in
  let handlers = List.map makers (( |> ) opts) |> List.filter_opt in
  Tast_visitor.iter_with
    ( handlers
    @ [
        Shape_field_check.handler;
        String_cast_check.handler;
        Tautology_check.handler;
        Enforceable_hint_check.handler;
        Ppl_check.handler;
        Coroutine_check.handler;
        Redundant_nullsafe_check.handler;
        Const_write_check.handler;
        Switch_check.handler (fun t ->
            if TypecheckerOptions.disallow_scrutinee_case_value_type_mismatch t
            then
              Errors.invalid_switch_case_value_type
            else
              fun _ _ _ ->
            ());
        Void_return_check.handler;
        Rvalue_check.handler;
        Callconv_check.handler;
        Xhp_check.handler;
        Discarded_awaitable_check.handler;
        Invalid_index_check.handler;
        Basic_reactivity_check.handler;
        Pseudofunctions_check.handler;
        Reified_check.handler;
        Instantiability_check.handler;
        Static_memoized_check.handler;
        Abstract_class_check.handler;
        Type_params_arity_check.handler;
        Class_parent_check.handler;
        Method_type_param_check.handler;
        Foreach_collection_reactivity_check.handler;
        Obj_get_check.handler;
        This_hint_check.handler;
        Unresolved_type_variable_check.handler;
        Invalid_arraykey_constraint_check.handler;
        Type_const_check.handler;
      ] )

let program opts = (visitor opts)#go

let def opts = (visitor opts)#go_def

(**
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
let visitor = Tast_visitor.iter_with [
  Shape_field_check.handler;
  Sketchy_null_check.handler;
  String_cast_check.handler;
  Tautology_check.handler;
  Truthiness_test.handler;
  Type_test_hint_check.handler;
  Ppl_check.handler;
  Coroutine_check.handler;
  Redundant_nullsafe_check.handler;
  Switch_check.handler begin fun t ->
    if TypecheckerOptions.disallow_scrutinee_case_value_type_mismatch t
    then Errors.invalid_switch_case_value_type
    else fun _ _ _ -> ()
  end;
  Void_return_check.handler;
  Rvalue_check.handler;
  Callconv_check.handler;
  Xhp_check.handler;
  Discarded_awaitable_check.handler;
  Invalid_arraykey_check.handler;
  Invalid_arraykey_check.index_handler;
  Basic_reactivity_check.handler;
  Pseudofunctions_check.handler;
  Dynamic_method_call_check.handler;
  Reified_check.handler;
  Instantiability_check.handler;
  Static_memoized_check.handler;
  Abstract_class_check.handler;
  Type_params_arity_check.handler;
  Class_parent_check.handler;
]

let program = visitor#go
let def = visitor#go_def

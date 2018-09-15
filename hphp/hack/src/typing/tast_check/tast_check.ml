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
  Tautology_check.handler;
  Type_test_hint_check.handler;
  Ppl_check.handler;
  Coroutine_check.handler;
  Redundant_nullsafe_check.handler;
  Switch_check.handler @@ fun t ->
    if TypecheckerOptions.disallow_scrutinee_case_value_type_mismatch t
    then Errors.invalid_switch_case_value_type
    else fun _ _ _ -> ();
]

let program = visitor#go
let def = visitor#go_def

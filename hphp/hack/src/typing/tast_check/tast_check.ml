(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let visitor = Tast_visitor.iter_with [
  Shape_field_check.handler;
  Sketchy_null_check.handler;
  Tautology_check.handler;
  Type_test_hint_check.handler;
  Ppl_check.handler;
  Coroutine_check.handler;
  Redundant_nullsafe_check.handler;
]

let program = visitor#go
let def = visitor#go_def

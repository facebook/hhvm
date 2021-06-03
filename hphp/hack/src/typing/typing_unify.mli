(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val unify_param_modes :
  'a Typing_defs.fun_param ->
  'b Typing_defs.fun_param ->
  Errors.error_from_reasons_callback ->
  unit

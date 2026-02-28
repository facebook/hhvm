(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

val widen_for_array_get_ci :
  Typing_defs_constraints.can_index ->
  Typing_env_types.env ->
  locl_ty ->
  Typing_env_types.env * locl_ty option

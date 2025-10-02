(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type mismatch = {
  extra_names: string list;
  missing_names: string list;
  too_optional_names: string list;
}

val find_names_mismatch :
  actual_ft:'a Typing_defs_core.fun_type ->
  expected_ft:'b Typing_defs_core.fun_type ->
  mismatch

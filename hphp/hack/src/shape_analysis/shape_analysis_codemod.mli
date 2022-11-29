(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types

val group_of_results :
  error_count:int -> Typing_env_types.env -> shape_result list -> Hh_json.json

val codemods_of_entries :
  Typing_env_types.env ->
  solve:solve_entries ->
  atomic:bool (* TODO(T138659101): remove this option *) ->
  ConstraintEntry.t list ->
  Hh_json.json list

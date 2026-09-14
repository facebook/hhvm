(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Server_command_types.Find_refs

val go :
  action:action ->
  genv:Server_env.genv ->
  env:Server_env.env ->
  Server_env.env * server_result_or_retry

val go_for_single_file :
  ctx:Provider_context.t ->
  action:action ->
  naming_table:Naming_table.t ->
  filename:Relative_path.t ->
  server_result

val is_searchable : action:action -> bool

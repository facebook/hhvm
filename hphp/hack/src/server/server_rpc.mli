(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Might raise {!Naming_table.File_info_not_found} *)
val handle :
  'res.
  Server_env.genv ->
  Server_env.env ->
  is_stale:bool ->
  Server_command_types.cmd_metadata ->
  'res Server_command_types.t ->
  Server_env.env * 'res

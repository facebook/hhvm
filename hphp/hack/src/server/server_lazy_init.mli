(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Server_init_types

val full_init :
  Server_env.genv ->
  Server_env.env ->
  Cgroup_profiler.step_group ->
  Server_env.env * float

val parse_only_init :
  Server_env.genv ->
  Server_env.env ->
  Cgroup_profiler.step_group ->
  Server_env.env * float

val write_symbol_info_full_init :
  Server_env.genv ->
  Server_env.env ->
  Cgroup_profiler.step_group ->
  Server_env.env * float

(** if [index] is true, call Glean indexer after init, otherwise typechecks *)
val saved_state_init :
  do_indexing:bool ->
  load_state_approach:load_state_approach ->
  Server_env.genv ->
  Server_env.env ->
  Path.t ->
  Cgroup_profiler.step_group ->
  ( (Server_env.env * float) * (loaded_info * files_changed_while_parsing),
    load_state_error )
  result

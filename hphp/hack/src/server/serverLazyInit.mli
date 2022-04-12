(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open ServerInitTypes

val full_init :
  ServerEnv.genv ->
  ServerEnv.env ->
  CgroupProfiler.step_group ->
  ServerEnv.env * float

val parse_only_init :
  ServerEnv.genv ->
  ServerEnv.env ->
  CgroupProfiler.step_group ->
  ServerEnv.env * float

val write_symbol_info_full_init :
  ServerEnv.genv ->
  ServerEnv.env ->
  CgroupProfiler.step_group ->
  ServerEnv.env * float

(** if [index] is true, call Glean indexer after init, otherwise typechecks *)
val saved_state_init :
  do_indexing:bool ->
  load_state_approach:load_state_approach ->
  ServerEnv.genv ->
  ServerEnv.env ->
  Path.t ->
  CgroupProfiler.step_group ->
  ( (ServerEnv.env * float) * (loaded_info * files_changed_while_parsing),
    load_state_error )
  result

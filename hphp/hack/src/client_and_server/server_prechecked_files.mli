(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

val should_use : Server_args.options -> Server_local_config.t -> bool

val expand_all : Server_env.env -> Server_env.env

val init :
  Server_env.env ->
  dirty_local_deps:Typing_deps.DepSet.t ->
  dirty_master_deps:Typing_deps.DepSet.t ->
  Server_env.env

val update_after_recheck :
  Server_env.genv ->
  Server_env.env ->
  Relative_path.Set.t ->
  start_time:float ->
  Server_env.env * Telemetry.t

val update_after_local_changes :
  Server_env.genv ->
  Server_env.env ->
  Typing_deps.DepSet.t ->
  start_time:float ->
  Server_env.env * Telemetry.t

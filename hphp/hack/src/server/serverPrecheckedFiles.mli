(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

val should_use : ServerArgs.options -> ServerLocalConfig.t -> bool

val expand_all : ServerEnv.env -> ServerEnv.env

val init :
  ServerEnv.env ->
  dirty_local_deps:Typing_deps.DepSet.t ->
  dirty_master_deps:Typing_deps.DepSet.t ->
  ServerEnv.env

val update_after_recheck :
  ServerEnv.genv ->
  ServerEnv.env ->
  Relative_path.Set.t ->
  start_time:float ->
  ServerEnv.env * Telemetry.t

val update_after_local_changes :
  ServerEnv.genv ->
  ServerEnv.env ->
  Typing_deps.DepSet.t ->
  start_time:float ->
  ServerEnv.env * Telemetry.t

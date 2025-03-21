(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val make_genv :
  ServerArgs.options ->
  ServerConfig.t ->
  ServerLocalConfig.t ->
  MultiWorker.worker list ->
  ServerEnv.genv

val default_genv : ServerEnv.genv

val make_env :
  init_id:string ->
  deps_mode:Typing_deps_mode.t ->
  ?errorl:Errors.t ->
  ServerConfig.t ->
  ServerEnv.env

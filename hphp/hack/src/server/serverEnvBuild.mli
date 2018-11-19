(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val make_genv:
  ServerArgs.options ->
  ServerConfig.t ->
  ServerLocalConfig.t ->
  SharedMem.handle ->
  logging_init:(unit -> unit) ->
    ServerEnv.genv

val default_genv: ServerEnv.genv

val make_env: ServerConfig.t -> ServerEnv.env

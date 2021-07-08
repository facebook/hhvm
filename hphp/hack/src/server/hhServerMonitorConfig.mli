(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type pipe_type =
  | Default
  | Priority
  | Force_dormant_start_only

val pipe_type_to_string : pipe_type -> string

module HhServerConfig :
  ServerMonitorUtils.Server_config
    with type server_start_options = ServerArgs.options

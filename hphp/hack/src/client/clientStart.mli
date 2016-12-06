(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type env = {
  root: Path.t;
  no_load: bool;
  silent: bool;
  ai_mode: string option;
  debug_port: Debug_port.out_port option;
}

val main : env -> Exit_status.t
val start_server : env -> unit

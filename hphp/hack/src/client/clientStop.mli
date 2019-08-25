(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type env = {
  root: Path.t;
  from: string;
}

val kill_server : Path.t -> string -> unit

val main : env -> Exit_status.t Lwt.t

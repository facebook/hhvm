(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Called whenever the server is idle *)
val go : ServerEnv.env -> ServerEnv.env

val async : (env:ServerEnv.env -> ServerEnv.env) -> unit

(** Record timestamp of client connections *)
val stamp_connection : unit -> unit

val init : ServerEnv.genv -> Path.t -> unit

(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

 open ServerEnv

(* Called whenever the server is idle *)
val go: unit -> unit

val async: (unit -> unit) -> unit

(* Called every time a client connects *)
val stamp_connection: unit -> unit

val init: genv -> Path.t -> unit

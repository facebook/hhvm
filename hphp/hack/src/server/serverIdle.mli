(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This module contains the code to be run whenever the server is idle, e.g.
    garbage collection. It manages a set of callbacks to be run periodically
    whenever the server is idle. *)

(** Initialize with a set of default periodic callbacks, for example
    garbage collection, log flushing, etc. *)
val init : ServerEnv.genv -> Path.t -> unit

(** Register the provided function as a callback to be run next time the server
    is idle. *)
val async : (env:ServerEnv.env -> ServerEnv.env) -> unit

(** Called whenever the server is idle. Will run any due callbacks. *)
val go : ServerEnv.env -> ServerEnv.env

(** Record timestamp of client connections *)
val stamp_connection : unit -> unit

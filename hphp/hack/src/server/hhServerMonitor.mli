(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val start : unit -> unit

(** Start a server daemon with these options and exits.
 * Warning: use carefully. options.should_detach must be set to true
 * for this to work properly. *)
val start_daemon : ServerArgs.options -> proc_stack:string list -> Exit_status.t

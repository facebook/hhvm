(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This exception might be thrown in code which executes in MultiWorker
 * workers. If you happen to catch it, the best course of action is to
 * re-throw it to guarantee speedy cancellation.
 *)
exception Worker_should_exit

val is_stop_requested : unit -> bool

val raise_if_stop_requested : unit -> unit

val stop_workers : unit -> unit

val resume_workers : unit -> unit

val with_no_cancellations : (unit -> 'a) -> 'a

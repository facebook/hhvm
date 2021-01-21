(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception Worker_should_exit

val stop_workers : unit -> unit

val resume_workers : unit -> unit

val with_no_cancellations : (unit -> 'a) -> 'a

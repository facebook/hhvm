(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* Module saving/loading a pre-populated cache *)
(*****************************************************************************)
open Utils

(* The files with an error *)
type failed = SSet.t

(* The files with a parsing error *)
type failed_parsing = SSet.t

type filename = string

val filename: unit -> string

val save: ServerEnv.env -> failed_parsing:failed_parsing -> failed:failed 
    -> filename -> unit

val load: Worker.t list -> filename -> ServerEnv.env * failed_parsing * failed

val is_ready: unit -> bool

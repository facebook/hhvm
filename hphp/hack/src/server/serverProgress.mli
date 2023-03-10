(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

val send_warning : string option -> unit

(* This is basically signature of "Printf.printf" *)
val send_progress :
  ?include_in_logs:bool -> ('a, unit, string, unit) format4 -> 'a

val send_percentage_progress :
  operation:string ->
  done_count:int ->
  total_count:int ->
  unit:string ->
  extra:string option ->
  unit

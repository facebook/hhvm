(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

(** These are human-readable messages, shown at command-line and within the editor. *)
type t = {
  message: string;  (** e.g. "typechecking 5/15 files" *)
  timestamp: float;
}

(** All other functions will throw until you've set root *)
val set_root : Path.t -> unit

(** use this in tests, instead of set_root, to disable progress-logging *)
val disable : unit -> unit

val read : unit -> t

(* This is basically signature of "Printf.printf" *)
val write : ?include_in_logs:bool -> ('a, unit, string, unit) format4 -> 'a

val write_percentage :
  operation:string ->
  done_count:int ->
  total_count:int ->
  unit:string ->
  extra:string option ->
  unit

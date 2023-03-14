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
  server_progress: string;  (** e.g. "typechecking 5/15 files" *)
  server_warning: string option;  (** e.g. "typechecking will be slow" *)
  server_timestamp: float;
}

(** All other functions will throw until you've set root *)
val set_root : Path.t -> unit

(** use this in tests, instead of set_root, to disable progress-logging *)
val disable : unit -> unit

val write : t -> unit

val read : unit -> t

(* TODO(ljw): we no longer need warnings. The following three functions can be rationalized. *)

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

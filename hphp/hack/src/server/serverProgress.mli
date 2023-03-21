(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

(** All functions in this file will throw unless you've already called set_root
at the start of your process. *)
val set_root : Path.t -> unit

(** use this in tests, instead of set_root,
to disable progress-logging and error-streaming. *)
val disable : unit -> unit

(** Progress is a file in /tmp/hh_server/<repo>.progress.json which is written
by monitor+server. It lives from the moment the monitor starts up until the
moment it finally dies or is killed. You should only read it by the `read`
call, since that protects against races. Anyone at any time can read this
file to learn the current state. The state is represented solely as a
human-readable string to be shown to the user in the CLI or VSCode status bar.
It specifically shouldn't be acted upon in code -- it's slightly handwavey
in places (e.g. there's an interval from when a server dies until the monitor
realizes that fact where attempting to read will say "unknown"). *)
type t = {
  pid: int;
  message: string;  (** e.g. "typechecking 5/15 files" *)
  timestamp: float;
}

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

(** Call this upon monitor shutdown to delete the progress file *)
val try_delete : unit -> unit

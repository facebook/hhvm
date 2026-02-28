(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type watch_error =
  | Killed of Exit_status.finale_data option
      (** Hh_server was killed so we can't read errors. *)
  | Read_error of Server_progress.errors_file_read_error
[@@deriving show]

val watch_error_short_description : watch_error -> string

(** This function kicks off a long-running task which watches the errors.bin file that the
caller has already opened, and has already called openfile upon.
It returns a Lwt_stream.t as follows:
* The stream provides errors, telemetry and completion sentinels written to the file
* If the producing PID gets killed without a clean finish, then that too will
  be detected and reported with "Error Killed" in the stream, albeit not quite so soon.
  (It only performs this inter-process polling every 5s).
* There's no way to cancel the long-running task.
* The stream should terminate (i.e. return None) iff the previous message
  was a completion sentinel or an error. *)
val watch_errors_file :
  pid:int ->
  Unix.file_descr ->
  (Server_progress.ErrorsRead.read_result, watch_error) result Lwt_stream.t

(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This function kicks off a long-running task which watches the errors.bin file that the
caller has already opened, and has already called openfile upon.
It returns a Lwt_stream.t as follows.
* Shortly after a new set of errors has been reported to the file,
  "Ok errors" will be placed in the stream.
* Shortly after the file is finished cleanly (completed, restarted, killed),
  "Error" will be placed in the stream and the stream will be closed
  and the long-running task will finish.
* If the producing PID gets killed without a clean finish, then that too will
  be detected and reported with "Error Killed" in the stream, albeit not quite so soon.
  (It only performs this inter-process polling every 5s).
* There's no way to cancel the long-running task. *)
val watch_errors_file :
  pid:int ->
  Unix.file_descr ->
  Server_progress.ErrorsRead.read_result Lwt_stream.t

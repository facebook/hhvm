(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *)

(** Use this instead of [Lwt_main.run] to ensure that the right engine is set, and to
    improve stack traces. *)
val run_main : (unit -> 'a Lwt.t) -> 'a

(** Drop-in replacement for [Unix.select] that works even when the Lwt main loop
is running (i.e. your function has [Lwt_main.run] somewhere higher up in the
call stack).

The Lwt main loop is an event loop pumped by [Unix.select], and so regular
[Unix.select] calls are prone to raising `EINTR`. The implementation of this
function does not use [Unix.select] at all, but Lwt primitives that accomplish
the same thing.
*)
val select :
  Unix.file_descr list ->
  Unix.file_descr list ->
  Unix.file_descr list ->
  float ->
  (Unix.file_descr list * Unix.file_descr list * Unix.file_descr list) Lwt.t

module Process_success : sig
  type t = {
    command_line: string;
    stdout: string;
    stderr: string;
    start_time: float;
    end_time: float;
  }
end

module Process_failure : sig
  type t = {
    command_line: string;
    process_status: Unix.process_status;
    stdout: string;
    stderr: string;
    exn: exn option;
    start_time: float;
    end_time: float;
  }

  (** Human-readable multi-line fairly verbose explanation of the failure,
  aimed for logging and hack developers, not for end users *)
  val to_string : t -> string
end

(** Run a command with a given input and return the output. If the command exits
with an exit status other than zero, returns [Process_failure] instead.

There are several ways for it to be cancelled:
* [cancel] parameter
  e.g. `let (cancel, u) = Lwt.bind(); Lwt.wakeup_later u (); exec_checked ~cancel`
  Upon the cancel parameter being completed, we will send a SIGKILL to the underlying
  process. In Unix, this cannot be ignored nor blocked. It will result in
  Process_failure.status=WSIGNALED Sys.sigkill, and .stdout/.stderr will contain
  whatever the process had sent before that.
* [timeout] parameter
  e.g. `exec_checked ~timeout:30.0`
  This parameter is passed straight through to Lwt_process.open_process_full.
  Its behavior is to aggressively closes stdout/stderr pipes and also send a SIGKILL.
  It will result in Process_failure.status=WSIGNALED Sys.sigkill, and .stdout/.stderr
  will be empty because of the closed pipes, and .exn will likely be something about
  those closed pipes too.
* Lwt.cancel
  e.g. `Lwt.pick [exec_checked ...; Lwt.return_none]`.
  This has been deprecated as a cancellation mechanism by the Lwt folks.
  This will *NOT* kill the underlying process. It will close the pipes.
  As to what happens to the promise? you shouldn't really even be asking,
  and indeed varies depending on the process. *)
val exec_checked :
  ?input:string ->
  ?env:string array ->
  ?timeout:float ->
  ?cancel:unit Lwt.t ->
  Exec_command.t ->
  string array ->
  (Process_success.t, Process_failure.t) Lwt_result.t

(** Asynchronous version of [Utils.try_finally]. Run and wait for [f] to
complete, and be sure to invoke [finally] asynchronously afterward, even if [f]
raises an exception. *)
val try_finally :
  f:(unit -> 'a Lwt.t) -> finally:(unit -> unit Lwt.t) -> 'a Lwt.t

(** Asynchronous version of [Sys_utils.with_lock]. Locks the entire file
using [Lwt_utils.lockf], and runs the callback. *)
val with_lock :
  Lwt_unix.file_descr ->
  Lwt_unix.lock_command ->
  f:(unit -> 'a Lwt.t) ->
  'a Lwt.t

(** Asynchronous version of [Utils.with_context]. Call [enter], then run and
wait for [do_] to complete, and finally call [exit], even if [f] raises an exception. *)
val with_context :
  enter:(unit -> unit Lwt.t) ->
  exit:(unit -> unit Lwt.t) ->
  do_:(unit -> 'a Lwt.t) ->
  'a Lwt.t

(** Reads all the contents from the given file on disk, or returns an error
message if unable to do so. *)
val read_all : string -> (string, string) Lwt_result.t

module Promise : Promise.S with type 'a t = 'a Lwt.t

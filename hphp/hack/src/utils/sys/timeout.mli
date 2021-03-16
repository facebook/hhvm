(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Helpers for handling timeout, in particular input timeout. *)

type timings = {
  start_time: float;
  deadline_time: float;  (** caller-supplied deadline *)
  timeout_time: float;  (** actual time, after deadline, when we fired *)
}
[@@deriving show]

type t

(** The function `with_timeout` executes 'do_' for at most 'timeout'
    seconds. If the `timeout` is reached, the `on_timeout` is executed
    if available, otherwise the `Timeout` exception is raised.

    On Unix platform, this function is based on interval timer ITIMER_REAL
    which sends SIGALRM upon completion, and setting a signal handler for SIGALRM.
    On Windows platform, this is based on the equivalent of `select`.
    Hence, this module exports variant of basic input functions, adding
    them a `timeout` parameter. It should correspond to the parameter of the
    `do_` function.

    For `do_` function based only on computation (and not I/O), you
    should call the `check_timeout` function on a regular
    basis. Otherwise, on Windows, the timeout will never be detected.
    On Unix, the function `check_timeout` is no-op.

    On Unix, the type `in_channel` is in fact an alias for
    `Stdlib.in_channel`. *)
val with_timeout :
  timeout:int -> on_timeout:(timings -> 'a) -> do_:(t -> 'a) -> 'a

val check_timeout : t -> unit

type in_channel

val open_in : string -> in_channel

val close_in : in_channel -> unit

val close_in_noerr : in_channel -> unit

val in_channel_of_descr : Unix.file_descr -> in_channel

val descr_of_in_channel : in_channel -> Unix.file_descr

(** Selects ready file descriptor. Based on Unix.select in Unix.
    [timeout] is only there to achieve a similar effect as Unix interval
    timers on Windows, but is ignored on Unix. On Windows,
    [select ~timeout read write exn select_timeout] runs with a timeout
    that is the minimum of [timeout and select_timeout].*)
val select :
  ?timeout:t ->
  Unix.file_descr list ->
  Unix.file_descr list ->
  Unix.file_descr list ->
  float ->
  Unix.file_descr list * Unix.file_descr list * Unix.file_descr list

val input : ?timeout:t -> in_channel -> Bytes.t -> int -> int -> int

val really_input : ?timeout:t -> in_channel -> Bytes.t -> int -> int -> unit

val input_char : ?timeout:t -> in_channel -> char

val input_line : ?timeout:t -> in_channel -> string

val input_value : ?timeout:t -> in_channel -> 'a

val open_process : Exec_command.t -> string array -> in_channel * out_channel

val open_process_in : Exec_command.t -> string array -> in_channel

val close_process_in : in_channel -> Unix.process_status

val read_process :
  timeout:int ->
  on_timeout:(timings -> 'a) ->
  reader:(t -> in_channel -> out_channel -> 'a) ->
  Exec_command.t ->
  string array ->
  'a

val open_connection : ?timeout:t -> Unix.sockaddr -> in_channel * out_channel

val read_connection :
  timeout:int ->
  on_timeout:(timings -> 'a) ->
  reader:(t -> in_channel -> out_channel -> 'a) ->
  Unix.sockaddr ->
  'a

val shutdown_connection : in_channel -> unit

(* Some silly people like to catch all exceptions. This means they need to explicitly detect and
 * reraise the timeout exn. *)
val is_timeout_exn : t -> exn -> bool

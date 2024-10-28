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

type in_channel = Stdlib.in_channel * int option

val open_process :
  Exec_command.t ->
  string array ->
  (Stdlib.in_channel * int option) * out_channel

val open_process_in :
  Exec_command.t -> string array -> Stdlib.in_channel * int option

val close_process_in : in_channel -> Unix.process_status

val read_process :
  timeout:int ->
  on_timeout:(timings -> 'a) ->
  reader:(t -> in_channel -> Stdlib.out_channel -> 'a) ->
  Exec_command.t ->
  string array ->
  'a

(* Some silly people like to catch all exceptions. This means they need to explicitly detect and
 * reraise the timeout exn. *)
val is_timeout_exn : t -> exn -> bool

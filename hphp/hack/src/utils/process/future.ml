(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * We shell out hg commands, so its computation is out-of-process, and
 * return a Future for the result.
 *
 * A promise is a strongly-typed wrapper around Process
 *)

type 'a promise =
  | Complete : 'a -> 'a promise
    (** Delayed is useful for testing. Must be tapped by "is ready"
     * the given number of times before it is ready. *)
  | Delayed : (int * 'a) -> 'a promise
  | Incomplete of Process_types.t * (string -> 'a)
type 'a t = 'a promise ref

let make process transformer =
  ref (Incomplete (process, transformer))

let of_value v = ref @@ Complete v

let delayed_value ~delays v =
  ref (Delayed (delays, v))

let get : 'a t -> 'a =
  let open Process_types in
  fun promise -> match !promise with
  | Complete v -> v
  | Delayed (i, v) when i <= 0 ->
    v
  | Delayed _ ->
    raise (Future_sig.Timed_out "Delayed value not ready yet")
  | Incomplete (process, transformer) ->
    match Process.read_and_wait_pid ~timeout:30 process with
    | Result.Ok (stdout, _stderr) ->
      let result = transformer stdout in
      let () = promise := Complete result in
      result
    | Result.Error (Process_exited_abnormally
        (status, _, stderr)) ->
      (** TODO: Prefer monad over exceptions. *)
      raise (Future_sig.Process_failure (status, stderr))
    | Result.Error (Timed_out (_, stderr)) ->
      (** TODO: Prefer monad over exceptions. *)
      raise (Future_sig.Timed_out stderr)
    | Result.Error Process_aborted_input_too_large ->
      raise Future_sig.Process_aborted

let is_ready promise = match !promise with
  | Complete _ -> true
  | Delayed (i, _) when i <= 0 -> true
  | Delayed (i, v) ->
    promise := Delayed(i - 1, v);
    false
  | Incomplete (process, _) ->
    Process.is_ready process

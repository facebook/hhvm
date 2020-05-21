(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Deserializes the byte sequence. *)
type 'a deserializer = string -> 'a

module Types = struct
  type process_status = Unix.process_status

  let pp_process_status fmt status =
    Caml.Format.fprintf fmt "%s" (Process.status_to_string status)

  type error_mode =
    | Process_failure of {
        status: process_status;
        stderr: string;
      }
    | Timed_out of {
        stdout: string;
        stderr: string;
      }
    | Process_aborted
    | Continuation_raised of Exception.t
    | Transformer_raised of Exception.t
  [@@deriving show]

  type error = Process_types.invocation_info * error_mode [@@deriving show]

  type verbose_error = {
    message: string;
    stack: Utils.callstack;
    environment: string option;
  }
  [@@deriving show]

  type 'a status =
    | Complete_with_result of ('a, error) result
    | In_progress of { age: float }

  exception Failure of error
end

module type S = sig
  include module type of Types

  type 'a t [@@deriving eq]

  (* Blocking. Returns the value from the underlying process. *)
  val get : ?timeout:int -> 'a t -> ('a, error) result

  (* Like get, but raises Failure instead of returning when result is Error. *)
  val get_exn : ?timeout:int -> 'a t -> 'a

  (* Creates a future out of the process handle. If the timeout is specified,
      then this timeout will take priority over the timeout that's passed
      into the `get` function, or whichever is less. If the timeout is not
      specified, then it's assumed to be infinite for the purposes of how
      `is_ready` and `check_status` work, and for the purpose of figuring out
      `get`'s timeout.

     Example 1:
        - the future is made without a timeout
        - the future is synchronously gotten with a timeout of 15 seconds
        - if the process is not ready after 15 seconds, the result is a timeout

     Example 2:
        - the future is made with a timeout of 5 seconds
        - the future is synchronously gotten with a timeout of 10 seconds
        - the timeout that applies is the 5 second `make` timeout because its
            priority is higher than the priority of the `get` timeout

     Example 3:
        - the future is made with a timeout of 10 seconds
        - the future is synchronously gotten with a timeout of 5 seconds
        - the timeout that applies is the 5 second `get` timeout because its
            value is less than the value of the `make` timeout

    NOTE: the timeout affects the behavior of `is_ready` and `check_status`:
        if set here and if it is expired, `is_ready` will return true and
        `check_status` will return a "timed out" result
    *)
  val make :
    Process_types.t -> ?timeout:int -> 'result deserializer -> 'result t

  (* Sets or resets the timeout on an existing future. The meaning of
      setting a timeout on a merged or a bound future is thus:
        - for a merged future, we set the timeout recursively on all the futures
        - for a bound future, we set the timeout on the first future, but we
        can't do anything reasonable for the bound continuation - if that
        continuation produces a true future, this timeout will not be applied
        to it.

     This is a convenience function meant to enable setting timeouts on futures
     produced by other modules. Normally, you would create a future with
     a timeout by specifying the timeout when invoking the `make` function, but
     if you've already gotten a future from, e.g., another module, then it might
     be convenient to set the timeout separately, instead of threading
     the timeout into the other module's APIs.
    *)
  val with_timeout : 'result t -> timeout:int -> 'result t

  (* Analogous to "make" above, but takes in two futures and a function that
   * consumes their results, producing a third future that "is_ready" when both
   * of the underlying are ready (and will block on "get" until both of the
   * underlying are completed).
   *
   * NB: The handler is run each time "get" is called on the Future. *)
  val merge :
    'a t ->
    'b t ->
    (('a, error) result -> ('b, error) result -> ('c, error) result) ->
    'c t

  (* Adds a computation that will be applied to the result of the future when
   * it is finished. *)
  val continue_with : 'a t -> ('a -> 'b) -> 'b t

  (* Adds another future to be generated after the given future finishes. *)
  val continue_with_future : 'a t -> ('a -> 'b t) -> 'b t

  (* Adds another future to be generated after the given future finishes, but
  * allows custom handling of process errors. *)
  val continue_and_map_err :
    'a t -> (('a, error) result -> ('b, 'c) result) -> ('b, 'c) result t

  val on_error : 'value t -> (error -> unit) -> 'value t

  (* Wraps a value inside a future. *)
  val of_value : 'a -> 'a t

  (* Wraps an error with the specified string message inside a future *)
  val of_error : string -> 'a t

  (* Like of_value, except returns false "delays" number of times of
   * calling is_ready on it before returning true. *)
  val delayed_value : delays:int -> 'a -> 'a t

  (* Checks whether the future is ready or not by checking the underlying
      implementation (e.g., Process) whether it's ready. The meaning of
      "ready" is that calling the `get` function will not block. The `is_ready`
      check will also return true if getting the future failed or timed out.
      The latter can happen if the future was created with a timeout:
      `is_ready` will return true if the timeout is expired, even if
      the underlying implementation is still working. Calling `get` afterwards
      will result in a timeout failure. *)
  val is_ready : 'a t -> bool

  (* Think of this as a combination of `is_ready` and `get`:
    - if ready, returns the result
    - if not ready, then returns the in-progress status
    Note that, if the future was created with a timeout, it will return
    a timed out result if the timeout is expired *)
  val check_status : 'a t -> 'a status

  (* Return the timestamp the future was constructed. For Merged futures,
   * returns the older of the merged futures. *)
  val start_t : 'a t -> float

  val error_to_string : error -> string

  val error_to_string_verbose : error -> verbose_error

  val error_to_exn : error -> exn
end

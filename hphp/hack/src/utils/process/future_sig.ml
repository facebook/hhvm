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
  type error_mode =
    | Process_failure of {
        status: Unix.process_status;
        stderr: string;
      }
    | Timed_out of {
        stdout: string;
        stderr: string;
      }
    | Process_aborted
    | Continuation_raised of Exception.t
    | Transformer_raised of Exception.t

  type error = Process_types.invocation_info * error_mode

  type 'a status =
    | Complete_with_result of ('a, error) result
    | In_progress of { age: float }

  exception Failure of error
end

module type S = sig
  include module type of Types

  type 'a t

  (* Blocking. Returns the value from the underlying process. *)
  val get : ?timeout:int -> 'a t -> ('a, error) result

  (* Like get, but raises Failure instead of returning when result is Error. *)
  val get_exn : ?timeout:int -> 'a t -> 'a

  val make : Process_types.t -> 'a deserializer -> 'a t

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

  (* Just wrap a value inside a future. *)
  val of_value : 'a -> 'a t

  val of_error : string -> 'a t

  (* Like of_value, except returns false "delays" number of times of
   * calling is_ready on it before returning true. *)
  val delayed_value : delays:int -> 'a -> 'a t

  (* Returns true if "get" will not block. *)
  val is_ready : 'a t -> bool

  val check_status : 'a t -> 'a status

  (* Return the timestamp the future was constructed. For Merged futures,
   * returns the older of the merged futures. *)
  val start_t : 'a t -> float

  val error_to_string : error -> string

  val error_to_string_verbose : error -> string * Utils.callstack

  val error_to_exn : error -> exn
end

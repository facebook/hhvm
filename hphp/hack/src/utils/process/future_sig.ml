(**
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
  type stdout = string
  type stderr = string
  type error_mode =
    | Process_failure of Unix.process_status * stderr
    (** string is stderr output received so far. *)
    | Timed_out of stdout * stderr
    | Process_aborted
    | Transformer_raised of exn

  type error = Process_types.info * error_mode

  type age = (** milliseconds *) float

  type 'a status =
    | Complete_with_result of ('a, error) result
    | In_progress of age

  exception Failure of error

end

module type S = sig
  include module type of Types
  type 'a t
  (** Blocking. Returns the value from the underlying process. *)
  val get : ?timeout:int -> 'a t -> ('a, error) result

  (** Like get, but raises Failure instead of returning when result is Error. *)
  val get_exn : ?timeout:int -> 'a t -> 'a

  val make : Process_types.t -> 'a deserializer -> 'a t

  (** Analogous to "make" above, but takes in two futures and a function that
   * consumes their results, producing a third future that "is_ready" when both
   * of the underlying are ready (and will block on "get" until both of the
   * underlying are completed).
   *
   * NB: The handler is run each time "get" is called on the Future. *)
  val merge : 'a t -> 'b t ->
    (('a, error) result -> ('b, error) result -> ('c, error) result) -> 'c t

  (** Just wrap a value inside a future. *)
  val of_value : 'a -> 'a t

  (** Like of_value, except returns false "delays" number of times of
   * calling is_ready on it before returning true. *)
  val delayed_value : delays:int -> 'a -> 'a t

  (** Returns true if "get" will not block. *)
  val is_ready : 'a t -> bool

  val check_status : 'a t -> 'a status

  (** Return the timestamp the future was constructed. For Merged futures,
   * returns the older of the merged futures. *)
  val start_t : 'a t -> float

  val error_to_string : error -> string
  val error_to_exn : error -> exn
end

(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(** Deserializes the byte sequence. *)
type 'a deserializer = string -> 'a

module Types = struct
  type stdout = string
  type stderr = string
  type error =
    | Process_failure of Unix.process_status * stderr
    (** string is stderr output received so far. *)
    | Timed_out of stdout * stderr
    | Process_aborted
    | Transformer_raised of exn

  exception Failure of error

end

module type S = sig
  include module type of Types
  type 'a t
  (** Blocking. Returns the value from the underlying process. *)
  val get : 'a t -> ('a, error) result

  (** Like get, but raises Failure instead of returning when result is Error. *)
  val get_exn : 'a t -> 'a

  val make : Process_types.t -> 'a deserializer -> 'a t

  (** Just wrap a value inside a future. *)
  val of_value : 'a -> 'a t

  (** Like of_value, except returns false "delays" number of times of
   * calling is_ready on it before returning true. *)
  val delayed_value : delays:int -> 'a -> 'a t

  (** Returns true if "get" will not block. *)
  val is_ready : 'a t -> bool

  val error_to_string : error -> string
  val error_to_exn : error -> exn
end

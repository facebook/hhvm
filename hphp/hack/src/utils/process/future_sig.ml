(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(** Fully-defined types go outside the module type. *)
exception Process_failure of Unix.process_status * (** Stderr *) string

(** string is stderr output received so far. *)
exception Timed_out of string

exception Process_aborted

(** Deserializes the byte sequence. *)
type 'a deserializer = string -> 'a

module type S = sig
  type 'a t
  (** Blocking. Returns the value from the underlying process. *)
  val get : 'a t -> 'a
  val make : Process_types.t -> 'a deserializer -> 'a t

  (** Just wrap a value inside a future. *)
  val of_value : 'a -> 'a t

  (** Like of_value, except returns false "delays" number of times of
   * calling is_ready on it before returning true. *)
  val delayed_value : delays:int -> 'a -> 'a t

  (** Returns true if "get" will not block. *)
  val is_ready : 'a t -> bool
end

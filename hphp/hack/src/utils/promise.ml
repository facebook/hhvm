(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type S = sig
  type 'value t

  (** Creates a promise that returns the given value immediately. *)
  val return : 'value -> 'value t

  (** Returns a new promise that will map the result of the given one. *)
  val map : 'value t -> ('value -> 'next_value) -> 'next_value t

  (** Returns a new promise generated from the results of the given one. *)
  val bind : 'value t -> ('value -> 'next_value t) -> 'next_value t

  val both : 'a t -> 'b t -> ('a * 'b) t
end

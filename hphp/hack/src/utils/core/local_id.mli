(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** Used to represent local variables in the named AST. *)

module S : sig
  type t [@@deriving eq]

  val compare : t -> t -> int
end

type t = S.t [@@deriving eq, hash]

val pp : Format.formatter -> t -> unit

val compare : t -> t -> int

val to_string : t -> string

val to_int : t -> int

val get_name : t -> string

(** Is this local variable something a user could write, or an
    internally generated variable?

    Yes: $foo, $_bar1, $$
    No: $#foo, $0bar *)
val is_user_denotable : t -> bool

(** Make an id for a scoped variable. Return a fresh id every time.
This is used to enforce that two locals with the same name but with
different scopes have different ids. *)
val make_scoped : string -> t

(** Make an id for an unscoped variable. Two calls with the same input
 * string will return the same id. *)
val make_unscoped : string -> t

val make : int -> string -> t

val tmp : unit -> t

module Set : module type of Stdlib.Set.Make (S)

module Map : sig
  include module type of WrappedMap.Make (S)

  val pp : (Format.formatter -> 'a -> unit) -> Format.formatter -> 'a t -> unit

  val show : (Format.formatter -> 'a -> unit) -> 'a t -> string

  val hash_fold_t :
    (Hash.state -> 'a -> Hash.state) -> Hash.state -> 'a t -> Hash.state
end

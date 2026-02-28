(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t [@@deriving eq, hash, ord]

type provider

val make : provider -> t

val make_provider : unit -> provider

val make_immutable : t -> t

val is_immutable : t -> bool

module Map : WrappedMap_sig.S with type key = t

val display : t -> string

(** Shows the internal value if the ID. Only use for debugging *)
val debug : t -> string

val pp : Format.formatter -> t -> unit

(** You probably should get your types straight before using this.
    Hint: don't use string mingling. *)
val dodgy_from_int : int -> t

(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Reordered_argument_collections

type prefix =
  | Root
  | Hhi
  | Dummy
  | Tmp
[@@deriving eq, hash, show, enum]

val is_hhi : prefix -> bool

val is_root : prefix -> bool

val set_path_prefix : prefix -> Path.t -> unit

val path_of_prefix : prefix -> string

module S : sig
  type t

  val compare : t -> t -> int

  val equal : t -> t -> bool

  val hash : t -> int

  val to_string : t -> string
end

type t = S.t [@@deriving eq, hash, show, ord, sexp_of]

val default : t

(** Checks that the provided string indeed has the given prefix before constructing path *)
val create : prefix -> string -> t

(** Creates a new path, inferring the prefix. Will default to Dummy. *)
val create_detect_prefix : string -> t

(** Creates a Relative_path.t relative to the root. The argument must be
    a *relative* path (the path suffix). If you wish to construct
    a Relative_path.t from an absolute path, use
    `create_detect_prefix` instead. *)
val from_root : suffix:string -> t

val prefix : t -> prefix

val suffix : t -> string

val to_absolute : t -> string

val to_absolute_with_prefix : www:Path.t -> hhi:Path.t -> t -> string

val to_tmp : t -> t

val to_root : t -> t

val strip_root_if_possible : string -> string option

module Set : sig
  include module type of Reordered_argument_set (Set.Make (S))

  val pp : Format.formatter -> t -> unit

  val show : t -> string

  val pp_large : ?max_items:int -> Format.formatter -> t -> unit

  val show_large : ?max_items:int -> t -> string
end

module Map : sig
  include module type of Reordered_argument_map (WrappedMap.Make (S))

  val pp : (Format.formatter -> 'a -> unit) -> Format.formatter -> 'a t -> unit

  val show : (Format.formatter -> 'a -> unit) -> 'a t -> string

  val yojson_of_t : ('a -> Yojson.Safe.t) -> 'a t -> Yojson.Safe.t
end

val relativize_set : prefix -> SSet.t -> Set.t

val set_of_list : t list -> Set.t

val storage_to_string : t -> string

val storage_of_string : string -> t

(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

module type Intra = sig
  type constraint_ [@@deriving eq, hash]

  (** for observability only: no behavior should depend on what this function does *)
  val debug_show_constraint_ : constraint_ -> string
end

module type T = sig
  type intra_constraint_

  type id =
    | ClassLike of string
    | Function of string
  [@@deriving ord, show { with_path = false }]

  type inter_constraint_ = Inherits of id
  [@@deriving ord, show { with_path = false }]

  module Read : sig
    type t

    val get_inters : t -> id -> inter_constraint_ Sequence.t

    val get_intras : t -> id -> intra_constraint_ Sequence.t

    val get_keys : t -> id Sequence.t
  end

  module Write : sig
    type t

    (** enables flushing to disk. You may not need this, since flushing happens automatically `at_exit` *)
    type flush = unit -> unit

    val create : db_dir:string -> worker_id:int -> flush * t

    val add_id : t -> id -> unit

    val add_inter : t -> id -> inter_constraint_ -> unit

    val add_intra : t -> id -> intra_constraint_ -> unit
  end

  (** read the constraints provided with `add_inter` and `add_intra`, without modification *)
  val debug_dump : db_dir:string -> Read.t

  val solve : db_dir:string -> Read.t
end

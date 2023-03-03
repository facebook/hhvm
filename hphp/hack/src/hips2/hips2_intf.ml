(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

module type Intra = sig
  type constraint_ [@@deriving eq, hash, show]

  type custom_inter_constraint_ [@@deriving eq, hash, ord, show]
end

module type T = sig
  type intra_constraint_

  type custom_inter_constraint_ [@@deriving eq, hash, ord, show]

  module Id : sig
    type t =
      | ClassLike of string
      | Function of string
    [@@deriving ord, show { with_path = false }]
  end

  type inter_constraint_ =
    | Inherits of Id.t
        (** Interpreted broadly: extends+implements+trait require *)
    | CustomInterConstraint of custom_inter_constraint_
  [@@deriving ord, show { with_path = false }]

  module Read : sig
    type t

    val get_inters : t -> Id.t -> inter_constraint_ Sequence.t

    val get_intras : t -> Id.t -> intra_constraint_ Sequence.t

    val get_keys : t -> Id.t Sequence.t
  end

  module Write : sig
    type t

    (** enables flushing to disk. You may not need this, since flushing happens automatically `at_exit` *)
    type flush = unit -> unit

    val create : db_dir:string -> worker_id:int -> flush * t

    val add_id : t -> Id.t -> unit

    val add_inter : t -> Id.t -> inter_constraint_ -> unit

    val add_intra : t -> Id.t -> intra_constraint_ -> unit
  end

  (** read the constraints provided with `add_inter` and `add_intra`, without modification *)
  val debug_dump : db_dir:string -> Read.t

  val solve : db_dir:string -> Read.t
end

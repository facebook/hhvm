(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Options : sig
  type command =
    | DumpConstraints
    | SolveConstraints

  type t = {
    command: command;
    verbosity: int;
  }
end

module Constraint : sig
  type t = NeedsSDT [@@deriving ord, show]
end

type 'a decorated = {
  hack_pos: Pos.t;
  origin: int;
  decorated_data: 'a;
}
[@@deriving ord]

module H : Hips2.T with type intra_constraint_ = Constraint.t

module IdMap : Map.S with type key := H.id

module WalkResult : sig
  type 'a t = 'a list IdMap.t

  val ( @ ) : 'a t -> 'a t -> 'a t

  val empty : 'a t

  val add_constraint : 'a t -> H.id -> 'a -> 'a t

  val add_id : 'a t -> H.id -> 'a t

  val singleton : H.id -> 'a -> 'a t
end

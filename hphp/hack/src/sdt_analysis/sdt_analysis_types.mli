(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Options : sig
  type command = DumpConstraints

  type t = {
    command: command;
    verbosity: int;
  }
end

module Constraint : sig
  type t = NeedsSDT [@@deriving ord, show]
end

module DecoratedConstraint : sig
  type t = {
    hack_pos: Pos.t;
    origin: int;
    constraint_: Constraint.t;
  }
  [@@deriving ord]

  module Set : Caml.Set.S with type elt = t
end

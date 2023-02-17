(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Options = struct
  type command = DumpConstraints

  type t = {
    command: command;
    verbosity: int;
  }
end

module Constraint = struct
  type t = NeedsSDT [@@deriving ord, show { with_path = false }]
end

module DecoratedConstraint = struct
  type t = {
    hack_pos: Pos.t;
    origin: int;
    constraint_: Constraint.t;
  }
  [@@deriving ord]

  module Set = Caml.Set.Make (struct
    type nonrec t = t

    let compare = compare
  end)
end

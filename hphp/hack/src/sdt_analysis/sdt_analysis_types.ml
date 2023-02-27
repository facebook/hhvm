(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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
  [@@deriving ord, show { with_path = false }]
end

module H = Hips2.Make (struct
  type constraint_ = DecoratedConstraint.t

  let debug_show_constraint_ = DecoratedConstraint.show
end)

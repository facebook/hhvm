(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Options = struct
  type command =
    | DumpConstraints
    | SolveConstraints
    | DumpPersistedConstraints
    | SolvePersistedConstraints

  type t = {
    command: command;
    verbosity: int;
  }
end

module Constraint = struct
  type t = NeedsSDT [@@deriving eq, hash, ord, show { with_path = false }]
end

type 'a decorated = {
  hack_pos: Pos.t;
  origin: int;
  decorated_data: 'a;
}
[@@deriving ord]

module H = Hips2.Make (struct
  type constraint_ = Constraint.t [@@deriving eq, hash, show]
end)

module IdMap = Map.Make (struct
  type t = H.id

  let compare = H.compare_id
end)

module WalkResult = struct
  type 'a t = 'a list IdMap.t

  let ( @ ) t1 t2 = IdMap.union (fun _k v1 v2 -> Some (v1 @ v2)) t1 t2

  let empty = IdMap.empty

  let add_constraint t id constraint_ =
    let merge = function
      | None -> Some [constraint_]
      | Some constraints -> Some (constraint_ :: constraints)
    in
    IdMap.update id merge t

  let add_id t id =
    let merge = function
      | None -> Some []
      | Some _ as some -> some
    in
    IdMap.update id merge t

  let singleton id constraint_ = add_constraint empty id constraint_
end

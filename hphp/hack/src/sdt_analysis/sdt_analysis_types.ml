(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

module Options = struct
  type command =
    | DumpConstraints
    | SolveConstraints

  type t = {
    command: command;
    verbosity: int;
  }
end

module Constraint = struct
  type t = NeedsSDT [@@deriving eq, hash, ord, show { with_path = false }]
end

type abstraction = Ast_defs.abstraction =
  | Concrete
  | Abstract
[@@deriving eq, hash, ord, sexp, show { with_path = false }]

type classish_kind = Ast_defs.classish_kind =
  | Cclass of abstraction  (** Kind for `class` and `abstract class` *)
  | Cinterface  (** Kind for `interface` *)
  | Ctrait  (** Kind for `trait` *)
  | Cenum  (** Kind for `enum` *)
  | Cenum_class of abstraction
[@@deriving eq, hash, ord, sexp, show { with_path = false }]

module Relative_path = struct
  include Relative_path

  let hash_fold_t hash_state path =
    String.hash_fold_t hash_state (Relative_path.storage_to_string path)
end

module CustomInterConstraint = struct
  type t = {
    classish_kind_opt: classish_kind option;
    hierarchy_for_final_item: string list option;
    path_opt: Relative_path.t option;
  }
  [@@deriving eq, hash, ord, show { with_path = false }]
end

type 'a decorated = {
  hack_pos: Pos.t;
  origin: int;
  decorated_data: 'a;
}
[@@deriving ord]

module H = Hips2.Make (struct
  type constraint_ = Constraint.t
  [@@deriving eq, hash, show { with_path = false }]

  type custom_inter_constraint_ = CustomInterConstraint.t
  [@@deriving eq, hash, ord, show { with_path = false }]
end)

module IdMap = Caml.Map.Make (H.Id)

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

module Summary = struct
  type nadable_kind =
    | ClassLike of classish_kind option
    | Function

  type nadable = {
    id: H.Id.t;
    kind: nadable_kind;
    path_opt: Relative_path.t option;
  }

  type t = {
    id_cnt: int;
    syntactically_nadable_cnt: int;
    nadable_cnt: int;
    nadable_groups: nadable list Sequence.t;
  }
end

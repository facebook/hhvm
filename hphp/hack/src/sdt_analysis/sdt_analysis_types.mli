(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

module Options : sig
  type command =
    | DumpConstraints
        (** print constraints for a single file without solving *)
    | SolveConstraints  (** generate and solve constraints for a single file *)
    | DumpPersistedConstraints
        (** For debugging: print all constraints from directory `Sdt_analysis.default_db_dir`, without solving.
    NOTE: hh_single_type_check requires an arg that is a path to a Hack file, but the analysis intentionally ignores that argument (T146711502)
     *)
    | SolvePersistedConstraints
        (** Solve all constraints from directory `Sdt_analysis.default_db_dir`.
        The directory must already contain constraints generated with `hh  --config log_levels='{"sdt_analysis":1}'
    NOTE: hh_single_type_check requires an arg that is a path to a Hack file, but the analysis intentionally ignores that argument (T146711502)
     *)

  type t = {
    command: command;
    verbosity: int;
  }
end

module Constraint : sig
  type t = NeedsSDT [@@deriving ord, show]
end

type abstraction = Ast_defs.abstraction =
  | Concrete
  | Abstract
[@@deriving eq, hash, ord, sexp, show { with_path = false }]

val hash_abstraction : abstraction -> int

type classish_kind = Ast_defs.classish_kind =
  | Cclass of abstraction  (** Kind for `class` and `abstract class` *)
  | Cinterface  (** Kind for `interface` *)
  | Ctrait  (** Kind for `trait` *)
  | Cenum  (** Kind for `enum` *)
  | Cenum_class of abstraction
[@@deriving eq, hash, ord, sexp, show { with_path = false }]

val hash_classish_kind : classish_kind -> int

module CustomInterConstraint : sig
  (** Facts that help us summarize results. *)
  type t = {
    classish_kind_opt: classish_kind option;
        (** In `CustomInterConstraint`s, classish_kind is always `None` for functions *)
    hierarchy_for_final_item: string list option;
        (**
        `Some []` indicates something with no parents or descendents, such as a top-level function or final class with no `extends` or `implements`.
        `Some [c1, c2]` indicates a final class that inherits transitively from c1 and c2.
        Anything else is `None`
        *)
  }
end

type 'a decorated = {
  hack_pos: Pos.t;
  origin: int;
  decorated_data: 'a;
}
[@@deriving ord]

module H :
  Hips2.T
    with type intra_constraint_ = Constraint.t
     and type custom_inter_constraint_ = CustomInterConstraint.t

module IdMap : Caml.Map.S with type key := H.Id.t

module WalkResult : sig
  type 'a t = 'a list IdMap.t

  val ( @ ) : 'a t -> 'a t -> 'a t

  val empty : 'a t

  val add_constraint : 'a t -> H.Id.t -> 'a -> 'a t

  val add_id : 'a t -> H.Id.t -> 'a t

  val singleton : H.Id.t -> 'a -> 'a t
end

module Summary : sig
  type nadable_kind =
    | ClassLike of classish_kind option
        (** classish_kind of `None` indicates unknown, which can happen for definitions in hhis *)
    | Function

  type nadable = {
    id: H.Id.t;
    kind: nadable_kind;
  }

  type t = {
    id_cnt: int;
    (* count of functions and class-like things *)
    syntactically_nadable_cnt: int;
    (* count of things where the <<__NoAutoDynamic>> attribute is syntactically allowed *)
    nadable_cnt: int;
    (* count of things the analysis thinks can have <<__NoAutoDynamic>> added *)
    nadable_groups: nadable list Sequence.t;
        (* Sequence of things the analysis thinks can have <<__NoAutoDynamic>> added. Items in the same list should be modified together. *)
  }
end

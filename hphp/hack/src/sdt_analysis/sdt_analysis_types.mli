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

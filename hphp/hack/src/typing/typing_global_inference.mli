(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_env_types

module StateErrors : sig
  type t

  val mk_empty : unit -> t

  val add : t -> Ident.t -> unit

  val has_error : t -> Ident.t -> bool

  val show : t -> string -> unit
end

module StateSubConstraintGraphs : sig
  type t = global_tvenv list

  val load : string -> t

  val save : t -> unit
end

module StateConstraintGraph : sig
  type t = env * StateErrors.t

  val load : string -> t

  val save : string -> t -> unit

  val merge_subgraphs : t -> StateSubConstraintGraphs.t -> t
end

module StateSolvedGraph : sig
  type t = env * StateErrors.t * (Pos.t * int) list

  val load : string -> t

  val save : string -> t -> unit

  val from_constraint_graph : StateConstraintGraph.t -> t
end

val init : unit -> unit

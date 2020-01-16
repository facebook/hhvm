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

  val add : t -> Ident.t -> Errors.error -> unit

  val has_error : t -> Ident.t -> bool

  val get_errors : t -> Ident.t -> Errors.error list

  val elements : t -> (Ident.t * Errors.error list) list
end

module StateSubConstraintGraphs : sig
  type t = Typing_inference_env.t_global_with_pos list

  val load : string -> t

  val save : t -> unit

  val save_to : string -> t -> unit
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

val set_path : unit -> unit

val get_path : unit -> string

val restore_path : string -> unit

val init : unit -> unit

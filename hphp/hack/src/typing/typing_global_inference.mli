(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_env_types

type global_type_map = Typing_defs.locl_ty Pos.AbsolutePosMap.t

module StateErrors : sig
  type t

  val mk_empty : unit -> t

  val add : t -> Ident.t -> Errors.error -> unit

  val has_error : t -> Ident.t -> bool

  val get_errors : t -> Ident.t -> Errors.error list

  val elements : t -> (Ident.t * Errors.error list) list

  val cardinal : t -> int
end

module StateSubConstraintGraphs : sig
  type t = global_type_map * Typing_inference_env.t_global_with_pos list

  val global_tvenvs : t -> Typing_inference_env.t_global_with_pos list

  val global_type_map : t -> global_type_map

  val build :
    Provider_context.t ->
    Tast.program ->
    Typing_inference_env.t_global_with_pos list ->
    t

  val load : string -> t

  val save : t -> unit

  val save_to : string -> t -> unit

  val build_and_save :
    Provider_context.t ->
    Tast.program ->
    Typing_inference_env.t_global_with_pos list ->
    unit
end

module StateConstraintGraph : sig
  type t = global_type_map * env * StateErrors.t

  val load : string -> t

  val save : string -> t -> unit

  val merge_subgraphs :
    Provider_context.t -> StateSubConstraintGraphs.t list -> t
end

module StateSolvedGraph : sig
  type t = env * StateErrors.t * global_type_map

  val load : string -> t

  val save : string -> t -> unit

  val from_constraint_graph : StateConstraintGraph.t -> t
end

val set_path : unit -> unit

val get_path : unit -> string

val restore_path : string -> unit

val init : unit -> unit

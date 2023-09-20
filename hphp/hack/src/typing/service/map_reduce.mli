(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** A map-reducer maps a TAST into intermediate data, and then reduces a
    bunch of intermediate data to a single value. *)
module type MapReducer = sig
  (** Type of the intermediate and reduced data. *)
  type t

  (** A function that takes type checker options and returns whether the
      map-reducer is enabled *)
  val is_enabled : TypecheckerOptions.t -> bool

  (** A map function that takes in a TAST and produces the intermediate data.

  Note that we don't make guarantees about the uniqueness of the file path. *)
  val map :
    Provider_context.t -> Relative_path.t -> Tast.by_names -> Errors.t -> t

  (** Reduce two intermediate data elements. *)
  val reduce : t -> t -> t

  (** Consume the final result of the full map-reduce analysis. *)
  val finalize :
    progress:(string -> unit) ->
    init_id:string ->
    recheck_id:string option ->
    t ->
    unit
end

(** The result of an analysis. *)
type t

(** The empty analysis. *)
val empty : t

(** Take in a TAST and analyze it. *)
val map :
  Provider_context.t -> Relative_path.t -> Tast.by_names -> Errors.t -> t

(** Reduce two analysis into one. *)
val reduce : t -> t -> t

(** Consume the final result of the full map-reduce analysis. *)
val finalize :
  progress:(string -> unit) ->
  init_id:string ->
  recheck_id:string option ->
  t ->
  unit

(** Convert the results of an anlysis to an FFI friendly data structure *)
val to_ffi : t -> Map_reduce_ffi.t

(** Read in the results of an analysis from an FFI friendly data structure *)
val of_ffi : Map_reduce_ffi.t -> t

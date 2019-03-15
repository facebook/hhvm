(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Dep :
  sig
    type variant =
      | GConst of string
      | GConstName of string
      | Const of string * string
      | AllMembers of string
      | Class of string
      | Fun of string
      | FunName of string
      | Prop of string * string
      | SProp of string * string
      | Method of string * string
      | SMethod of string * string
      | Cstr of string
      | Extends of string
    type t
    val make : variant -> t
    val is_class : t -> bool
    val extends_of_class : t -> t
    val compare : t -> t -> int
    val to_string : variant -> string
  end

module DepSet : module type of
  Reordered_argument_collections.Reordered_argument_set(Set.Make (Dep))

val trace : bool ref

val debug_trace : bool ref
val dump_debug_deps : unit -> unit

(* returns the previous value of the flag *)
val allow_dependency_table_reads: bool -> bool

val add_idep : Dep.variant -> Dep.variant -> unit
val get_ideps_from_hash : Dep.t -> DepSet.t
val get_ideps : Dep.variant -> DepSet.t
val get_files : DepSet.t -> Relative_path.Set.t
val update_file : Relative_path.t -> FileInfo.t -> unit

(* Add to accumulator all extend dependencies of source_class. Visited is used
 * to avoid processing nodes reachable in multiple ways more than once. In other
 * words: use DFS to find all nodes reachable by "extends" edges starting from
 * source class *)
val get_extend_deps:
  visited:(DepSet.t ref) ->
  source_class:Dep.t ->
  acc:DepSet.t ->
  DepSet.t

(* Grow input set by adding all its extend dependencies (including recursive) *)
val add_extend_deps : DepSet.t -> DepSet.t
(* Grow input set by adding all its typing dependencies (direct only) *)
val add_typing_deps : DepSet.t -> DepSet.t
(* add_extend_deps and add_typing_deps chained together *)
val add_all_deps : DepSet.t -> DepSet.t

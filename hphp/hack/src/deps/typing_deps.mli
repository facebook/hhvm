(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Dep :
  sig
    type variant =
      | GConst of string
      | GConstName of string
      | Const of string * string
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


type debug_trace_type = Bazooka | Full | No_trace

val trace : bool ref

val debug_trace : debug_trace_type ref
val print_string_hash_set : string HashSet.t -> unit
val dump_debug_deps : unit -> unit

val add_idep : Dep.variant -> Dep.variant -> unit
val get_ideps_from_hash : Dep.t -> DepSet.t
val get_ideps : Dep.variant -> DepSet.t
val get_bazooka : Dep.variant -> DepSet.t
val get_files : DepSet.t -> Relative_path.Set.t
val update_files : FileInfo.t Relative_path.Map.t -> unit

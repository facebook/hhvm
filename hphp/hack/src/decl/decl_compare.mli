(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_deps
open Decl_heap

val class_big_diff : Class.t -> Class.t -> bool

module ClassDiff : sig
  val compare :
    Typing_deps_mode.t -> string -> Class.t -> Class.t -> DepSet.t * bool
end

module ClassEltDiff : sig
  val compare :
    Typing_deps_mode.t ->
    Class.t ->
    Class.t ->
    DepSet.t * [> `Changed | `Unchanged ]
end

val get_extend_deps : Typing_deps_mode.t -> DepSet.elt -> DepSet.t -> DepSet.t

val get_classes_deps :
  ctx:Provider_context.t ->
  Class.t option SMap.t ->
  Class.t option SMap.t ->
  SSet.t ->
  DepSet.t * DepSet.t * DepSet.t

val get_funs_deps :
  ctx:Provider_context.t ->
  Funs.t option SMap.t ->
  SSet.t ->
  DepSet.t * DepSet.t * DepSet.t

val get_types_deps :
  ctx:Provider_context.t ->
  Typedef.t option SMap.t ->
  SSet.t ->
  DepSet.t * DepSet.t

val get_gconsts_deps :
  ctx:Provider_context.t ->
  GConsts.t option SMap.t ->
  SSet.t ->
  DepSet.t * DepSet.t * DepSet.t

val get_record_defs_deps :
  ctx:Provider_context.t ->
  Decl_heap.RecordDef.t option SMap.t ->
  SSet.t ->
  Typing_deps.DepSet.t * Typing_deps.DepSet.t * Typing_deps.DepSet.t

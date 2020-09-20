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

val class_big_diff : Classes.t -> Classes.t -> bool

module ClassDiff : sig
  val compare : string -> Classes.t -> Classes.t -> DepSet.t * bool
end

module ClassEltDiff : sig
  val compare : Classes.t -> Classes.t -> DepSet.t * [> `Changed | `Unchanged ]
end

val get_extend_deps : DepSet.elt -> DepSet.t -> DepSet.t

val get_classes_deps :
  Provider_context.t ->
  conservative_redecl:bool ->
  Classes.t option SMap.t ->
  Classes.t option SMap.t ->
  SSet.t ->
  DepSet.t * DepSet.t * DepSet.t

val get_funs_deps :
  conservative_redecl:bool ->
  Funs.t option SMap.t ->
  SSet.t ->
  DepSet.t * DepSet.t * DepSet.t

val get_types_deps : Typedef.t option SMap.t -> SSet.t -> DepSet.t * DepSet.t

val get_gconsts_deps :
  conservative_redecl:bool ->
  GConsts.t option SMap.t ->
  SSet.t ->
  DepSet.t * DepSet.t * DepSet.t

val get_record_defs_deps :
  conservative_redecl:bool ->
  Decl_heap.RecordDef.t option SMap.t ->
  SSet.t ->
  Typing_deps.DepSet.t * Typing_deps.DepSet.t * Typing_deps.DepSet.t

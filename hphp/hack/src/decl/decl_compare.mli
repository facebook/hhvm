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
  val compare : Class.t -> Class.t -> bool
end

module ClassEltDiff : sig
  val compare : Class.t -> Class.t -> [> `Changed | `Unchanged ]
end

val get_extend_deps : Typing_deps_mode.t -> DepSet.elt -> DepSet.t -> DepSet.t

val get_funs_deps :
  ctx:Provider_context.t -> Funs.value option SMap.t -> SSet.t -> Fanout.t * int

val get_types_deps :
  ctx:Provider_context.t -> Typedef.t option SMap.t -> SSet.t -> Fanout.t * int

val get_gconsts_deps :
  ctx:Provider_context.t ->
  GConsts.value option SMap.t ->
  SSet.t ->
  Fanout.t * int

val get_modules_deps :
  ctx:Provider_context.t ->
  old_modules:Modules.value option SMap.t ->
  modules:SSet.t ->
  Fanout.t * int

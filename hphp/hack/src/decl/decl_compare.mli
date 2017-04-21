(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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

val get_classes_deps : Classes.t option SMap.t -> Classes.t option SMap.t ->
  SSet.t -> DepSet.t * DepSet.t

val get_funs_deps : Funs.t option SMap.t -> SSet.t -> DepSet.t * DepSet.t

val get_types_deps : Typedef.t option SMap.t -> SSet.t -> DepSet.t

val get_gconsts_deps : GConsts.t option SMap.t -> SSet.t -> DepSet.t * DepSet.t

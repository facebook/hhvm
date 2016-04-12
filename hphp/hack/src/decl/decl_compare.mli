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
open Typing_heap

val get_extend_deps : DepSet.elt -> DepSet.t -> DepSet.t

val get_classes_deps : Classes.t option SMap.t -> Classes.t option SMap.t ->
  SSet.t -> DepSet.t * DepSet.t

val get_funs_deps : Funs.t option SMap.t -> SSet.t -> DepSet.t * DepSet.t

val get_types_deps : Typedef.t option SMap.t -> SSet.t -> DepSet.t

val get_gconsts_deps : GConsts.t option SMap.t -> SSet.t -> DepSet.t * DepSet.t

(*
 * XXX UNUSED: Position substition has been disabled for now, but we're
 * leaving the code in to minimize bitrot
 *)
val get_classes_psubst : Classes.t option SMap.t ->
  Classes.t option SMap.t -> SSet.t -> (Pos.t, Pos.t) Hashtbl.t * bool

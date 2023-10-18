(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shallow_decl_defs

val get_old_batch :
  Provider_context.t ->
  during_init:bool ->
  SSet.t ->
  shallow_class option SMap.t

val oldify_batch : Provider_context.t -> SSet.t -> unit

val remove_old_batch : Provider_context.t -> SSet.t -> unit

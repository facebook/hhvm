(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val tast_holes :
  Provider_context.t ->
  Tast.program ->
  Server_command_types.Tast_hole.filter ->
  Tast_holes_service.result

val go_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  hole_filter:Server_command_types.Tast_hole.filter ->
  Tast_holes_service.result

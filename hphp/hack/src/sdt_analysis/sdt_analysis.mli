(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val do_ :
  command:string ->
  verbosity:int ->
  on_bad_command:(string -> 'a) ->
  unit ->
  Provider_context.t ->
  Tast.program ->
  unit

val create_handler : Provider_context.t -> Tast_visitor.handler

(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Returns signature help for the given location. *)
val go :
  env:ServerEnv.env ->
  file:ServerCommandTypes.file_input ->
  line:int ->
  column:int ->
  Lsp.SignatureHelp.result

val go_ctx :
  env:ServerEnv.env ->
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  line:int ->
  column:int ->
  Lsp.SignatureHelp.result

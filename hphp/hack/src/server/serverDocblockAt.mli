(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Returns the documentation comments for the given symbol or expression. *)
val go_comments_for_symbol :
  def:'a SymbolDefinition.t ->
  base_class_name:string option ->
  file:ServerCommandTypes.file_input ->
  string option

(** Returns the docblock most appropriate to this position *)
val go_docblock_at :
  filename:string ->
  line:int ->
  column:int ->
  kind:SearchUtils.si_kind ->
  DocblockService.result

(** Returns the docblock from these file contents *)
val go_docblock_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  line:int ->
  column:int ->
  kind:SearchUtils.si_kind ->
  DocblockService.result

(** Simplified one-step symbol/docblock *)
val go_docblock_for_symbol :
  env:ServerEnv.env ->
  symbol:string ->
  kind:SearchUtils.si_kind ->
  DocblockService.result

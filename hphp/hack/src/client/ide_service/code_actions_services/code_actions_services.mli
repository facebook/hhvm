(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  range:Ide_api_types.range ->
  Lsp.CodeAction.result

val resolve :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  range:Ide_api_types.range ->
  resolve_title:string ->
  use_snippet_edits:bool ->
  Lsp.CodeActionResolve.result

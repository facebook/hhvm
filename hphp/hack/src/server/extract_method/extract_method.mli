(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val find :
  range:Lsp.range ->
  path:string ->
  entry:Provider_context.entry ->
  Provider_context.t ->
  Lsp.CodeAction.resolvable_command_or_action list

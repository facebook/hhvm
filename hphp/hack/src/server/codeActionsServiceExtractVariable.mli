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
  Tast.def list ->
  Lsp.CodeAction.command_or_action list

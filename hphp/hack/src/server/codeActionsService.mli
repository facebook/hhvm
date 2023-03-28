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
  path:string ->
  range:Ide_api_types.range ->
  Lsp.CodeAction.result

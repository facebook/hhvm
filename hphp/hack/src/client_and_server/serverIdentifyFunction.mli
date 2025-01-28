(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go_quarantined :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  File_content.Position.t ->
  (Relative_path.t SymbolOccurrence.t
  * Relative_path.t SymbolDefinition.t option)
  list

val go_quarantined_absolute :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  File_content.Position.t ->
  (string SymbolOccurrence.t * string SymbolDefinition.t option) list

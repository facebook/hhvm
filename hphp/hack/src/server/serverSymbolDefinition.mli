(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go :
  Provider_context.t ->
  Nast.program option ->
  Relative_path.t SymbolOccurrence.t ->
  Relative_path.t SymbolDefinition.t option

val get_definition_cst_node_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  kind:SymbolDefinition.kind ->
  pos:'a Pos.pos ->
  Full_fidelity_positioned_syntax.t option

val get_class_by_name : Provider_context.t -> string -> Nast.class_ option

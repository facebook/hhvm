(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go :
  Ast.program ->
  Relative_path.t SymbolOccurrence.t ->
  Relative_path.t SymbolDefinition.t option

val get_definition_cst_node_from_pos :
  SymbolDefinition.kind ->
  Full_fidelity_source_text.t ->
  'a Pos.pos ->
  Full_fidelity_positioned_syntax.t option

val get_definition_cst_node :
  ServerCommandTypes.file_input ->
  Relative_path.t SymbolDefinition.t ->
  Full_fidelity_positioned_syntax.t option

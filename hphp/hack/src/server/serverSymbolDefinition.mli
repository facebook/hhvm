(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

val go :
  TypecheckerOptions.t ->
  Ast.program ->
  Relative_path.t SymbolOccurrence.t ->
  Relative_path.t SymbolDefinition.t option

val get_definition_cst_node :
  ServerUtils.file_input ->
  Relative_path.t SymbolDefinition.t ->
  Full_fidelity_positioned_syntax.t option

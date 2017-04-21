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

val from_symbol_id :
  TypecheckerOptions.t ->
  string ->
  Relative_path.t SymbolDefinition.t option

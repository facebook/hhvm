(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go :
  ('a, 'b) Aast.program ->
  Pos.t ->
  string ->
  Relative_path.t SymbolDefinition.t option

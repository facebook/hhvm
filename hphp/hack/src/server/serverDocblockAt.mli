(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Returns the docblock for the given symbol or expression. *)
val go_def :
  Relative_path.t SymbolDefinition.t ->
  base_class_name: string option ->
  file: ServerCommandTypes.file_input ->
  basic_only: bool ->
  DocblockService.result

(** Returns the docblock for the symbol or expression at the given location. *)
val go_location :
  ServerEnv.env ->
  (string * int * int) ->
  base_class_name: string option ->
  basic_only: bool ->
  DocblockService.result

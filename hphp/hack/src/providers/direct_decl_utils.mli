(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val direct_decl_parse_and_cache :
  ?file_decl_hash:bool ->
  ?symbol_decl_hashes:bool ->
  Provider_context.t ->
  Relative_path.t ->
  ((string * Shallow_decl_defs.decl) list
  * FileInfo.mode option
  * Int64.t option
  * Int64.t option list)
  option

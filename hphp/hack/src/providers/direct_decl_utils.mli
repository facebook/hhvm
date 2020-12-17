(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val direct_decl_parse_and_cache :
  ?decl_hash:bool ->
  Provider_context.t ->
  Relative_path.t ->
  ( (string * Shallow_decl_defs.decl) list
  * FileInfo.mode option
  * Int64.t option )
  option

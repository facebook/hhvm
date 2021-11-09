(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type parsed_file_with_hashes = Direct_decl_parser.parsed_file_with_hashes = {
  pfh_mode: FileInfo.mode option;
  pfh_hash: Int64.t;
  pfh_decls: (string * Shallow_decl_defs.decl * Int64.t) list;
}

val direct_decl_parse_and_cache :
  Provider_context.t ->
  Relative_path.t ->
  Direct_decl_parser.parsed_file_with_hashes option

val direct_decl_parse :
  Provider_context.t ->
  Relative_path.t ->
  Direct_decl_parser.parsed_file_with_hashes option

val cache_decls :
  Provider_context.t ->
  Relative_path.t ->
  (string * Shallow_decl_defs.decl * Int64.t) list ->
  unit

val decls_to_fileinfo :
  Relative_path.t -> Direct_decl_parser.parsed_file_with_hashes -> FileInfo.t

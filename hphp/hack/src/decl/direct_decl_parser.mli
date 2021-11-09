(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type decls = (string * Shallow_decl_defs.decl) list [@@deriving show]

type parsed_file = {
  pf_mode: FileInfo.mode option;
  pf_file_attributes: Typing_defs.user_attribute list;
  pf_decls: decls;
  pf_has_first_pass_parse_errors: bool;
}

type parsed_file_with_hashes = {
  pfh_mode: FileInfo.mode option;
  pfh_hash: Int64.t;
  pfh_decls: (string * Shallow_decl_defs.decl * Int64.t) list;
}

val parse_decls_ffi : DeclParserOptions.t -> Relative_path.t -> string -> decls

val decls_hash : decls -> Int64.t

val parse_decls_and_mode_ffi :
  DeclParserOptions.t ->
  Relative_path.t ->
  string ->
  bool ->
  bool ->
  decls * FileInfo.mode option * Int64.t option * Int64.t list option

val decls_to_fileinfo :
  Relative_path.t ->
  (string * Shallow_decl_defs.decl) list
  * FileInfo.mode option
  * Int64.t option
  * Int64.t option list ->
  FileInfo.t

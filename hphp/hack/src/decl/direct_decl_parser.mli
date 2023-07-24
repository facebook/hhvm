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

(** NOTE: this doesn't respect deregister_php_lib, and has decls in reverse lexical order. *)
val parse_decls :
  DeclParserOptions.t -> Relative_path.t -> string -> parsed_file

(** NOTE: this doesn't respect deregister_php_lib, and has decls in reverse lexical order *)
val parse_and_hash_decls :
  DeclParserOptions.t ->
  bool ->
  Relative_path.t ->
  string ->
  parsed_file_with_hashes

val decls_hash : decls -> Int64.t

(** NOTE: this takes input in reverse-lexical-order, and emits FileInfo.t in forward lexical order *)
val decls_to_fileinfo : Relative_path.t -> parsed_file_with_hashes -> FileInfo.t

val decls_to_addenda : parsed_file_with_hashes -> SearchUtils.si_addendum list

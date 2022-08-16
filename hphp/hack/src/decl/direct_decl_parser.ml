(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

open Hh_prelude

(* NB: Must keep in sync with Rust type
   oxidized_by_ref::direct_decl_parser::Decls *)
type decls = (string * Shallow_decl_defs.decl) list [@@deriving show]

(* NB: Must keep in sync with Rust type
   oxidized_by_ref::direct_decl_parser::ParsedFile *)
type parsed_file = {
  pf_mode: FileInfo.mode option;
  pf_file_attributes: Typing_defs.user_attribute list;
  pf_decls: decls;
  pf_has_first_pass_parse_errors: bool;
}

(* NB: Must keep in sync with Rust type rust_decl_ffi::ParsedFileWithHashes *)
type parsed_file_with_hashes = {
  pfh_mode: FileInfo.mode option;
  pfh_hash: Int64.t;
  pfh_decls: (string * Shallow_decl_defs.decl * Int64.t) list;
}

external parse_decls :
  DeclParserOptions.t -> Relative_path.t -> string -> parsed_file
  = "hh_parse_decls_ffi"

external parse_and_hash_decls :
  DeclParserOptions.t -> Relative_path.t -> string -> parsed_file_with_hashes
  = "hh_parse_and_hash_decls_ffi"

external decls_hash : decls -> Int64.t = "decls_hash"

let decls_to_fileinfo fn (parsed_file : parsed_file_with_hashes) =
  let file_mode = parsed_file.pfh_mode in
  let hash = Some parsed_file.pfh_hash in
  List.fold
    parsed_file.pfh_decls
    ~init:FileInfo.{ empty_t with hash; file_mode; comments = None }
    ~f:(fun acc (name, decl, hash) ->
      let pos p = FileInfo.Full (Pos_or_decl.fill_in_filename fn p) in
      match decl with
      | Shallow_decl_defs.Class c ->
        let info = (pos (fst c.Shallow_decl_defs.sc_name), name, Some hash) in
        FileInfo.{ acc with classes = info :: acc.classes }
      | Shallow_decl_defs.Fun f ->
        let info = (pos f.Typing_defs.fe_pos, name, Some hash) in
        FileInfo.{ acc with funs = info :: acc.funs }
      | Shallow_decl_defs.Typedef tf ->
        let info = (pos tf.Typing_defs.td_pos, name, Some hash) in
        FileInfo.{ acc with typedefs = info :: acc.typedefs }
      | Shallow_decl_defs.Const c ->
        let info = (pos c.Typing_defs.cd_pos, name, Some hash) in
        FileInfo.{ acc with consts = info :: acc.consts }
      | Shallow_decl_defs.Module m ->
        let info = (pos m.Typing_defs.mdt_pos, name, Some hash) in
        FileInfo.{ acc with modules = info :: acc.modules })

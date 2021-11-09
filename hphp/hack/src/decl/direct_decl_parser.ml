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
  pf_decls: decls;
}

(* NB: Must keep in sync with Rust type rust_decl_ffi::ParsedFileWithHashes *)
type parsed_file_with_hashes = {
  pfh_mode: FileInfo.mode option;
  pfh_hash: Int64.t;
  pfh_decls: (string * Shallow_decl_defs.decl * Int64.t) list;
}

external parse_decls_ffi :
  DeclParserOptions.t -> Relative_path.t -> string -> parsed_file
  = "hh_parse_decls_ffi"

external parse_and_hash_decls_ffi :
  DeclParserOptions.t -> Relative_path.t -> string -> parsed_file_with_hashes
  = "hh_parse_and_hash_decls_ffi"

external decls_hash : decls -> Int64.t = "decls_hash"

let parse_decls_ffi
    (opts : DeclParserOptions.t) (path : Relative_path.t) (text : string) :
    decls =
  let parsed_file = parse_decls_ffi opts path text in
  parsed_file.pf_decls

let parse_decls_and_mode_ffi
    (opts : DeclParserOptions.t)
    (path : Relative_path.t)
    (text : string)
    _include_file_hash
    _include_decl_hashes :
    decls * FileInfo.mode option * Int64.t option * Int64.t list option =
  let parsed_file = parse_and_hash_decls_ffi opts path text in
  let decls =
    List.map parsed_file.pfh_decls ~f:(fun (name, decl, _) -> (name, decl))
  in
  let decl_hashes =
    List.map parsed_file.pfh_decls ~f:(fun (_, _, hash) -> hash)
  in
  (decls, parsed_file.pfh_mode, Some parsed_file.pfh_hash, Some decl_hashes)

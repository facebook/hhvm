(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

type decls = (string * Shallow_decl_defs.decl) list [@@deriving show]

external parse_decls_and_mode_ffi :
  (* (disable_xhp_element_mangling, interpret_soft_types_as_like_types) *)
  DeclParserOptions.t ->
  Relative_path.t ->
  string ->
  bool ->
  decls * FileInfo.mode option * Int64.t option = "hh_parse_decls_and_mode_ffi"

external decls_hash : decls -> Int64.t = "decls_hash"

let parse_decls_ffi
    (opts : DeclParserOptions.t) (path : Relative_path.t) (text : string) :
    decls =
  let (decls, _, _) = parse_decls_and_mode_ffi opts path text false in
  decls

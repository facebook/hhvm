(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

type decls = (string * Shallow_decl_defs.decl) list [@@deriving show]

type decl_lists = {
  dl_classes: (string * Shallow_decl_defs.shallow_class) list;
  dl_funs: (string * Typing_defs.fun_elt) list;
  dl_typedefs: (string * Typing_defs.typedef_type) list;
  dl_consts: (string * Typing_defs.const_decl) list;
  dl_records: (string * Typing_defs.record_def_type) list;
}

type ns_map = (string * string) list

external parse_decls_and_mode_ffi :
  Relative_path.t -> string -> ns_map -> decls * FileInfo.mode option
  = "hh_parse_decls_and_mode_ffi"

let parse_decls_ffi (path : Relative_path.t) (text : string) (ns_map : ns_map) :
    decls =
  parse_decls_and_mode_ffi path text ns_map |> fst

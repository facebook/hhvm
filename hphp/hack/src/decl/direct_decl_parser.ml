(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

type decls = {
  classes: Shallow_decl_defs.shallow_class SMap.t;
  funs: Typing_defs.fun_elt SMap.t;
  typedefs: Typing_defs.typedef_type SMap.t;
  consts: Typing_defs.const_decl SMap.t;
  records: Typing_defs.record_def_type SMap.t;
}
[@@deriving show]

let empty_decls =
  {
    classes = SMap.empty;
    funs = SMap.empty;
    typedefs = SMap.empty;
    consts = SMap.empty;
    records = SMap.empty;
  }

type decl_lists = {
  dl_classes: (string * Shallow_decl_defs.shallow_class) list;
  dl_funs: (string * Typing_defs.fun_elt) list;
  dl_typedefs: (string * Typing_defs.typedef_type) list;
  dl_consts: (string * Typing_defs.const_decl) list;
  dl_records: (string * Typing_defs.record_def_type) list;
}

type ns_map = (string * string) list

external parse_decls_ffi : Relative_path.t -> string -> ns_map -> decls
  = "hh_parse_decls_ffi"

external parse_decl_lists_ffi :
  Relative_path.t -> string -> ns_map -> decl_lists * FileInfo.mode option
  = "hh_parse_decl_lists_ffi"

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  hack_arr_dv_arrs: bool;
  auto_namespace_map: (string * string) list;
  disable_xhp_element_mangling: bool;
  interpret_soft_types_as_like_types: bool;
}
[@@deriving show]

let from_parser_options popt =
  {
    hack_arr_dv_arrs = popt.GlobalOptions.po_hack_arr_dv_arrs;
    auto_namespace_map = popt.GlobalOptions.po_auto_namespace_map;
    disable_xhp_element_mangling =
      popt.GlobalOptions.po_disable_xhp_element_mangling;
    interpret_soft_types_as_like_types =
      popt.GlobalOptions.po_interpret_soft_types_as_like_types;
  }

let default = from_parser_options ParserOptions.default

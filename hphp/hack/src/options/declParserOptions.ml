(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  auto_namespace_map: (string * string) list;
  disable_xhp_element_mangling: bool;
  interpret_soft_types_as_like_types: bool;
  everything_sdt: bool;
}
[@@deriving show]

let from_parser_options popt =
  let open GlobalOptions in
  {
    auto_namespace_map = popt.po_auto_namespace_map;
    disable_xhp_element_mangling = popt.po_disable_xhp_element_mangling;
    interpret_soft_types_as_like_types =
      popt.po_interpret_soft_types_as_like_types;
    everything_sdt = popt.tco_everything_sdt;
  }

let default = from_parser_options ParserOptions.default

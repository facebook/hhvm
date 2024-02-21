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
  enable_xhp_class_modifier: bool;
  everything_sdt: bool;
  php5_compat_mode: bool;
  hhvm_compat_mode: bool;
  keep_user_attributes: bool;
  include_assignment_values: bool;
  stack_size: int;
}
[@@deriving show]

let from_parser_options popt =
  let open GlobalOptions in
  {
    auto_namespace_map = popt.po_auto_namespace_map;
    disable_xhp_element_mangling = popt.po_disable_xhp_element_mangling;
    interpret_soft_types_as_like_types =
      popt.po_interpret_soft_types_as_like_types;
    enable_xhp_class_modifier = popt.po_enable_xhp_class_modifier;
    everything_sdt = popt.tco_everything_sdt;
    php5_compat_mode = false;
    hhvm_compat_mode = false;
    keep_user_attributes = popt.po_keep_user_attributes;
    include_assignment_values = false;
    stack_size = popt.po_stack_size;
  }

let default = from_parser_options ParserOptions.default

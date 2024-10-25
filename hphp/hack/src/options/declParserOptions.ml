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
  deregister_php_stdlib: bool;
  package_info: PackageInfo.t;
  package_v2: bool;
  package_v2_support_multifile_tests: bool;
}
[@@deriving show]

let from_parser_options (popt : ParserOptions.t) =
  let open ParserOptions in
  {
    auto_namespace_map = popt.auto_namespace_map;
    disable_xhp_element_mangling = popt.disable_xhp_element_mangling;
    interpret_soft_types_as_like_types = popt.interpret_soft_types_as_like_types;
    enable_xhp_class_modifier = popt.enable_xhp_class_modifier;
    everything_sdt = popt.everything_sdt;
    php5_compat_mode = false;
    hhvm_compat_mode = false;
    keep_user_attributes = popt.keep_user_attributes;
    include_assignment_values = false;
    stack_size = popt.stack_size;
    deregister_php_stdlib = popt.deregister_php_stdlib;
    package_info = popt.package_info;
    package_v2 = popt.package_v2;
    package_v2_support_multifile_tests = popt.package_v2_support_multifile_tests;
  }

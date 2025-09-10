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
  stack_size: int;  (** Stack size to for the parallel workers *)
  deregister_php_stdlib: bool;
  package_info: PackageInfo.t;
  package_support_multifile_tests: bool;
  enable_class_pointer_hint: bool;
  disallow_non_annotated_memoize: bool;
  treat_non_annotated_memoize_as_kbic: bool;
}
[@@deriving show]

val from_parser_options : ParserOptions.t -> t

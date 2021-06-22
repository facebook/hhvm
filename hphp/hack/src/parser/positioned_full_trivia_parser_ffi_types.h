/* Copyright (c) 2021, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the MIT license found in the
 LICENSE file in the "hack" directory of this source tree.
*/

struct hackc_parse_positioned_full_trivia_environment {
  bool codegen;
  bool hhvm_compat_mode;
  bool php5_compat_mode;
  bool allow_new_attribute_syntax;
  bool enable_xhp_class_modifier;
  bool disable_xhp_element_mangling;
  bool disable_xhp_children_declarations;
  bool disable_modes;
  bool disallow_hash_comments;
  bool disallow_fun_and_cls_meth_pseudo_funcs;
  bool interpret_soft_types_as_like_types;
};

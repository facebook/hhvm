/* Copyright (c) 2021, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the MIT license found in the
 LICENSE file in the "hack" directory of this source tree.
*/
#if !defined(POSITIONED_PARSER_FFI_H)
#  define POSITIONED_PARSER_FFI_H

struct parse_positioned_environment {
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
  bool array_unification;
  bool interpret_soft_types_as_like_types;
};

#  if defined(__cplusplus)
extern "C" {
#  endif /*defined(__cplusplus)*/
char const* parse_positioned_cpp_ffi(
    char const* filename
  , char const* source_text
  , parse_positioned_environment const* env);

void parse_positioned_free_string_cpp_ffi(char const*);
#  if defined(__cplusplus)
}
#  endif /*defined(__cplusplus)*/

#endif/*!defined(POSITIONED_PARSER_FFI_H)*/

(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val mangle_xhp_mode : bool ref

val flags_abstract : int

val flags_final : int

val extract_as_json_string :
  php5_compat_mode:bool ->
  hhvm_compat_mode:bool ->
  disable_nontoplevel_declarations:bool ->
  disable_legacy_soft_typehints:bool ->
  allow_new_attribute_syntax:bool ->
  disable_legacy_attribute_syntax:bool ->
  enable_xhp_class_modifier:bool ->
  disable_xhp_element_mangling:bool ->
  disallow_hash_comments:bool ->
  filename:Relative_path.t ->
  text:string ->
  string option

val from_text :
  php5_compat_mode:bool ->
  hhvm_compat_mode:bool ->
  disable_nontoplevel_declarations:bool ->
  disable_legacy_soft_typehints:bool ->
  allow_new_attribute_syntax:bool ->
  disable_legacy_attribute_syntax:bool ->
  enable_xhp_class_modifier:bool ->
  disable_xhp_element_mangling:bool ->
  disallow_hash_comments:bool ->
  filename:Relative_path.t ->
  text:string ->
  Facts.facts option

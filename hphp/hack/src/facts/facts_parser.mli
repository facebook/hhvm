(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val flags_abstract : int

val flags_final : int

val from_text :
  php5_compat_mode:bool ->
  hhvm_compat_mode:bool ->
  disable_legacy_soft_typehints:bool ->
  allow_new_attribute_syntax:bool ->
  disable_legacy_attribute_syntax:bool ->
  enable_xhp_class_modifier:bool ->
  disable_xhp_element_mangling:bool ->
  mangle_xhp_mode:bool ->
  auto_namespace_map:(string * string) list ->
  filename:Relative_path.t ->
  text:string ->
  Facts.facts option

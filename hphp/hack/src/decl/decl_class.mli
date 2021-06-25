(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception Decl_heap_elems_bug

val map_property :
  Decl_defs.decl_class_type ->
  string ->
  Decl_defs.element ->
  Typing_defs.class_elt

val map_static_property :
  Decl_defs.decl_class_type ->
  string ->
  Decl_defs.element ->
  Typing_defs.class_elt

val map_method :
  Decl_defs.decl_class_type ->
  string ->
  Decl_defs.element ->
  Typing_defs.class_elt

val map_static_method :
  Decl_defs.decl_class_type ->
  string ->
  Decl_defs.element ->
  Typing_defs.class_elt

val map_constructor :
  Decl_defs.subst_context SMap.t ->
  Decl_defs.element option * Typing_defs.consistent_kind ->
  Typing_defs.class_elt option * Typing_defs.consistent_kind

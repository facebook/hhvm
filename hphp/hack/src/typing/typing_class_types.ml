(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type inherited_members = {
  consts: Typing_defs.class_const Lazy_string_table.t;
  typeconsts: Typing_defs.typeconst_type Lazy_string_table.t;
  props: Typing_defs.class_elt Lazy_string_table.t;
  sprops: Typing_defs.class_elt Lazy_string_table.t;
  methods: Typing_defs.class_elt Lazy_string_table.t;
  smethods: Typing_defs.class_elt Lazy_string_table.t;
  all_inherited_methods: Typing_defs.class_elt list Lazy_string_table.t;
  all_inherited_smethods: Typing_defs.class_elt list Lazy_string_table.t;
  typeconst_enforceability: (Pos_or_decl.t * bool) Lazy_string_table.t;
  construct:
    (Typing_defs.class_elt option * Typing_defs_core.consistent_kind) Lazy.t;
}

type eager_members = {
  props: Typing_defs.class_elt String.Table.t;
  static_props: Typing_defs.class_elt String.Table.t;
  methods: Typing_defs.class_elt String.Table.t;
  static_methods: Typing_defs.class_elt String.Table.t;
  construct:
    (Typing_defs.class_elt option * Typing_defs_core.consistent_kind) option ref;
}

(** class_t:
This type is an abstraction layer over the way folded decls are stored, and
provides a view of classes which includes all inherited members and their types.

This view is constructed by merging a single heap entry for the folded class
with many entries for the types of each of its members (those member entries are
looked up lazily, as needed). *)
type class_t = Decl_defs.decl_class_type * (eager_members[@opaque])
[@@deriving show]

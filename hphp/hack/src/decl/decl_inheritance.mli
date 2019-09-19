(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
module LSTable = Lazy_string_table

type inherited_members = {
  consts: class_const LSTable.t;
  typeconsts: typeconst_type LSTable.t;
  pu_enums: pu_enum_type LSTable.t;
  props: class_elt LSTable.t;
  sprops: class_elt LSTable.t;
  methods: class_elt LSTable.t;
  smethods: class_elt LSTable.t;
  all_inherited_methods: class_elt list LSTable.t;
  all_inherited_smethods: class_elt list LSTable.t;
  construct: (class_elt option * consistent_kind) Lazy.t;
}

val make : string -> (string -> decl_ty option) -> inherited_members

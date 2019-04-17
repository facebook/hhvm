(**
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
  methods : class_elt LSTable.t;
  smethods : class_elt LSTable.t;
  all_inherited_methods : class_elt list LSTable.t;
  all_inherited_smethods : class_elt list LSTable.t;
}

val make: string -> inherited_members

(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Decl_inheritance
open Typing_defs
open Shallow_decl_defs

module LSTable = Lazy_string_table

type lazy_class_type = {
  sc: shallow_class;
  ih: inherited_members;
  ancestors: decl ty LSTable.t;
  parents_and_traits: unit LSTable.t;
  members_fully_known: bool Lazy.t;
  req_ancestor_names: unit LSTable.t;
  all_requirements: (Pos.t * decl ty) list Lazy.t;
}

type class_type_variant =
  | Lazy of lazy_class_type
  | Eager of class_type

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type ancestor_caches = {
  ancestors: Typing_defs.decl_ty Lazy_string_table.t;
      (** Types of parents, interfaces, and traits *)
  parents_and_traits: unit Lazy_string_table.t;
      (** Names of parents and traits only *)
  members_fully_known: bool Lazy.t;
  req_ancestor_names: unit Lazy_string_table.t;
  all_requirements: (Pos.t * Typing_defs.decl_ty) Sequence.t;
  is_disposable: bool Lazy.t;
}

type class_name = string

val make : Provider_context.t -> class_name -> ancestor_caches

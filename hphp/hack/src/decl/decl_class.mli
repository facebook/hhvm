(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception Decl_heap_elems_bug

(** Fold together all the heap entries for a class
    (the class itself + its individual members)
    into a single data structure.
    The parameters might not contain all the member entries,
    in which case those are fetched from the member heaps. *)
val to_class_type :
  Decl_defs.decl_class_type * Decl_store.class_members option ->
  Typing_defs.class_type

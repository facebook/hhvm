(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Check if the class is already in the heap, and if not,
    declare it, its members and its ancestors and add them to
    their respective shared heaps.
    Return what has been added in the multiple heaps, i.e. the class
    heap entry and the entries in the various member heaps. There is no
    guarantee that it returns all the member heap entries for that class
    as some might have already been added previously when decling the ancestors. *)
val class_decl_if_missing :
  sh:SharedMem.uses ->
  Provider_context.t ->
  string ->
  (string * Decl_store.class_entries) option

val class_decl :
  sh:SharedMem.uses ->
  Provider_context.t ->
  Shallow_decl_defs.shallow_class ->
  parents:Decl_store.class_entries SMap.t ->
  Decl_defs.decl_class_type * Decl_store.class_members

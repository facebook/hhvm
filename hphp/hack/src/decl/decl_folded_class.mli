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
    their respective shared heaps. *)
val class_decl_if_missing :
  sh:SharedMem.uses ->
  Provider_context.t ->
  Nast.class_ ->
  string * Decl_defs.decl_class_type

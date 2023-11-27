(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Reordered_argument_collections

type get_classes_in_file = Relative_path.t -> SSet.t

type redo_type_decl_result = {
  fanout: Fanout.t;
  old_decl_missing_count: int;
      (** The number of old decls we didn't have when calculating the redecl. *)
}

(** Oldify any defs in [defs] which aren't already in
[previously_oldified_defs], then determines which symbols need to be
re-typechecked as a result of comparing the current versions of the symbols
to their old versions. *)
val redo_type_decl :
  Provider_context.t ->
  during_init:bool ->
  MultiWorker.worker list option ->
  bucket_size:int ->
  get_classes_in_file ->
  previously_oldified_defs:FileInfo.names ->
  defs:Naming_table.defs_per_file ->
  redo_type_decl_result

(** Mark all provided [defs] as old, as long as they were not previously
oldified. All definitions in [previously_oldified_defs] are then removed from
the decl heaps.

Typically, there are only any [previously_oldified_defs] during two-phase
redeclaration. *)
val oldify_decls_and_remove_descendants :
  Provider_context.t ->
  ?collect_garbage:bool ->
  MultiWorker.worker list option ->
  get_classes_in_file ->
  bucket_size:int ->
  defs:FileInfo.names ->
  unit

(** Remove the given old definitions from the decl heaps. *)
val remove_old_defs :
  Provider_context.t ->
  bucket_size:int ->
  MultiWorker.worker list option ->
  FileInfo.names ->
  unit

(** Exposed for tests only!
  For a set of classes, return all the declared classes that share their class
  elements (see Decl_class_elements).
  Not for general use case since it doesn't use lazy decl and makes sense only
  in a very particular use case of invalidate_type_decl. *)
val get_descendant_classes :
  Provider_context.t ->
  MultiWorker.worker list option ->
  bucket_size:int ->
  (Relative_path.t -> SSet.t) ->
  SSet.t ->
  SSet.t

(** Test-only *)
val remove_defs :
  FileInfo.names ->
  elems:Decl_class_elements.t SMap.t ->
  collect_garbage:bool ->
  unit

(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Reordered_argument_collections
open Typing_deps

val redo_type_decl :
  MultiWorker.worker list option ->
  bucket_size:int ->
  conservative_redecl:bool ->
  FileInfo.names ->
  Naming_table.fast ->
  Errors.t * DepSet.t * DepSet.t * DepSet.t

(**
 * Exposed for tests only!
 * For a set of classes, return all the declared classes that share their class
 * elements (see Decl_class_elements).
 * Not for general use case since it doesn't use lazy decl and makes sense only
 * in a very particular use case of invalidate_type_decl.
 *)
val get_dependent_classes :
  MultiWorker.worker list option ->
  bucket_size:int ->
  (Relative_path.t -> SSet.t) ->
  SSet.t ->
  SSet.t

val oldify_type_decl :
  ?collect_garbage:bool ->
  MultiWorker.worker list option ->
  (Relative_path.t -> SSet.t) ->
  bucket_size:int ->
  FileInfo.names ->
  FileInfo.names ->
  unit

val remove_defs :
  FileInfo.names ->
  Decl_class_elements.t SMap.t ->
  collect_garbage:bool ->
  unit

val remove_old_defs :
  bucket_size:int ->
  MultiWorker.worker list option ->
  FileInfo.names ->
  unit

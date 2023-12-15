(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Compare classes in files and deduce fanout. *)
val compute_class_fanout :
  Provider_context.t ->
  during_init:bool ->
  class_names:Decl_compare.VersionedSSet.diff ->
  old_decl_client_opt:Remote_old_decls_ffi.old_decl_client option ->
  Relative_path.t list ->
  Fanout.t

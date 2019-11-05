(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Compute the given AST for the given path, and return an updated [t]
containing that entry. *)
val update_context :
  ctx:Provider_context.t ->
  path:Relative_path.t ->
  file_input:ServerCommandTypes.file_input ->
  Provider_context.t * Provider_context.entry

(** Load the declarations of [t] into any global-memory storage, then call
[f], then unload those declarations. *)
val with_context : ctx:Provider_context.t -> f:(unit -> 'a) -> 'a

(** Compute TAST and error-list by doing typechecking/type inference for the
given entry in the given context. Note that you must call this function
within a [with_context], to ensure that all global state reflects the
contents of the context. *)
val compute_tast_and_errors :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  Tast.program * Errors.t

(** Find an existing entry within the context *)
val find_entry :
  ctx:Provider_context.t ->
  path:Relative_path.t ->
  Provider_context.entry option

(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Compute the TAST by doing typechecking/type inference for the given entry in
the given context. Note: this function uses Provider_context.with_context
in case a 'with_context' isn't already in force. *)
val compute_tast :
  ctx:Provider_context.t -> entry:Provider_context.entry -> Tast.program

(** Compute TAST and error-list by doing typechecking/type inference for the
given entry in the given context. Note: this function does not use
Provider_context.with_context, so if one isn't already in force, then it
won't be for the duration of this function. *)
val compute_tast_and_errors :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  Errors.t * Tast.program

(** Compute the given AST for the given path, and return an updated [t]
containing that entry. *)
val update_context :
  ctx:Provider_context.t ->
  path:Relative_path.t ->
  file_input:ServerCommandTypes.file_input ->
  Provider_context.t * Provider_context.entry

(** Load the declarations of [t] and call [f], then unload those declarations.
*)
val with_context : ctx:Provider_context.t -> f:(unit -> 'a) -> 'a

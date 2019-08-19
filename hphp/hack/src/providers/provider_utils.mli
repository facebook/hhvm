(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val compute_tast :
  ctx:Provider_context.t -> entry:Provider_context.entry -> Tast.program
(** Compute the TAST by doing typechecking/type inference for the given entry in
the given context. *)

val update_context :
  ctx:Provider_context.t ->
  path:Relative_path.t ->
  file_input:ServerCommandTypes.file_input ->
  Provider_context.t * Provider_context.entry
(** Compute the given AST for the given path, and return an updated [t]
containing that entry. *)

val with_context : ctx:Provider_context.t -> f:(unit -> 'a) -> 'a
(** Load the declarations of [t] and call [f], then unload those declarations.
*)

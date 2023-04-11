(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val do_ :
  command:string ->
  verbosity:int ->
  on_bad_command:(string -> 'a) ->
  unit ->
  Provider_context.t ->
  Tast.program ->
  unit

val default_db_dir : string

val create_handler :
  db_dir:string -> worker_id:int -> Provider_context.t -> Tast_visitor.handler

val patches_of_codemod_line :
  string ->
  ServerRenameTypes.patch list * string list * [ `ClassLike | `Function ]

module StandaloneApi : sig
  (* solve constraints from `db_dir` and log the solution to stdout *)
  val solve_persisted : db_dir:string -> unit

  (* For debugging: dump the constraints from `db_dir` to stdout *)
  val dump_persisted : db_dir:string -> unit
end

module ClientCheck : module type of Sdt_analysis_client_check

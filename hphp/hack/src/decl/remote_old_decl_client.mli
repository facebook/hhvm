(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val fetch_old_decls :
  ctx:Provider_context.t ->
  string list ->
  Shallow_decl_defs.shallow_class option SMap.t

module Utils : sig
  val get_dev_build_version : unit -> string

  val make_manifold_path : version:string -> string

  val name_to_decl_hash_opt :
    name:string -> db_path:Naming_sqlite.db_path -> string option

  val name_to_file_hash_opt :
    name:string -> db_path:Naming_sqlite.db_path -> string option

  val db_path_of_ctx : ctx:Provider_context.t -> Naming_sqlite.db_path option
end

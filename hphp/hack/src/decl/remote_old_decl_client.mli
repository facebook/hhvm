val fetch_old_decls :
  ctx:Provider_context.t ->
  string list ->
  Shallow_decl_defs.shallow_class option SMap.t

module Utils : sig
  val name_to_decl_hash_opt :
    name:string -> db_path:Naming_sqlite.db_path -> string option

  val db_path_of_ctx : ctx:Provider_context.t -> Naming_sqlite.db_path option
end

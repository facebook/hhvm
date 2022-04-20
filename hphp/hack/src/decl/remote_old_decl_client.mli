val fetch_old_decls :
  telemetry_label:string ->
  ctx:Provider_context.t ->
  string list ->
  Shallow_decl_defs.shallow_class option SMap.t

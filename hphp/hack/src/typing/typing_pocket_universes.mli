open Typing_defs

val expand_dep_ty :
  Typing_env_types.env ->
  ety_env:expand_env ->
  Reason.t ->
  locl_ty ->
  Ast_defs.id ->
  (Pos.t * string) * Aast_defs.pu_loc ->
  Ast_defs.id ->
  Typing_env_types.env * locl_phase ty

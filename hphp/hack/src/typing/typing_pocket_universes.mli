open Typing_defs

val expand_pocket_universes :
  Typing_env_types.env ->
  Reason.t ->
  locl_ty ->
  Ast_defs.id ->
  (Pos.t * string) * Aast_defs.pu_loc ->
  Ast_defs.id ->
  Typing_env_types.env * locl_phase ty

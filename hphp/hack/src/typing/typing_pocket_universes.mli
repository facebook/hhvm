open Typing_defs

val expand_pocket_universes :
  Typing_env_types.env ->
  Reason.t ->
  locl_ty ->
  Ast_defs.id ->
  locl_ty ->
  Ast_defs.id ->
  Typing_env_types.env * locl_ty option

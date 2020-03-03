open Typing_defs
open Typing_env_types

val get_tyvar_pu_access :
  env ->
  Typing_reason.t ->
  locl_ty ->
  Aast.sid ->
  Ident.t ->
  Aast.sid ->
  env * locl_ty

val make_all_pu_equal :
  env ->
  Ident.t ->
  internal_type ->
  on_error:Errors.typing_error_callback ->
  env

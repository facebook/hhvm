open Typing_defs
open Typing_env_types

(** Check that the method with signature ft_sub can be used to override
(is a subtype of) method with signature ft_super. *)
val subtype_method_decl :
  check_return:bool ->
  extra_info:Typing_subtype.reactivity_extra_info ->
  env ->
  Reason.t ->
  decl_fun_type ->
  Reason.t ->
  decl_fun_type ->
  Errors.typing_error_callback ->
  env

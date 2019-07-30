[@@@warning "-33"]
open Core_kernel
open Common
[@@@warning "+33"]
open Typing_defs

module Env = Typing_env

type method_instantiation =
{
  use_pos: Pos.t;
  use_name: string;
  explicit_targs: decl ty list;
}

type env = expand_env

val env_with_self:
  Env.env ->
  expand_env
val localize_with_self:
  Env.env ->
  decl ty ->
  Env.env * locl ty

val localize:
  ety_env:expand_env ->
  Env.env ->
  decl ty ->
  Env.env * locl ty
val localize_ft:
  ?instantiation:method_instantiation ->
  ety_env:expand_env ->
  Env.env ->
  decl fun_type ->
  Env.env * locl fun_type
val localize_hint_with_self:
  Env.env ->
  Aast.hint ->
  Env.env * locl ty
val localize_hint:
  ety_env:expand_env ->
  Env.env ->
  Aast.hint ->
  Env.env * locl ty
val localize_generic_parameters_with_bounds:
  ety_env:expand_env ->
  Env.env ->
  Nast.tparam list ->
  Env.env * (locl ty * Ast_defs.constraint_kind * locl ty) list
val localize_where_constraints:
  ety_env:expand_env ->
  Env.env ->
  Aast.where_constraint list ->
  Env.env
val localize_with_dty_validator:
  Env.env ->
  decl ty ->
  (env -> decl ty -> unit) ->
  Env.env * locl ty
val sub_type_decl:
  Env.env ->
  decl ty ->
  decl ty ->
  Errors.typing_error_callback ->
  unit
val unify_decl:
  Env.env ->
  decl ty ->
  decl ty ->
  Errors.typing_error_callback ->
  unit
val check_tparams_constraints:
  use_pos:Pos.t ->
  ety_env:expand_env ->
  Env.env ->
  decl tparam list ->
  Env.env
val check_where_constraints:
  in_class:bool ->
  use_pos:Pos.t ->
  ety_env:expand_env ->
  definition_pos:Pos.t ->
  Env.env ->
  decl where_constraint list ->
  Env.env
val decl:
  decl ty ->
  phase_ty
val locl:
  locl ty ->
  phase_ty

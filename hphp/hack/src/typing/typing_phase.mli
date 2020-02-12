[@@@warning "-33"]

open Hh_prelude
open Common

[@@@warning "+33"]

open Typing_defs
open Typing_env_types

type method_instantiation = {
  use_pos: Pos.t;
  use_name: string;
  explicit_targs: Tast.targ list;
}

val env_with_self : ?pos:Pos.t -> ?quiet:bool -> env -> expand_env

(** Transforms a declaration phase type ({!Typing_defs.decl_ty})
    into a localized type ({!Typing_defs.locl_ty} = {!Tast.ty}).
    Performs no substitutions of generics and initializes the late static bound
    type ({!Typing_defs.Tthis}) to the current class type (the type returned by
    {!get_self}).

    This is mostly provided as legacy support for {!AutocompleteService}, and
    should not be considered a general mechanism for transforming a {decl_ty} to
    a {!Tast.ty}.

    {!quiet} silences certain errors because those errors have already fired
    and/or are not appropriate at the time we call localize.
    *)
val localize_with_self :
  env -> ?pos:Pos.t -> ?quiet:bool -> decl_ty -> env * locl_ty

val localize_possibly_enforced_with_self :
  env -> decl_possibly_enforced_ty -> env * locl_possibly_enforced_ty

val localize : ety_env:expand_env -> env -> decl_ty -> env * locl_ty

val localize_ft :
  ?instantiation:method_instantiation ->
  ety_env:expand_env ->
  def_pos:Pos.t ->
  env ->
  decl_fun_type ->
  env * locl_fun_type

val localize_hint_with_self : env -> Aast.hint -> env * locl_ty

(* Declare and localize the type arguments to a constructor or function, given
 * information about the declared type parameters in `decl_tparam list`. If no
 * explicit type arguments are given, generate fresh type variables in their
 * place; do the same for any wildcard explicit type arguments.
 * Report arity errors using `def_pos` (for the declared parameters), `use_pos`
 * (for the use-site) and `use_name` (the name of the constructor or function).
 *)
val localize_targs :
  is_method:bool ->
  def_pos:Pos.t ->
  use_pos:Pos.t ->
  use_name:string ->
  env ->
  decl_tparam list ->
  Aast.hint list ->
  env * Tast.targ list

(* Declare and localize a single explicit type argument *)
val localize_targ : env -> Aast.hint -> env * Tast.targ

val localize_hint : ety_env:expand_env -> env -> Aast.hint -> env * locl_ty

val localize_generic_parameters_with_bounds :
  ety_env:expand_env ->
  env ->
  decl_tparam list ->
  env * (locl_ty * Ast_defs.constraint_kind * locl_ty) list

val localize_where_constraints :
  ety_env:expand_env -> env -> Aast.where_constraint list -> env

val sub_type_decl :
  env -> decl_ty -> decl_ty -> Errors.typing_error_callback -> env

val check_tparams_constraints :
  use_pos:Pos.t -> ety_env:expand_env -> env -> decl_tparam list -> env

val check_where_constraints :
  in_class:bool ->
  use_pos:Pos.t ->
  ety_env:expand_env ->
  definition_pos:Pos.t ->
  env ->
  decl_where_constraint list ->
  env

val decl : decl_ty -> phase_ty

val locl : locl_ty -> phase_ty

val resolve_type_arguments_and_check_constraints :
  exact:exact ->
  check_constraints:bool ->
  def_pos:Pos.t ->
  use_pos:Pos.t ->
  env ->
  Ast_defs.id ->
  Nast.class_id_ ->
  decl_tparam sexp_list ->
  Aast.hint sexp_list ->
  env * locl_ty * Tast.targ sexp_list

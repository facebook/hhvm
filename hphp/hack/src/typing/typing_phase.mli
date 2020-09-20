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

val env_with_self :
  ?pos:Pos.t -> ?quiet:bool -> ?report_cycle:Pos.t * string -> env -> expand_env

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
  env ->
  ?pos:Pos.t ->
  ?quiet:bool ->
  ?report_cycle:Pos.t * string ->
  decl_ty ->
  env * locl_ty

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
  check_well_kinded:bool ->
  is_method:bool ->
  def_pos:Pos.t ->
  use_pos:Pos.t ->
  use_name:string ->
  env ->
  decl_tparam list ->
  Aast.hint list ->
  env * Tast.targ list

(** Like localize_targs, but acts on kinds. *)
val localize_targs_with_kinds :
  check_well_kinded:bool ->
  is_method:bool ->
  def_pos:Pos.t ->
  use_pos:Pos.t ->
  use_name:string ->
  ?tparaml:decl_tparam list ->
  env ->
  Typing_kinding_defs.Simple.named_kind list ->
  Aast.hint list ->
  env * Tast.targ list

(* Declare and localize a single explicit type argument *)
val localize_targ :
  check_well_kinded:bool -> env -> Aast.hint -> env * Tast.targ

val localize_hint : ety_env:expand_env -> env -> Aast.hint -> env * locl_ty

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

val localize_targs_and_check_constraints :
  exact:exact ->
  check_well_kinded:bool ->
  check_constraints:bool ->
  def_pos:Pos.t ->
  use_pos:Pos.t ->
  env ->
  Ast_defs.id ->
  Nast.class_id_ ->
  decl_tparam list ->
  Aast.hint list ->
  env * locl_ty * Tast.targ list

(* Add generic parameters to the environment, with localized bounds,
 * and also add any consequences of `where` constraints *)
val localize_and_add_generic_parameters :
  Pos.t -> env -> decl_tparam list -> env

(* As above but from AST of generic parameters *)
val localize_and_add_ast_generic_parameters_and_where_constraints :
  Pos.t ->
  env ->
  (Pos.t, Nast.func_body_ann, unit, unit) Aast.tparam list ->
  (Aast.hint * Ast_defs.constraint_kind * Aast.hint) list ->
  env

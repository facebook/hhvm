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

(**
 Take a declaration phase type and use [ety_env] to transform it into a localized type.

 - Desugar [Tmixed], [Toption] and [Tlike] types into unions.

 - Transform [Tapply] to [Tclass] (for classish types), or [Tnewtype] (for opaque
   type defs outside their defining file), else expand aliases or opaque
   types within their defining file.

 - Replace [Tthis] by [ety_env.this_ty].

 - Replace [Tgeneric] by the type specified in [ety_env.substs], if present;
   otherwise leave as generic.

 - Expand [Taccess] type constant accesses through concrete classes.

 Use this function to localize types outside the body of the method being
 type-checked whose generic parameters and [this] type must be instantiated to
 explicit types or fresh type variables. Use [localize_no_subst] instead for types
 whose generic parameters and [this] type should be left as is.

 The following errors are detected during localization:

 - Bad type constant access e.g. [AbstractClass::T]

 - Cyclic types

 Errors are handled using [ety_env.on_error]. If you want to ignore errors
 during localization, set this to [Errors.ignore_error]. *)
val localize :
  ety_env:expand_env ->
  env ->
  decl_ty ->
  (env * Typing_error.t option) * locl_ty

(**
 Transform a declaration phase type into a localized type, with no substitution
 for generic parameters and [this].

 [ignore_errors] silences errors because those errors have already fired
 and/or are not appropriate at the time we call localize. *)
val localize_no_subst :
  env ->
  ignore_errors:bool ->
  decl_ty ->
  (env * Typing_error.t option) * locl_ty

(**
 Transform a type hint into a localized type, with no substitution for generic
 parameters and [this].

 [ignore_errors] silences errors because those errors have already fired
 and/or are not appropriate at the time we call localize. *)
val localize_hint_no_subst :
  env ->
  ignore_errors:bool ->
  ?report_cycle:Pos.t * string ->
  Aast.hint ->
  (env * Typing_error.t option) * locl_ty

val localize_hint_for_refinement :
  env -> Aast.hint -> (env * Typing_error.t option) * locl_ty

val localize_hint_for_lambda :
  env -> Aast.hint -> (env * Typing_error.t option) * locl_ty

val localize_ft :
  ?instantiation:method_instantiation ->
  ety_env:expand_env ->
  def_pos:Pos_or_decl.t ->
  env ->
  decl_fun_type ->
  (env * Typing_error.t option) * locl_fun_type

(** Declare and localize the type arguments to a constructor or function, given
    information about the declared type parameters in `decl_tparam list`. If no
    explicit type arguments are given, generate fresh type variables in their
    place; do the same for any wildcard explicit type arguments.
    Report arity errors using `def_pos` (for the declared parameters), `use_pos`
    (for the use-site) and `use_name` (the name of the constructor or function). *)
val localize_targs :
  check_well_kinded:bool ->
  is_method:bool ->
  def_pos:Pos_or_decl.t ->
  use_pos:Pos.t ->
  use_name:string ->
  ?check_explicit_targs:bool ->
  env ->
  decl_tparam list ->
  Aast.hint list ->
  (env * Typing_error.t option) * Tast.targ list

(** Like [localize_targs], but acts on kinds. *)
val localize_targs_with_kinds :
  check_well_kinded:bool ->
  is_method:bool ->
  def_pos:Pos_or_decl.t ->
  use_pos:Pos.t ->
  use_name:string ->
  ?check_explicit_targs:bool ->
  ?tparaml:decl_tparam list ->
  env ->
  Typing_kinding_defs.Simple.named_kind list ->
  Aast.hint list ->
  (env * Typing_error.t option) * Tast.targ list

(** Same as [localize_targs] but also check constraints on type parameters
    (though not `where` constraints) *)
val localize_targs_and_check_constraints :
  exact:exact ->
  check_well_kinded:bool ->
  def_pos:Pos_or_decl.t ->
  use_pos:Pos.t ->
  ?check_explicit_targs:bool ->
  env ->
  Ast_defs.id ->
  Typing_reason.t ->
  decl_tparam list ->
  Aast.hint list ->
  (env * Typing_error.t option) * locl_ty * Tast.targ list

(** Declare and localize a single explicit type argument *)
val localize_targ :
  ?tparam:decl_tparam ->
  check_well_kinded:bool ->
  env ->
  Aast.hint ->
  (env * Typing_error.t option) * Tast.targ

val sub_type_decl :
  env ->
  decl_ty ->
  decl_ty ->
  Typing_error.Reasons_callback.t option ->
  env * Typing_error.t option

(** Are two decl types definitely subtypes of each other? *)
val is_sub_type_decl :
  ?coerce:Typing_logic.coercion_direction option ->
  env ->
  decl_ty ->
  decl_ty ->
  bool

(** Add some [as] or [super] constraint to the environment.
    Raise an error if any inconsistency is detected. *)
val check_tparams_constraints :
  use_pos:Pos.t ->
  ety_env:expand_env ->
  env ->
  decl_tparam list ->
  env * Typing_error.t option

(** Add some [where] constraints to the environment.
    Raise an error if any inconsistency is detected. *)
val check_where_constraints :
  in_class:bool ->
  use_pos:Pos.t ->
  ety_env:expand_env ->
  definition_pos:Pos_or_decl.t ->
  env ->
  decl_where_constraint list ->
  env * Typing_error.t option

val decl : decl_ty -> phase_ty

val locl : locl_ty -> phase_ty

(** Add generic parameters to the environment, with localized bounds,
    and also add any consequences of `where` constraints *)
val localize_and_add_generic_parameters_and_where_constraints :
  ety_env:expand_env ->
  env ->
  decl_tparam list ->
  decl_where_constraint list ->
  env * Typing_error.t option

(** Add generic parameters to the environment, with localized bounds,
    and also add any consequences of `where` constraints.

    {!ignore_errors} silences errors because those errors may have already fired
    during another localization and/or are not appropriate at the time we call localize. *)
val localize_and_add_ast_generic_parameters_and_where_constraints :
  env ->
  ignore_errors:bool ->
  Nast.tparam list ->
  (Aast.hint * Ast_defs.constraint_kind * Aast.hint) list ->
  env * Typing_error.t option

open Typing_defs
module Env = Typing_env
module Phase = Typing_phase
module ITySet = Internal_type_set
module Utils = Typing_utils

(** Make tconstty (usually a type variable representing the type constant of
another type variable v) equal to ty::tconstid (where ty is usually a bound
of v) *)
let make_type_const_equal
    env tconstty (ty : internal_type) tconstid ~on_error ~as_tyvar_with_cnstr =
  let rec make_equal env ty =
    match ty with
    | LoclType ty ->
      let ety_env = Phase.env_with_self env in
      let (env, tytconst) =
        Utils.expand_typeconst
          ety_env
          env
          ~as_tyvar_with_cnstr
          ty
          tconstid
          ~root_pos:(get_pos ty)
          ~on_error
          ~allow_abstract_tconst:true
          ~ignore_errors:as_tyvar_with_cnstr
      in
      let error = Errors.type_constant_mismatch on_error in
      let env = Utils.sub_type env tytconst tconstty error in
      let env = Utils.sub_type env tconstty tytconst error in
      env
    | ConstraintType ty ->
      (match deref_constraint_type ty with
      | (_, Thas_member _)
      | (_, Tdestructure _) ->
        env
      | (_, TCunion (lty, cty))
      | (_, TCintersection (lty, cty)) ->
        (* This not quite correct but works for now since no constraint type has any
        type constant. The proper way to do it would be to have Utils.expand_typeconst
        work on constraint types directly. *)
        let env = make_equal env (LoclType lty) in
        let env = make_equal env (ConstraintType cty) in
        env)
  in
  make_equal env ty

(** Add a type constant with id `tyconstid` and type `ty` to a type variable,
and propagate constraints to all type constants `tyconstid` of upper bounds and
lower bounds. *)
let add_tyvar_type_const env var tconstid ty ~on_error =
  let env = Env.set_tyvar_type_const env var tconstid ty in
  let upper_bounds = Env.get_tyvar_upper_bounds env var in
  let env =
    ITySet.fold
      (fun bound env ->
        make_type_const_equal
          env
          ty
          bound
          tconstid
          ~on_error
          ~as_tyvar_with_cnstr:true)
      upper_bounds
      env
  in
  let lower_bounds = Env.get_tyvar_lower_bounds env var in
  ITySet.fold
    (fun bound env ->
      make_type_const_equal
        env
        ty
        bound
        tconstid
        ~on_error
        ~as_tyvar_with_cnstr:false)
    lower_bounds
    env

(** For all type constants T of var, make its type equal to ty::T *)
let make_all_type_consts_equal
    env var (ty : internal_type) ~on_error ~as_tyvar_with_cnstr =
  SMap.fold
    (fun _ (tconstid, tconstty) env ->
      make_type_const_equal
        env
        tconstty
        ty
        tconstid
        ~on_error
        ~as_tyvar_with_cnstr)
    (Env.get_tyvar_type_consts env var)
    env

(** `p` is the position where var::tconstid was encountered. *)
let get_tyvar_type_const env var ((pos, tconstid_) as tconstid) =
  match Env.get_tyvar_type_const env var tconstid with
  | Some (_pos, ty) -> (env, ty)
  | None ->
    let (env, tvar) = Env.fresh_invariant_type_var env pos in
    Typing_log.log_new_tvar_for_tconst env pos var tconstid_ tvar;
    let env =
      add_tyvar_type_const env var tconstid tvar (Errors.unify_error_at pos)
    in
    (env, tvar)

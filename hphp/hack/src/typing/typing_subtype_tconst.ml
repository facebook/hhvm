open Hh_prelude
open Typing_defs
module Env = Typing_env
module ITySet = Internal_type_set
module Utils = Typing_utils

(** Make tconstty (usually a type variable representing the type constant of
another type variable v) equal to ty::tconstid (where ty is usually a bound
of v) *)
let make_type_const_equal
    env
    tconstty
    (ty : internal_type)
    tconstid
    ~(on_error : Typing_error.Reasons_callback.t option)
    ~as_tyvar_with_cnstr =
  let rec make_equal env ty =
    match ty with
    | LoclType ty ->
      let ety_env = empty_expand_env in
      let ((env, e1), tytconst) =
        Utils.expand_typeconst
          ety_env
          env
          ~as_tyvar_with_cnstr
          ty
          tconstid
          ~root_pos:(get_pos ty)
          ~allow_abstract_tconst:true
          ~ignore_errors:(Option.is_some as_tyvar_with_cnstr)
      in
      let error =
        Option.map
          ~f:Typing_error.Reasons_callback.type_constant_mismatch
          on_error
      in
      let (env, e2) = Utils.sub_type env tytconst tconstty error in
      let (env, e3) = Utils.sub_type env tconstty tytconst error in
      (env, Typing_error.multiple_opt @@ List.filter_map ~f:Fn.id [e1; e2; e3])
    | ConstraintType ty ->
      (match deref_constraint_type ty with
      | (_, Thas_member _)
      | (_, Tdestructure _) ->
        (env, None)
        (* This not quite correct but works for now since no constraint type has any
           type constant. The proper way to do it would be to have Utils.expand_typeconst
           work on constraint types directly. *)
      | (_, TCunion (lty, cty)) ->
        let (env, e1) = make_equal env (LoclType lty) in
        let (env, e2) = make_equal env (ConstraintType cty) in
        (env, Option.merge e1 e2 ~f:(fun e1 e2 -> Typing_error.union [e1; e2]))
      | (_, TCintersection (lty, cty)) ->
        let (env, e1) = make_equal env (LoclType lty) in
        let (env, e2) = make_equal env (ConstraintType cty) in
        (* TODO: should we be taking the intersection of the errors? *)
        ( env,
          Option.merge e1 e2 ~f:(fun e1 e2 -> Typing_error.multiple [e1; e2]) ))
  in
  make_equal env ty

(** Add a type constant with id `tyconstid` and type `ty` to a type variable,
and propagate constraints to all type constants `tyconstid` of upper bounds and
lower bounds. *)
let add_tyvar_type_const env var tconstid ty ~on_error =
  let env = Env.set_tyvar_type_const env var tconstid ty in
  let var_pos = Env.get_tyvar_pos env var in
  let upper_bounds = Env.get_tyvar_upper_bounds env var in
  let (env, upper_ty_errs) =
    ITySet.fold
      (fun bound (env, ty_errs) ->
        match
          make_type_const_equal
            env
            ty
            bound
            tconstid
            ~on_error
            ~as_tyvar_with_cnstr:(Some var_pos)
        with
        | (env, Some ty_err) -> (env, ty_err :: ty_errs)
        | (env, _) -> (env, ty_errs))
      upper_bounds
      (env, [])
  in
  let lower_bounds = Env.get_tyvar_lower_bounds env var in
  let (env, ty_errs) =
    ITySet.fold
      (fun bound (env, ty_errs) ->
        match
          make_type_const_equal
            env
            ty
            bound
            tconstid
            ~on_error
            ~as_tyvar_with_cnstr:None
        with
        | (env, Some ty_err) -> (env, ty_err :: ty_errs)
        | (env, _) -> (env, ty_errs))
      lower_bounds
      (env, upper_ty_errs)
  in
  (env, Typing_error.multiple_opt ty_errs)

(** For all type constants T of var, make its type equal to ty::T *)
let make_all_type_consts_equal
    env var (ty : internal_type) ~on_error ~as_tyvar_with_cnstr =
  let var_pos = Env.get_tyvar_pos env var in
  let as_tyvar_with_cnstr =
    if as_tyvar_with_cnstr then
      Some var_pos
    else
      None
  in
  let (env, ty_errs) =
    SMap.fold
      (fun _ (tconstid, tconstty) (env, ty_errs) ->
        match
          make_type_const_equal
            env
            tconstty
            ty
            tconstid
            ~on_error
            ~as_tyvar_with_cnstr
        with
        | (env, Some ty_err) -> (env, ty_err :: ty_errs)
        | (env, _) -> (env, ty_errs))
      (Env.get_tyvar_type_consts env var)
      (env, [])
  in
  (env, Typing_error.multiple_opt ty_errs)

(** `p` is the position where var::tconstid was encountered. *)
let get_tyvar_type_const env var tconstid ~on_error =
  match Env.get_tyvar_type_const env var tconstid with
  | Some (_pos, ty) -> ((env, None), ty)
  | None ->
    let var_pos = Env.get_tyvar_pos env var in
    let (env, tvar) = Env.fresh_type_invariant env var_pos in
    Typing_log.log_new_tvar_for_tconst env (var_pos, var) tconstid tvar;
    let env = add_tyvar_type_const env var tconstid tvar ~on_error in
    (env, tvar)

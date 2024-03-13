open Hh_prelude
open Typing_defs
module Env = Typing_env
module ITySet = Internal_type_set
module Utils = Typing_utils

(** Make `tvar` equal to `ty::tconstid`. The locl type `tvar` is a
type variable that represents the the type constant ::tconstid of
another variable v. See get_tyvar_type_const below, where such type
variables are created. *)
let make_type_const_equal
    env
    tvar
    (ty : internal_type)
    tconstid
    ~(on_error : Typing_error.Reasons_callback.t option)
    ~as_tyvar_with_cnstr =
  let subtype_error =
    Option.map ~f:Typing_error.Reasons_callback.type_constant_mismatch on_error
  in
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
          ~allow_abstract_tconst:true
          ~ignore_errors:(Option.is_some as_tyvar_with_cnstr)
      in
      let (env, e2) = Utils.sub_type env tytconst tvar subtype_error in
      let (env, e3) = Utils.sub_type env tvar tytconst subtype_error in
      (env, Typing_error.multiple_opt @@ List.filter_map ~f:Fn.id [e1; e2; e3])
    | ConstraintType ty ->
      (match deref_constraint_type ty with
      | (r, Thas_type_member { htm_id; htm_upper = up_ty; htm_lower = lo_ty })
        when String.equal htm_id (snd tconstid) ->
        let (env, e) = Utils.sub_type env up_ty lo_ty on_error in
        (match e with
        | Some _ ->
          let err_opt =
            Option.map
              ~f:(fun on_error ->
                Typing_error.(
                  apply_reasons
                    ~on_error
                    (Secondary.Inexact_tconst_access (Reason.to_pos r, tconstid))))
              on_error
          in
          (env, err_opt)
        | None ->
          (* We constrain the upper and lower bounds to be equal
           * between themselves and with the type variable. *)
          let (env, e1) = Utils.sub_type env up_ty tvar subtype_error in
          let (env, e2) = Utils.sub_type env tvar lo_ty subtype_error in
          (env, Option.merge ~f:Typing_error.both e1 e2))
      | (_, Thas_type_member _)
      | (_, Thas_member _)
      | (_, Tcan_index _)
      | (_, Tcan_traverse _)
      | (_, Ttype_switch _)
      | (_, Tdestructure _) ->
        (env, None)
      | (_, TCunion (lty, cty)) ->
        let (env, e1) = make_equal env (LoclType lty) in
        let (env, e2) = make_equal env (ConstraintType cty) in
        (env, Option.merge e1 e2 ~f:(fun e1 e2 -> Typing_error.union [e1; e2]))
      | (_, TCintersection (lty, cty)) ->
        let (env, e1) = make_equal env (LoclType lty) in
        let (env, e2) = make_equal env (ConstraintType cty) in
        (* TODO: should we be taking the intersection of the errors? *)
        (env, Option.merge ~f:Typing_error.both e1 e2))
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
  let is_mixed =
    match ty with
    | LoclType lty ->
      let (_is_supportdyn, env, lty) = Utils.strip_supportdyn env lty in
      Utils.is_mixed env lty
    | _ -> false
  in
  if is_mixed then
    (env, None)
  else
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

module Env = Typing_env
module Phase = Typing_phase
module TySet = Typing_set
module Utils = Typing_utils

(** Make tconstty (usually a type variable representing the type constant of
another type variable v) equal to ty::tconstid (where ty is usually a bound
of v) *)
let make_type_const_equal env tconstty ty tconstid ~as_tyvar_with_cnstr =
  let ety_env = Phase.env_with_self env in
  let (env, (_ety_env, tytconst)) =
    Utils.expand_typeconst ety_env env ~as_tyvar_with_cnstr (fst ty) ty
      [tconstid] in
  let env = Utils.sub_type env tytconst tconstty in
  let env = Utils.sub_type env tconstty tytconst in
  env

(** Add a type constant with id `tyconstid` and type `ty` to a type variable,
and propagate constraints to all type constants `tyconstid` of upper bounds and
lower bounds. *)
let add_tyvar_type_const env var tconstid ty =
  let env = Env.set_tyvar_type_const env var tconstid ty in
  let upper_bounds = Env.get_tyvar_upper_bounds env var in
  let env = TySet.fold
    (fun bound env -> make_type_const_equal env ty bound tconstid
      ~as_tyvar_with_cnstr:true)
    upper_bounds env in
  let lower_bounds = Env.get_tyvar_lower_bounds env var in
  TySet.fold
    (fun bound env -> make_type_const_equal env ty bound tconstid
      ~as_tyvar_with_cnstr:false)
    lower_bounds env


(** For all type constants T of var, make its type equal to ty::T *)
let make_all_type_consts_equal env var ty ~as_tyvar_with_cnstr =
  SMap.fold
    (fun _ (tconstid, tconstty) env ->
      make_type_const_equal env tconstty ty tconstid ~as_tyvar_with_cnstr)
    (Env.get_tyvar_type_consts env var)
    env


(** `p` is the position where var::tconstid was encountered. *)
let get_tyvar_type_const env var (pos, tconstid_ as tconstid) =
  match Env.get_tyvar_type_const env var tconstid with
  | Some (_pos, ty) -> env, ty
  | None ->
    let env, tvar = Env.fresh_invariant_type_var env pos in
    Typing_log.log_new_tvar_for_tconst env pos var tconstid_ tvar;
    let env = add_tyvar_type_const env var tconstid tvar in
    env, tvar

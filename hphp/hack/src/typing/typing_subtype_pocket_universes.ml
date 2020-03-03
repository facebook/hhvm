open Typing_defs
module Env = Typing_env
module Phase = Typing_phase
module ITySet = Internal_type_set
module Utils = Typing_utils

type result =
  | Ignore
  | Expand of (Reason.t * ((Pos.t * string) * Aast_defs.pu_loc))
  | Var of Ident.t

(* Make new_ty (a type variable representing a pocket universe dependent type)
 * equal to base:@enum:@ty:@name (where ty can be a Tgeneric, or an atom).
 * If ty is an atom, we can simplify the whole type to the actual definition.
 * If ty is a Tgeneric, we can't simplify at all.
 * debug: ty must never be a Tvar because we got rid of these in Typing_phase,
 *        and we internally never generate such a case. I'd like to make
 *        sure so I'm leaving a failwith during the review / test process
 *)
let make_pocket_universes_type_equal
    env new_ty (ty : internal_type) base enum name ~on_error =
  let rec make_equal env ty =
    match ty with
    | LoclType lty ->
      let result =
        match deref lty with
        | (r, Tprim (Aast_defs.Tatom atom)) ->
          Expand (r, ((Reason.to_pos r, atom), Aast_defs.Atom))
        | (r, Tgeneric s) ->
          Expand (r, ((Reason.to_pos r, s), Aast_defs.TypeParameter))
        | (_, Tvar v) -> Var v
        | _ -> Ignore
      in
      (match result with
      | Ignore -> env
      | Expand (reason, member) ->
        let ety_env = Phase.env_with_self env in
        let (env, pu_ty) =
          Utils.expand_pocket_universes env ety_env reason base enum member name
        in
        (* TODO update error code *)
        let error = Errors.type_constant_mismatch on_error in
        let env = Utils.sub_type env pu_ty new_ty error in
        let env = Utils.sub_type env new_ty pu_ty error in
        env
      | Var _ -> failwith "not sure what to do here, see you later aligator")
    | ConstraintType ty ->
      (* see typing_subtype_tconst.ml *)
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

let add_tyvar_pu_access env var base enum name new_var ~on_error =
  let env = Env.set_tyvar_pu_access env var base enum new_var name in
  let upper_bounds = Env.get_tyvar_upper_bounds env var in
  let env =
    ITySet.fold
      (fun bound env ->
        make_pocket_universes_type_equal
          env
          new_var
          bound
          base
          enum
          name
          ~on_error)
      upper_bounds
      env
  in
  let lower_bounds = Env.get_tyvar_lower_bounds env var in
  ITySet.fold
    (fun bound env ->
      make_pocket_universes_type_equal
        env
        new_var
        bound
        base
        enum
        name
        ~on_error)
    lower_bounds
    env

let make_all_pu_equal env var (ty : internal_type) ~on_error =
  SMap.fold
    (fun _ (base, enum, new_var, name) env ->
      make_pocket_universes_type_equal env new_var ty base enum name ~on_error)
    (Env.get_tyvar_pu_accesses env var)
    env

let get_tyvar_pu_access env reason base enum var name =
  let pos = Reason.to_pos @@ reason in
  match Env.get_tyvar_pu_access env var name with
  | Some (base', enum', ty, name') ->
    (* Checking syntactic equality *)
    if
      Typing_defs.ty_equal base base'
      && String.equal (snd enum) (snd enum')
      && String.equal (snd name) (snd name')
      (* this one should be useless *)
    then
      (env, ty)
    else
      failwith "TODO(T36532263) support multiple hiding"
  | None ->
    let on_error = Errors.unify_error_at pos in
    let (env, new_var) = Env.fresh_invariant_type_var env pos in
    (* TODO(T35357243) add some logging *)
    let env = add_tyvar_pu_access env var base enum name new_var ~on_error in
    (env, new_var)

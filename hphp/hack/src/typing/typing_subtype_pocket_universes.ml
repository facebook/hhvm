open Typing_defs
module Env = Typing_env
module ITySet = Internal_type_set
module TUtils = Typing_utils

(* Make new_ty (a type variable representing a pocket universe dependent type)
 * equal to ty:@name (where ty can be a Tgeneric, or an atom).
 * If ty is an atom, we can simplify the whole type to the actual definition.
 * If ty is a Tgeneric, we can't simplify at all.
 *)
let make_pocket_universes_type_equal
    env new_ty (ty : internal_type) base enum name ~on_error =
  let error = Errors.type_constant_mismatch on_error in
  let reason = get_reason_i ty in
  let rec make_equal env ty =
    match ty with
    | LoclType lty ->
      let (env, opt_pu_ty) =
        TUtils.expand_pocket_universes env reason base enum lty name
      in
      (match opt_pu_ty with
      | Some pu_ty ->
        let env = TUtils.sub_type env pu_ty new_ty error in
        let env = TUtils.sub_type env new_ty pu_ty error in
        env
      | None -> env)
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
  let env = Env.set_tyvar_pu_access env var name new_var in
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

let extract_pu_from_upper_bounds upper_bounds =
  ITySet.fold
    (fun ity acc ->
      match ity with
      | ConstraintType _ -> acc
      | LoclType lty ->
        (match get_node lty with
        | Tpu (base, env) -> (base, env) :: acc
        | _ -> acc))
    upper_bounds
    []

let make_all_pu_equal env var (ty : internal_type) ~on_error =
  let var_uppers = Env.get_tyvar_upper_bounds env var in
  let upper_pus = extract_pu_from_upper_bounds var_uppers in
  match upper_pus with
  | [(base, enum)] ->
    SMap.fold
      (fun _ (name, new_var) env ->
        make_pocket_universes_type_equal env new_var ty base enum name ~on_error)
      (Env.get_tyvar_pu_accesses env var)
      env
  | [] -> env
  | _ :: _ ->
    let pos = Env.get_tyvar_pos env var in
    Errors.pu_typing_invalid_upper_bounds pos;
    env

let get_tyvar_pu_access env reason var name =
  let pos = Reason.to_pos reason in
  let uppers = Env.get_tyvar_upper_bounds env var in
  let upper_pus = extract_pu_from_upper_bounds uppers in
  match upper_pus with
  | [(base, enum)] ->
    (match Env.get_tyvar_pu_access env var name with
    | Some (name', ty) ->
      assert (String.equal (snd name) (snd name'));
      (env, ty)
    | None ->
      let on_error = Errors.unify_error_at pos in
      let (env, new_var) = Env.fresh_invariant_type_var env pos in
      let () =
        Typing_log.log_new_tvar_for_pu_access env pos var (snd name) new_var
      in
      let env = add_tyvar_pu_access env var base enum name new_var ~on_error in
      (env, new_var))
  | _ ->
    Errors.pu_typing_invalid_upper_bounds pos;
    (env, TUtils.terr env reason)

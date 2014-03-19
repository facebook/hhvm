(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Utils
open Typing_defs
open Silent

module Env = Typing_env
module TUtils = Typing_utils
module TDef = Typing_tdef
module Inst = Typing_instantiate

let rec unify env ty1 ty2 =
  if ty1 == ty2 then env, ty1 else
  match ty1, ty2 with
  | (_, Tany), ty | ty, (_, Tany) -> env, ty
  | (r1, Tvar n1), (r2, Tvar n2) -> unify_var env r1 n1 r2 n2
  | (r, Tvar n), ty2
  | ty2, (r, Tvar n) ->
      let env, ty1 = Env.get_type env n in
      let n' = Env.fresh() in
      let env = Env.rename env n n' in
      let env, ty = unify env ty1 ty2 in
      let env = Env.add env n ty in
      env, (r, Tvar n')
  | (r1, Tunresolved tyl1), (r2, Tunresolved tyl2) ->
      let r = unify_reason r1 r2 in
      let env, tyl = TUtils.normalize_inter env tyl1 tyl2 in
      env, (r, Tunresolved tyl)
  | (_, Tunresolved tyl), (_, ty_ as ty)
  | (_, ty_ as ty), (_, Tunresolved tyl) ->
      let p1 = try TUtils.find_pos tyl with _ -> Pos.none in
      let str_ty = Typing_print.error ty_ in
      let r = Reason.Rcoerced (p1, env.Env.pos, str_ty) in
      let env = List.fold_left (fun env x -> TUtils.sub_type env ty x) env tyl in
      env, (r, ty_)
  | (r1, Toption ty1), (r2, Toption ty2) ->
      let r = unify_reason r1 r2 in
      let env, ty = unify env ty1 ty2 in
      env, (r, Toption ty)
  | (r1, (Tprim Nast.Tvoid as ty1')), (r2, (Toption ty as ty2')) ->
     (* When we are in async functions, we allow people to write Awaitable<void>
      * and then do yield result(null) *)
      if Env.allow_null_as_void env
      then unify env ty1 ty
      else if !is_silent_mode
      then env, (r1, ty1')
      else TUtils.uerror r1 ty1' r2 ty2'
  (* It might look like you can combine the next two cases, but you can't --
   * if both sides are a Tapply the "when" guard will only check ty1, so if ty2
   * is a typedef it won't get expanded. So we need an explicit check for both.
   *)
  | (r, Tapply ((_, x), argl)), ty2 when Typing_env.is_typedef env x ->
      let env, ty1 = TDef.expand_typedef SSet.empty env r x argl in
      unify env ty1 ty2
  | ty2, (r, Tapply ((_, x), argl)) when Typing_env.is_typedef env x ->
      let env, ty1 = TDef.expand_typedef SSet.empty env r x argl in
      unify env ty1 ty2
  | (r1, ty1), (r2, ty2) ->
      let r = unify_reason r1 r2 in
      let env, ty = unify_ env r1 ty1 r2 ty2 in
      env, (r, ty)

and unify_var env r1 n1 r2 n2 =
  let r = unify_reason r1 r2 in
  let env, n1 = Env.get_var env n1 in
  let env, n2 = Env.get_var env n2 in
  if n1 = n2 then env, (r, Tvar n1) else
  let env, ty1 = Env.get_type_unsafe env n1 in
  let env, ty2 = Env.get_type_unsafe env n2 in
  let n' = Env.fresh() in
  let env = Env.rename env n1 n' in
  let env = Env.rename env n2 n' in
  let env, ty = unify env ty1 ty2 in
  (* I ALWAYS FORGET THIS! ALWAYS!!! *)
  (* The type of n' could have changed because of recursive types *)
  (* We need one more round *)
  let env, ty' = Env.get_type env n' in
  let env, ty = unify env ty ty' in
  let env = Env.add env n' ty in
  env, (r, Tvar n')

and unify_ env r1 ty1 r2 ty2 =
  match ty1, ty2 with
  | Tprim x, Tprim y ->
      (match x, y with
      | x, y when x = y ->
	  env, Tprim x
      | _ when !is_silent_mode ->
          env, Tany
      | _ ->
          TUtils.uerror r1 ty1 r2 ty2
      )
  | Tarray (_, None, None), (Tarray _ as ty)
  | (Tarray _ as ty), Tarray (_, None, None) ->
      env, ty
  | Tarray (b1, Some ty1, None), Tarray (b2, Some ty2, None) ->
      let is_local = b1 && b2 in
      let env, ty = unify env ty1 ty2 in
      env, Tarray (is_local, Some ty, None)
  | Tarray (b1, Some ty1, Some ty2), Tarray (b2, Some ty3, Some ty4) ->
      let is_local = b1 && b2 in
      let env, ty1 = unify env ty1 ty3 in
      let env, ty2 = unify env ty2 ty4 in
      env, Tarray (is_local, Some ty1, Some ty2)
  | Tfun ft1, Tfun ft2 ->
      let env, ft1 = Inst.instantiate_ft env ft1 in
      let env, ft2 = Inst.instantiate_ft env ft2 in
      let env, ft = unify_funs env r1 ft1 r2 ft2 in
      env, Tfun ft
  | Tapply (((p1, x1) as id), argl1),
      Tapply ((p2, x2), argl2) when String.compare x1 x2 = 0 ->
        (* We handle the case where a generic A<T> is used as A *)
        let argl1 =
          if argl1 = [] && not (Env.is_strict env)
          then List.map (fun _ -> (r1, Tany)) argl2
          else argl1
        in
        let argl2 =
          if argl2 = [] && not (Env.is_strict env)
          then List.map (fun _ -> (r1, Tany)) argl1
          else argl2
        in
        if List.length argl1 <> List.length argl2
        then
          if !is_silent_mode
          then env, Tany
          else
            error_l [p1, "This type has "^soi (List.length argl1)^
                     " arguments";
                     p2, "This one has "^soi (List.length argl2)]
        else
          let env, argl = lfold2 unify env argl1 argl2 in
          env, Tapply (id, argl)
  | Tabstract (((p1, x1) as id), argl1, tcstr1),
      Tabstract ((p2, x2), argl2, tcstr2) when String.compare x1 x2 = 0 ->
        assert (List.length argl1 = List.length argl2);
        let env, tcstr =
          match tcstr1, tcstr2 with
          | None, None -> env, None
          | Some x1, Some x2 ->
              let env, x = unify env x1 x2 in
              env, Some x
          | _ -> assert false
        in
        let env, argl = lfold2 unify env argl1 argl2 in
        env, Tabstract (id, argl, tcstr)
  | Tgeneric (x1, None), Tgeneric (x2, None) when x1 = x2 ->
      env, Tgeneric (x1, None)
  | Tgeneric (x1, Some ty1), Tgeneric (x2, Some ty2) when x1 = x2 ->
      let env, ty = unify env ty1 ty2 in
      env, Tgeneric (x1, Some ty)
  | Tgeneric ("this", Some ((_, Tapply ((_, x) as id, _) as ty))), _ ->
      let env, class_ = Env.get_class env x in
      (* For final class C, there is no difference between this<X> and X *)
      let env, ty = (match class_ with
        | Some {tc_final = true} -> unify env ty (r2, ty2)
        | _ ->
            (try TUtils.uerror r1 ty1 r2 ty2
            with Error l ->
              match ty2 with
              | Tapply ((_, y), _) when y = x ->
                let message1 = "Since "^(snd id)^" is not final" in
                let message2 = "this might not be a "^(snd id) in
                Utils.error_l
                  (l @ [(fst id, message1);  (Reason.to_pos r1, message2)])
              | _ -> Utils.error_l l
            )
        ) in
      env, snd ty
  | _, Tgeneric ("this", Some (_, Tapply ((_, x), _))) ->
      unify_ env r2 ty2 r1 ty1
  | (Ttuple _ as ty), Tarray (_, None, None)
  | Tarray (_, None, None), (Ttuple _ as ty) ->
      env, ty
  | Ttuple tyl1, Ttuple tyl2 ->
      let size1 = List.length tyl1 in
      let size2 = List.length tyl2 in
      if size1 <> size2
      then
        let p1 = Reason.to_pos r1 in
        let p2 = Reason.to_pos r2 in
        error_l [p1, "This tuple has "^soi size1^" elements";
                 p2, "This one has "^soi size2^" elements"]
      else
        let env, tyl = lfold2 unify env tyl1 tyl2 in
        env, Ttuple tyl
  | Tmixed, Tmixed -> env, Tmixed
  | Tanon (_, _, id1), Tanon (_, _, id2) when id1 = id2 -> env, ty1
  | Tanon _, Tanon _ -> env, Tunresolved [r1, ty1; r2, ty2]
  | Tfun ft, Tanon (mand_arg, total_arg, id)
  | Tanon (mand_arg, total_arg, id), Tfun ft ->
      if not (IMap.mem id env.Env.genv.Env.anons)
      then error (Reason.to_pos r1) "recursive call to an anonymous function"
      else
        let anon = IMap.find_unsafe id env.Env.genv.Env.anons in
        let p1 = Reason.to_pos r1 in
        let p2 = Reason.to_pos r2 in
        if mand_arg <> ft.ft_arity_min || total_arg <> ft.ft_arity_max
        then error_l [p1, ("Arity mismatch"); p2, "Because of this definition"];
        let env, ft = Inst.instantiate_ft env ft in
        let env, ret = anon env ft.ft_params in
        let env, _ = unify env ft.ft_ret ret in
        env, Tfun ft
  | Tobject, Tobject
  | Tobject, Tapply _
  | Tapply _, Tobject -> env, Tobject
  | Tshape fdm1, Tshape fdm2 ->
      let f env x y = fst (unify env x y) in
      (* We do it both direction to verify that no field is missing *)
      let env = TUtils.apply_shape ~f env (r1, fdm1) (r2, fdm2) in
      let env = TUtils.apply_shape ~f env (r2, fdm2) (r1, fdm1) in
      env, Tshape fdm1
  | _ when !is_silent_mode -> env, Tany
  | _ ->
      TUtils.uerror r1 ty1 r2 ty2

and unify_reason r1 r2 =
  if r1 = Reason.none then r2 else
  if r2 = Reason.none then r1 else
  let c = Reason.compare r1 r2 in
  if c <= 0 then r1
  else r2

and iunify env ty1 ty2 =
  let env, _ = unify env ty1 ty2 in
  env

(* This function is used to unify two functions *)
(* r1 is the reason for ft1 to have its type (witness) *)
and unify_funs env r1 ft1 r2 ft2 =
  let p = Reason.to_pos r2 in
  let p1 = Reason.to_pos r1 in
  if ft2.ft_arity_min <> ft1.ft_arity_min || ft2.ft_arity_max <> ft1.ft_arity_max
  then error_l [p, ("Arity mismatch"); p1, "Because of this definition"];
  let env, params = unify_params env ft1.ft_params ft2.ft_params in
  let env, ret = unify env ft1.ft_ret ft2.ft_ret in
  env, { ft1 with ft_params = params; ft_ret = ret }

and unify_params env l1 l2 =
  match l1, l2 with
  | [], l | l, [] -> env, l
  | (name1, x1) :: rl1, (name2, x2) :: rl2 ->
      let name = if name1 = name2 then name1 else None in
      let env, _ = unify env x2 x1 in
      let env, rl = unify_params env rl1 rl2 in
      env, (name, x2) :: rl

let unify_nofail env ty1 ty2 =
  try unify env ty1 ty2
  with Error _ ->
    let res = Env.fresh_type() in
    (* TODO: this can produce an unresolved of unresolved *)
    let env, res = unify env res (fst ty1, Tunresolved [ty1; ty2]) in
    env, res

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.unify_ref := unify

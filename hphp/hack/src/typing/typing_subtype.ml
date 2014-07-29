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

module Reason = Typing_reason
module Inst = Typing_instantiate
module Unify = Typing_unify
module Env = Typing_env
module TDef = Typing_tdef
module TUtils = Typing_utils
module ShapeMap = Nast.ShapeMap

(* This function checks that the method ft_sub can be used to replace
 * (is a subtype of) ft_super *)
let rec subtype_funs_generic ~check_return env r_super ft_super r_sub orig_ft_sub =
  let p_sub = Reason.to_pos r_sub in
  let p_super = Reason.to_pos r_super in
  let env, ft_sub = Inst.instantiate_ft env orig_ft_sub in
  if (arity_min ft_sub.ft_arity) > (arity_min ft_super.ft_arity)
  then Errors.fun_too_many_args p_sub p_super;
  (match ft_sub.ft_arity, ft_super.ft_arity with
    | Fellipsis _, Fvariadic _ ->
      (* The HHVM runtime ignores "..." entirely, but knows about
       * "...$args"; for contexts for which the runtime enforces method
       * compatibility (currently, inheritance from abstract/interface
       * methods), letting "..." override "...$args" would result in method
       * compatibility errors at runtime. *)
      Errors.fun_variadicity_hh_vs_php56 p_sub p_super;
    | Fstandard (_, sub_max), Fstandard (_, super_max) ->
      if sub_max < super_max
      then Errors.fun_too_few_args p_sub p_super;
    | Fstandard _, _ -> Errors.fun_unexpected_nonvariadic p_sub p_super;
    | _, _ -> ()
  );

  (* We are dissallowing contravariant arguments, they are not supported
   * by the runtime *)
  (* However, if we are polymorphic in the upper-class we have to be
   * polymorphic in the subclass. *)
  let env, var_opt = match ft_sub.ft_arity, ft_super.ft_arity with
    | Fvariadic (_, (n_super, var_super)), Fvariadic (_, (n_sub, var_sub)) ->
      let env, var = Unify.unify env var_super var_sub in
      env, Some (n_super, var)
    | _ -> env, None
  in
  let env, _ =
    Unify.unify_params env ft_super.ft_params ft_sub.ft_params var_opt in
  (* Checking that if the return type was defined in the parent class, it
   * is defined in the subclass too (requested by Gabe Levi).
   *)
  (* We agreed this was too painful for now, breaks too many things *)
  (*  (match ft_super.ft_ret, ft_sub.ft_ret with
      | (_, Tany), _ -> ()
      | (r_super, ty), (r_sub, Tany) ->
      let p_super = Reason.to_pos r_super in
      let p_sub = Reason.to_pos r_sub in
      error_l [p_sub, "Please add a return type";
      p_super, "Because we want to be consistent with this annotation"]
      | _ -> ()
      );
  *)
  let env = if check_return then sub_type env ft_super.ft_ret ft_sub.ft_ret else env in
  let env, _ = Unify.unify_funs env r_sub ft_sub r_sub orig_ft_sub in
  env

(**
 * Checks that ty2 is a subtype of ty1, and returns an env.
 *
 * E.g. sub_type env ?int int   => env
 *      sub_type env int alpha  => env where alpha==int
 *      sub_type env ?int alpha => env where alpha==?int
 *      sub_type env int string => error
 *)
and sub_type env ty1 ty2 =
  let env, ety1 = Env.expand_type env ty1 in
  let env, ety2 = Env.expand_type env ty2 in
  match ety1, ety2 with
  | (r, Tapply ((_, x), argl)), _ when Typing_env.is_typedef env x ->
      let env, ty1 = TDef.expand_typedef env r x argl in
      sub_type env ty1 ty2
  | _, (r, Tapply ((_, x), argl)) when Typing_env.is_typedef env x ->
      let env, ty2 = TDef.expand_typedef env r x argl in
      sub_type env ty1 ty2
  | (_, Tunresolved _), (_, Tunresolved _) ->
      let env, _ = Unify.unify env ty1 ty2 in
      env
  | (_, Tunresolved tyl), (r2, _) ->
      let env, _ = Unify.unify env ty1 (r2, Tunresolved [ty2]) in
      env
  | (_, Tany), (_, Tunresolved _) ->
      (* This branch is necessary in the following case:
       * function foo<T as I>(T $x)
       * if I call foo with an intersection type, T is a Tvar,
       * it's expanded version (ety1 in this case) is Tany and what
       * we end up doing is unifying all the elements of the intersection
       * together ...
       * Thanks to this branch, the type variable unifies with the intersection
       * type.
       *)
      fst (Unify.unify env ty1 ty2)
  | _, (_, Tunresolved tyl) ->
      List.fold_left (fun env x -> sub_type env ty1 x) env tyl
  | (_, Tapply _), (r2, Tgeneric (x, Some ty2)) ->
      (Errors.try_
         (fun () -> sub_type env ty1 ty2)
         (fun l -> Reason.explain_generic_constraint r2 x l; env)
      )
  | (r1, Tgeneric ("this", Some ty1)), (r2, Tgeneric ("this", Some ty2)) ->
      sub_type env ty1 ty2
  | (_, Tgeneric (x1, _)), (r2, Tgeneric (x2, Some ty2)) ->
      if x1 = x2 then env else
      (Errors.try_
         (fun () -> sub_type env ty1 ty2)
         (fun l -> Reason.explain_generic_constraint r2 x2 l; env)
      )
  (* Dirty covariance hacks *)
  | (_, (Tapply ((_, "\\Awaitable"), [ty1]))),
    (_, (Tapply ((_, "\\Awaitable"), [ty2]))) ->
      let old_allow_null_as_void = Env.allow_null_as_void env in
      let env = Env.set_allow_null_as_void env in
      let env = sub_type env ty1 ty2 in
      Env.set_allow_null_as_void ~allow:old_allow_null_as_void env
  | (_, (Tapply ((_, ("\\ImmVector" | "\\ImmSet" | "\\PrivacyPolicyBase")), [ty1]))),
    (_, (Tapply ((_, ("\\ImmVector" | "\\ImmSet" | "\\PrivacyPolicyBase")), [ty2]))) ->
      sub_type env ty1 ty2
  | (_, (Tapply ((_, ("\\Pair" | "\\ImmMap" | "\\GenReadApi" | "\\GenReadIdxApi")), [kty1; vty1]))),
    (_, (Tapply ((_, ("\\Pair" | "\\ImmMap" | "\\GenReadApi" | "\\GenReadIdxApi")), [kty2; vty2]))) ->
      let env = sub_type env kty1 kty2 in
      sub_type env vty1 vty2
  | (_, (Tapply ((_, "\\Generator"), [tk1; tv1; ts1]))),
    (_, (Tapply ((_, "\\Generator"), [tk2; tv2; ts2]))) ->
      (* Currently, we are only covariant in the type of the value yielded. I
       * think we could also be covariant in the type of the key yielded and
       * also *contravariant* in the type of the value sent in, but since this
       * code is new and no one is relying on those two yet, let's see if we can
       * get away with being invariant and if anyone complains we can
       * reconsider. TODO(#4534682) come back to this. *)
      let env = sub_type env tv1 tv2 in
      let env, _ = Unify.unify env tk1 tk2 in
      let env, _ = Unify.unify env ts1 ts2 in
      env
  | (p1, (Tapply (x1, tyl1) as ty1_)), (p2, (Tapply (x2, tyl2) as ty2_)) ->
    let cid1, cid2 = (snd x1), (snd x2) in
    if cid1 = cid2 then fst (Unify.unify env ety1 ety2)
    else begin
      let env, class_ = Env.get_class env cid2 in
      (match class_ with
        | None -> env
        | Some class_ ->
          let subtype_req_ancestor =
            if class_.tc_kind = Ast.Ctrait || class_.tc_kind = Ast.Cinterface then
              (* a trait is never the runtime type, but it can be used
               * as a constraint if it has requirements for its using
               * classes *)
              let env, ret = SMap.fold begin fun elt elt_type acc ->
                match acc with
                  | _, Some _ -> acc
                  | env, None ->
                    Errors.try_ begin fun () ->
                      let _, elt_ty = elt_type in
                      env, Some (sub_type env ty1 (p2, elt_ty))
                    end (fun _ -> acc)
              end class_.tc_req_ancestors (env, None) in
              ret
            else None in
          (match subtype_req_ancestor with
            | Some ret -> ret
            | None ->
              let up_obj = SMap.get cid1 class_.tc_ancestors in
              match up_obj with
                | Some up_obj ->
                  (* We handle the case where a generic A<T> is used as A *)
                  let tyl2 =
                    if tyl2 = [] && not (Env.is_strict env)
                    then List.map (fun _ -> (p2, Tany)) class_.tc_tparams
                    else tyl2
                  in
                  if List.length class_.tc_tparams <> List.length tyl2
                  then
                    Errors.expected_tparam
                      (Reason.to_pos p2) (List.length class_.tc_tparams);
                  let subst = Inst.make_subst class_.tc_tparams tyl2 in
                  let env, up_obj = Inst.instantiate subst env up_obj in
                  sub_type env ty1 up_obj
                | None when class_.tc_members_fully_known ->
                  TUtils.uerror p1 ty1_ p2 ty2_;
                  env
                | _ -> env
          )
      )
    end
  | (_, Tmixed), _ -> env
  | (_, Tprim Nast.Tnum), (_, Tprim (Nast.Tint | Nast.Tfloat)) -> env
  | (_, Tapply ((_, "\\Traversable"), [ty2])), (r, Tarray (_, ty3, ty4))
  | (_, Tapply ((_, "\\Container"), [ty2])), (r, Tarray (_, ty3, ty4)) ->
      (match ty3, ty4 with
      | None, _ -> env
      | Some ty3, None ->
          let env, _ = Unify.unify env ty2 ty3 in
          env
      | Some ty3, Some ty4 ->
          let env, _ = Unify.unify env ty2 ty4 in
          env
      )
  | (_, Tapply ((_, "\\KeyedTraversable"), [ty1; ty2])), (r, Tarray (_, ty3, ty4))
  | (_, Tapply ((_, "\\KeyedContainer"), [ty1; ty2])), (r, Tarray (_, ty3, ty4)) ->
      (match ty3 with
      | None -> env
      | Some ty3 ->
          (match ty4 with
          | None ->
              let env, _ = Unify.unify env ty1 (r, Tprim Nast.Tint) in
              let env, _ = Unify.unify env ty2 ty3 in
              env
          | Some ty4 ->
              let env, _ = Unify.unify env ty1 ty3 in
              let env, _ = Unify.unify env ty2 ty4 in
              env
          )
      )
  | (_, Tapply ((_, "\\Indexish"), [ty1; ty2])), (r, Tarray (_, ty3, ty4)) ->
      (match ty3 with
      | None -> env
      | Some ty3 ->
          (match ty4 with
          | None ->
              let env, _ = Unify.unify env ty1 (r, Tprim Nast.Tint) in
              let env, _ = Unify.unify env ty2 ty3 in
              env
          | Some ty4 ->
              let env, _ = Unify.unify env ty1 ty3 in
              let env, _ = Unify.unify env ty2 ty4 in
              env
          )
      )
  | (_, Tapply ((_, "\\Stringish"), _)), (_, Tprim Nast.Tstring) -> env
  | (_, Tapply ((_, "\\XHPChild"), _)), (_, Tarray _) -> env
  | (_, Tapply ((_, "\\XHPChild"), _)), (_, Tprim Nast.Tint) -> env
  | (_, Tapply ((_, "\\XHPChild"), _)), (_, Tprim Nast.Tfloat) -> env
  | (_, Tapply ((_, "\\XHPChild"), _)), (_, Tprim Nast.Tstring) -> env
  | (_, Tapply ((_, "\\XHPChild"), _)), (_, Tprim Nast.Tnum) -> env
  | (_, (Tarray (_, Some ty1, None))), (_, (Tarray (_, Some ty2, None))) ->
      sub_type env ty1 ty2
  | (_, (Tarray (_, Some kty1, Some vty1))), (_, Tarray (_, Some kty2, Some vty2)) ->
      let env = sub_type env kty1 kty2 in
      sub_type env vty1 vty2
  | (_, Tarray (_, Some _, Some _)), (reason, Tarray (is_local, Some elt_ty, None)) ->
      let int_reason = Reason.Ridx (Reason.to_pos reason) in
      let int_type = int_reason, Tprim Nast.Tint in
      sub_type env ty1 (reason, Tarray (is_local, Some int_type, Some elt_ty))
  | _, (_, Tany) -> env
  | (_, Tany), _ -> fst (Unify.unify env ty1 ty2)
  | (_, Toption ty1), (_, Toption ty2) ->
      sub_type env ty1 ty2
  | (_, Toption ty1), _ ->
      sub_type env ty1 ty2
  | (_, Ttuple tyl1), (_, Ttuple tyl2)
    when List.length tyl1 = List.length tyl2 ->
      wfold_left2 sub_type env tyl1 tyl2
  | (r1, Tfun ft1), (r2, Tfun ft2) ->
      subtype_funs_generic ~check_return:true env r1 ft1 r2 ft2
  | (r1, Tshape fdm1), (r2, Tshape fdm2) ->
      ShapeMap.iter begin fun k _ ->
        if not (ShapeMap.mem k fdm1)
        then
          let p1 = Reason.to_pos r1 in
          let p2 = Reason.to_pos r2 in
          Errors.field_missing (TUtils.get_shape_field_name k) p1 p2
      end fdm2;
      TUtils.apply_shape sub_type env (r1, fdm1) (r2, fdm2)
  | _, (_, Tabstract (_, _, Some x)) ->
      Errors.try_
         (fun () -> fst (Unify.unify env ty1 ty2))
         (fun _ -> sub_type env ty1 x)
  | _ -> fst (Unify.unify env ty1 ty2)

and is_sub_type env ty1 ty2 =
  Errors.try_
    (fun () -> ignore(sub_type env ty1 ty2); true)
    (fun _ -> false)

and sub_string p env ty2 =
  let env, ety2 = Env.expand_type env ty2 in
  match ety2 with
  | (_, Toption ty2) -> sub_string p env ty2
  | (_, Tunresolved tyl) ->
      List.fold_left (sub_string p) env tyl
  | (_, Tprim _) ->
      env
  | (_, Tabstract (_, _, Some ty))
  | (_, Tgeneric (_, Some ty)) ->
      sub_string p env ty
  | (r2, Tapply ((_, x), argl)) when Typing_env.is_typedef env x ->
      let env, ty2 = Typing_tdef.expand_typedef env r2 x argl in
      sub_string p env ty2
  | (r2, Tapply (x, _)) ->
      let env, class_ = Env.get_class env (snd x) in
      (match class_ with
      | None -> env
      | Some {tc_name = "\\Stringish"; _} -> env
      | Some tc when SMap.mem "\\Stringish" tc.tc_ancestors -> env
      | Some _ ->
          Errors.object_string p (Reason.to_pos r2);
          env
      )
  | (_, Tany) when Env.is_strict env ->
      Errors.untyped_string p;
      env
  | _, Tany ->
      env (* Unifies with anything *)
  | _, Tobject -> env
  | _ -> fst (Unify.unify env (Reason.Rwitness p, Tprim Nast.Tstring) ty2)

(* and subtype_params env l_super l_sub def_super = *)
(*   match l_super, l_sub with *)
(*   | l, [] -> env, l *)
(*   | [], l -> *)
(*   | (name_super, x_super) :: rl_super, (name_sub, x_sub) :: rl_sub -> *)
(*     let name = if name_super = name_sub then name_super else None in *)
(*     let env = { env with Env.pos = Reason.to_pos (fst x_super) } in *)
(*     let env, _ = Unify.unify env x_super x_sub in *)
(*     let env, rl = Unify.unify_params env rl_super rl_sub in *)
(*     env, (name, x_sub) :: rl *)

let subtype_funs = subtype_funs_generic ~check_return:true
let subtype_funs_no_return = subtype_funs_generic ~check_return:false

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.sub_type_ref := sub_type

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
 * Checks that ty_sub is a subtype of ty_super, and returns an env.
 *
 * E.g. sub_type env ?int int   => env
 *      sub_type env int alpha  => env where alpha==int
 *      sub_type env ?int alpha => env where alpha==?int
 *      sub_type env int string => error
 *)
and sub_type env ty_super ty_sub =
  let env, ety_super = Env.expand_type env ty_super in
  let env, ety_sub = Env.expand_type env ty_sub in
  match ety_super, ety_sub with
  | (r, Tapply ((_, x), argl)), _ when Typing_env.is_typedef env x ->
      let env, ty_super = TDef.expand_typedef env r x argl in
      sub_type env ty_super ty_sub
  | _, (r, Tapply ((_, x), argl)) when Typing_env.is_typedef env x ->
      let env, ty_sub = TDef.expand_typedef env r x argl in
      sub_type env ty_super ty_sub
  | (_, Tunresolved _), (_, Tunresolved _) ->
      let env, _ = Unify.unify env ty_super ty_sub in
      env
  | (_, Tunresolved tyl), (r_sub, _) ->
      let env, _ = Unify.unify env ty_super (r_sub, Tunresolved [ty_sub]) in
      env
  | (_, Tany), (_, Tunresolved _) ->
      (* This branch is necessary in the following case:
       * function foo<T as I>(T $x)
       * if I call foo with an intersection type, T is a Tvar,
       * it's expanded version (ety_super in this case) is Tany and what
       * we end up doing is unifying all the elements of the intersection
       * together ...
       * Thanks to this branch, the type variable unifies with the intersection
       * type.
       *)
      fst (Unify.unify env ty_super ty_sub)
  | _, (_, Tunresolved tyl) ->
      List.fold_left (fun env x -> sub_type env ty_super x) env tyl
  | (_, Tapply _), (r_sub, Tgeneric (x, Some ty_sub)) ->
      (Errors.try_
         (fun () -> sub_type env ty_super ty_sub)
         (fun l -> Reason.explain_generic_constraint r_sub x l; env)
      )
  | (r_super, Tgeneric ("this", Some ty_super)), (r_sub, Tgeneric ("this", Some ty_sub)) ->
      sub_type env ty_super ty_sub
  | (_, Tgeneric (x_super, _)), (r_sub, Tgeneric (x_sub, Some ty_sub)) ->
      if x_super = x_sub then env else
      (Errors.try_
         (fun () -> sub_type env ty_super ty_sub)
         (fun l -> Reason.explain_generic_constraint r_sub x_sub l; env)
      )
  (* Dirty covariance hacks *)
  | (_, (Tapply ((_, "\\Awaitable"), [ty_super]))),
    (_, (Tapply ((_, "\\Awaitable"), [ty_sub]))) ->
      let old_allow_null_as_void = Env.allow_null_as_void env in
      let env = Env.set_allow_null_as_void env in
      let env = sub_type env ty_super ty_sub in
      Env.set_allow_null_as_void ~allow:old_allow_null_as_void env
  | (_, (Tapply (
      (_, (("\\Traversable" | "\\Container" | "\\Iterable" | "\\Iterator" |
            "\\ConstCollection" | "\\ConstVector" | "\\ConstSet" |
            "\\ImmVector" | "\\ImmSet" | "\\PrivacyPolicyBase" |
            "\\DataTypeImplProvider") as name_super)),
      [ty_super]
    ))),
    (_, (Tapply (
      (_, (("\\Traversable" | "\\Container" | "\\Iterable" | "\\Iterator" |
            "\\ConstCollection" | "\\ConstVector" | "\\ConstSet" |
            "\\ImmVector" | "\\ImmSet" | "\\PrivacyPolicyBase" |
            "\\DataTypeImplProvider") as name_sub)),
      [ty_sub]
    )))
    when name_super = name_sub ->
      sub_type env ty_super ty_sub
  | (_, (Tapply (
      (_, (("\\KeyedTraversable" | "\\KeyedContainer" | "\\Indexish" |
            "\\KeyedIterable" | "\\KeyedIterator" | "\\ConstMap" | "\\ImmMap" |
            "\\Pair" | "\\GenReadApi" | "\\GenReadIdxApi") as name_super)),
      [tk_super; tv_super]
    ))),
    (_, (Tapply (
      (_, (("\\KeyedTraversable" | "\\KeyedContainer" | "\\Indexish" |
            "\\KeyedIterable" | "\\KeyedIterator" | "\\ConstMap" | "\\ImmMap" |
            "\\Pair" | "\\GenReadApi" | "\\GenReadIdxApi") as name_sub)),
      [tk_sub; tv_sub]
    )))
    when name_super = name_sub ->
      let env = sub_type env tk_super tk_sub in
      sub_type env tv_super tv_sub
  | (_, (Tapply (
      (_, (("\\DataType") as name_super)),
      [t1_super; t2_super; t3_super]
    ))),
    (_, (Tapply (
      (_, (("\\DataType") as name_sub)),
      [t1_sub; t2_sub; t3_sub]
    )))
    when name_super = name_sub ->
      let env = sub_type env t1_super t1_sub in
      let env = sub_type env t2_super t2_sub in
      sub_type env t3_super t3_sub
  | (_, (Tapply ((_, "\\Generator"), [tk_super; tv_super; ts_super]))),
    (_, (Tapply ((_, "\\Generator"), [tk_sub; tv_sub; ts_sub]))) ->
      (* Currently, we are only covariant in the type of the value yielded. I
       * think we could also be covariant in the type of the key yielded and
       * also *contravariant* in the type of the value sent in, but since this
       * code is new and no one is relying on those two yet, let's see if we can
       * get away with being invariant and if anyone complains we can
       * reconsider. TODO(#4534682) come back to this. *)
      let env = sub_type env tv_super tv_sub in
      let env, _ = Unify.unify env tk_super tk_sub in
      let env, _ = Unify.unify env ts_super ts_sub in
      env
  | (p_super, (Tapply (x_super, tyl_super) as ty_super_)),
      (p_sub, (Tapply (x_sub, tyl_sub) as ty_sub_))
      when Typing_env.get_enum_constraint env (snd x_sub) = None  ->
    let cid_super, cid_sub = (snd x_super), (snd x_sub) in
    if cid_super = cid_sub then fst (Unify.unify env ety_super ety_sub)
    else begin
      let env, class_ = Env.get_class env cid_sub in
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
                      env, Some (sub_type env ty_super (p_sub, elt_ty))
                    end (fun _ -> acc)
              end class_.tc_req_ancestors (env, None) in
              ret
            else None in
          (match subtype_req_ancestor with
            | Some ret -> ret
            | None ->
              let up_obj = SMap.get cid_super class_.tc_ancestors in
              match up_obj with
                | Some up_obj ->
                  (* We handle the case where a generic A<T> is used as A *)
                  let tyl_sub =
                    if tyl_sub = [] && not (Env.is_strict env)
                    then List.map (fun _ -> (p_sub, Tany)) class_.tc_tparams
                    else tyl_sub
                  in
                  if List.length class_.tc_tparams <> List.length tyl_sub
                  then
                    Errors.expected_tparam
                      (Reason.to_pos p_sub) (List.length class_.tc_tparams);
                  let subst = Inst.make_subst class_.tc_tparams tyl_sub in
                  let env, up_obj = Inst.instantiate subst env up_obj in
                  sub_type env ty_super up_obj
                | None when class_.tc_members_fully_known ->
                  TUtils.uerror p_super ty_super_ p_sub ty_sub_;
                  env
                | _ -> env
          )
      )
    end
  | (_, Tmixed), _ -> env
  | (_, Tprim Nast.Tnum), (_, Tprim (Nast.Tint | Nast.Tfloat)) -> env
  | (_, Tapply ((_, ("\\Traversable" | "\\Container")), [tv_super])), (r, Tarray (_, ty3, ty4)) ->
      (match ty3, ty4 with
      | None, _ -> env
      | Some ty3, None ->
          sub_type env tv_super ty3
      | Some ty3, Some ty4 ->
          sub_type env tv_super ty4
      )
  | (_, Tapply ((_, ("\\KeyedTraversable" | "\\KeyedContainer" | "\\Indexish")), [tk_super; tv_super])), (r, Tarray (_, ty3, ty4)) ->
      (match ty3 with
      | None -> env
      | Some ty3 ->
          (match ty4 with
          | None ->
              let env = sub_type env tk_super (r, Tprim Nast.Tint) in
              sub_type env tv_super ty3
          | Some ty4 ->
              let env = sub_type env tk_super ty3 in
              sub_type env tv_super ty4
          )
      )
  | (_, Tapply ((_, "\\Stringish"), _)), (_, Tprim Nast.Tstring) -> env
  | (_, Tapply ((_, "\\XHPChild"), _)), (_, Tarray _) -> env
  | (_, Tapply ((_, "\\XHPChild"), _)), (_, Tprim Nast.Tint) -> env
  | (_, Tapply ((_, "\\XHPChild"), _)), (_, Tprim Nast.Tfloat) -> env
  | (_, Tapply ((_, "\\XHPChild"), _)), (_, Tprim Nast.Tstring) -> env
  | (_, Tapply ((_, "\\XHPChild"), _)), (_, Tprim Nast.Tnum) -> env
  | (_, (Tarray (_, Some ty_super, None))), (_, (Tarray (_, Some ty_sub, None))) ->
      sub_type env ty_super ty_sub
  | (_, (Tarray (_, Some tk_super, Some tv_super))), (_, Tarray (_, Some tk_sub, Some tv_sub)) ->
      let env = sub_type env tk_super tk_sub in
      sub_type env tv_super tv_sub
  | (_, Tarray (_, Some _, Some _)), (reason, Tarray (is_local, Some elt_ty, None)) ->
      let int_reason = Reason.Ridx (Reason.to_pos reason) in
      let int_type = int_reason, Tprim Nast.Tint in
      sub_type env ty_super (reason, Tarray (is_local, Some int_type, Some elt_ty))
  | _, (_, Tany) -> env
  | (_, Tany), _ -> fst (Unify.unify env ty_super ty_sub)
  | (_, Toption ty_super), (_, Toption ty_sub) ->
      sub_type env ty_super ty_sub
  | (_, Toption ty_super), _ ->
      sub_type env ty_super ty_sub
  | (_, Ttuple tyl_super), (_, Ttuple tyl_sub)
    when List.length tyl_super = List.length tyl_sub ->
    wfold_left2 sub_type env tyl_super tyl_sub
  | (r_super, Tfun ft_super), (r_sub, Tfun ft_sub) ->
      subtype_funs_generic ~check_return:true env r_super ft_super r_sub ft_sub
  | (r_super, Tshape fdm_super), (r_sub, Tshape fdm_sub) ->
      ShapeMap.iter begin fun k _ ->
        if not (ShapeMap.mem k fdm_super)
        then
          let p_super = Reason.to_pos r_super in
          let p_sub = Reason.to_pos r_sub in
          Errors.field_missing (TUtils.get_shape_field_name k) p_super p_sub
      end fdm_sub;
      TUtils.apply_shape sub_type env (r_super, fdm_super) (r_sub, fdm_sub)
  | _, (_, Tabstract (_, _, Some x)) ->
      Errors.try_
         (fun () -> fst (Unify.unify env ty_super ty_sub))
         (fun _ -> sub_type env ty_super x)
  (* Handle enums with subtyping constraints. *)
  | _, (p_sub, (Tapply ((_, x), [])))
    when Typing_env.get_enum_constraint env x <> None ->
    (match Typing_env.get_enum_constraint env x with
      | Some base ->
        (* Handling is the same as abstracts with as *)
        Errors.try_
          (fun () -> fst (Unify.unify env ty_super ty_sub))
          (fun _ -> sub_type env ty_super base)
      | None -> assert false)
  | _ -> fst (Unify.unify env ty_super ty_sub)

and is_sub_type env ty_super ty_sub =
  Errors.try_
    (fun () -> ignore(sub_type env ty_super ty_sub); true)
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

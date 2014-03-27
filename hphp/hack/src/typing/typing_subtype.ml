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

module Reason = Typing_reason
module Inst = Typing_instantiate
module Unify = Typing_unify
module Env = Typing_env
module TDef = Typing_tdef
module TUtils = Typing_utils

(* This function checks that the method ft2 can be used to replace ft1 *)
let rec subtype_funs_generic ~check_return env r1 ft1 r2 orig_ft2 =
  let p = Reason.to_pos r2 in
  let p1 = Reason.to_pos r1 in
  let env, ft2 = Inst.instantiate_ft env orig_ft2 in
  if ft2.ft_arity_min > ft1.ft_arity_min
  then error_l [p, ("Too many mandatory arguments");
                p1, "Because of this definition"];
  if ft2.ft_arity_max < ft1.ft_arity_max
  then error_l [p, ("Too few arguments");
                p1, "Because of this definition"];
  let env, _ = subtype_params env ft1.ft_params ft2.ft_params in
  (* Checking that if the return type was defined in the parent class, it
   * is defined in the subclass too (requested by Gabe Levi).
   *)
  (* We agreed this was too painful for now, breaks too many things *)
  (*  (match ft1.ft_ret, ft2.ft_ret with
      | (_, Tany), _ -> ()
      | (r1, ty), (r2, Tany) ->
      let p1 = Reason.to_pos r1 in
      let p2 = Reason.to_pos r2 in
      error_l [p2, "Please add a return type";
      p1, "Because we want to be consistent with this annotation"]
      | _ -> ()
      );
  *)
  let env = if check_return then sub_type env ft1.ft_ret ft2.ft_ret else env in
  let env, _ = Unify.unify_funs env r2 ft2 r2 orig_ft2 in
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
      let env, ty1 = TDef.expand_typedef SSet.empty env r x argl in
      sub_type env ty1 ty2
  | _, (r, Tapply ((_, x), argl)) when Typing_env.is_typedef env x ->
      let env, ty2 = TDef.expand_typedef SSet.empty env r x argl in
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
  | _, (_, Tabstract (_, _, Some x)) ->
      (try fst (Unify.unify env ty1 ty2)
       with _ -> sub_type env ty1 x)
  | (_, Tapply _), (r2, Tgeneric (x, Some ty2)) ->
      (try
         sub_type env ty1 ty2
       with Error l ->
         Reason.explain_generic_constraint r2 x l
      )
  | (r1, Tgeneric ("this", Some ty1)), (r2, Tgeneric ("this", Some ty2)) ->
      sub_type env ty1 ty2
  | (_, Tgeneric (x1, _)), (r2, Tgeneric (x2, Some ty2)) ->
      if x1 = x2 then env else
      (try
         sub_type env ty1 ty2
       with Error l ->
         Reason.explain_generic_constraint r2 x2 l
      )
  (* Dirty covariance hacks *)
  | (_, (Tapply ((_, "Awaitable"), [ty1]))),
    (_, (Tapply ((_, "Awaitable"), [ty2]))) ->
      let old_allow_null_as_void = Env.allow_null_as_void env in
      let env = Env.set_allow_null_as_void env in
      let env = sub_type env ty1 ty2 in
      Env.set_allow_null_as_void ~allow:old_allow_null_as_void env
  | (_, (Tapply ((_, ("Continuation" | "ImmVector" | "ImmSet")), [ty1]))),
    (_, (Tapply ((_, ("Continuation" | "ImmVector" | "ImmSet")), [ty2]))) ->
      sub_type env ty1 ty2
  | (_, (Tapply ((_, ("Pair" | "ImmMap")), [kty1; vty1]))),
    (_, (Tapply ((_, ("Pair" | "ImmMap")), [kty2; vty2]))) ->
      let env = sub_type env kty1 kty2 in
      sub_type env vty1 vty2
  | (p1, (Tapply (x1, tyl1) as ty1_)), (p2, (Tapply (x2, tyl2) as ty2_)) ->
    let cid1, cid2 = (snd x1), (snd x2) in
    if cid1 = cid2 then fst (Unify.unify env ety1 ety2)
    else begin
      let env, class_ = Env.get_class env cid2 in
      (match class_ with
        | None ->
          (match env.Env.genv.Env.mode with
            | Ast.Mstrict -> raise Ignore
            | Ast.Mpartial | Ast.Mdecl -> env
          )
        | Some class_ ->
          let subtype_req_ancestor =
            if class_.tc_kind = Ast.Ctrait then
            (* a trait is never the runtime type, but it can be used as
             * a constraint if it has requirements for its using
             * classes *)
              let pos = (fst x2) in
              let env, ret = SSet.fold begin fun elt acc ->
                match acc with
                  | _, Some _ -> acc
                  | env, None ->
                    try
                      let candidate_ty = (p2, Tapply ((pos, elt), tyl2)) in
                      env, Some (sub_type env ty1 candidate_ty)
                    with _ -> acc
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
                    error (Reason.to_pos p2)
                      ("Expected " ^
                          (match List.length class_.tc_tparams with
                            | 0 -> "no type parameter"
                            | 1 -> "a type parameter"
                            | n -> string_of_int n ^ " type parameters"
                          ));
                  let subst = Inst.make_subst class_.tc_tparams tyl2 in
                  let env, up_obj = Inst.instantiate subst env up_obj in
                  sub_type env ty1 up_obj
                | None when !is_silent_mode -> env
                | None when class_.tc_members_fully_known ->
                  TUtils.uerror p1 ty1_ p2 ty2_
                | _ -> env
          )
      )
    end
  | (_, Tmixed), _ -> env
  | (_, Tprim Nast.Tnum), (_, Tprim (Nast.Tint | Nast.Tfloat)) -> env
  | (_, Tapply ((_, "Traversable"), [ty2])), (r, Tarray (_, ty3, ty4)) ->
      (match ty3, ty4 with
      | None, _ -> env
      | Some ty3, None ->
          let env, _ = Unify.unify env ty2 ty3 in
          env
      | Some ty3, Some ty4 ->
          let env, _ = Unify.unify env ty2 ty4 in
          env
      )
  | (_, Tapply ((_, "KeyedTraversable"), [ty1; ty2])), (r, Tarray (_, ty3, ty4)) ->
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
  | (_, Tapply ((_, "Indexish"), [ty1; ty2])), (r, Tarray (_, ty3, ty4)) ->
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
  | (_, Tapply ((_, "Stringish"), _)), (_, Tprim Nast.Tstring) -> env
  | (_, Tapply ((_, "XHPChild"), _)), (_, Tarray _) -> env
  | (_, Tapply ((_, "XHPChild"), _)), (_, Tprim Nast.Tint) -> env
  | (_, Tapply ((_, "XHPChild"), _)), (_, Tprim Nast.Tfloat) -> env
  | (_, Tapply ((_, "XHPChild"), _)), (_, Tprim Nast.Tstring) -> env
  | (_, Tapply ((_, "XHPChild"), _)), (_, Tprim Nast.Tnum) -> env
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
      SMap.iter begin fun k _ ->
        if not (SMap.mem k fdm1)
        then
          let p1 = Reason.to_pos r1 in
          let p2 = Reason.to_pos r2 in
          error_l [p2, "The field '"^k^"' is defined";
                   p1, "The field '"^k^"' is missing"]
      end fdm2;
      TUtils.apply_shape sub_type env (r1, fdm1) (r2, fdm2)
  | _ -> fst (Unify.unify env ty1 ty2)

and is_sub_type env ty1 ty2 =
  try
    ignore(sub_type env ty1 ty2);
    true
  with _ ->
    false

and sub_string p env ty2 =
  let env, ety2 = Env.expand_type env ty2 in
  match ety2 with
  | (_, Toption ty2) -> sub_string p env ty2
  | (_, Tunresolved tyl) ->
      List.fold_left (sub_string p) env tyl
  | (_, Tprim _) ->
      env
  | (_, Tgeneric (_, Some ty)) ->
      sub_string p env ty
  | (r2, Tapply (x, _)) ->
      let env, class_ = Env.get_class env (snd x) in
      (match class_ with
      | None when Env.is_strict env ->
          raise Ignore
      | None -> env
      | Some {tc_name = "Stringish"} -> env
      | Some tc when SMap.mem "Stringish" tc.tc_ancestors -> env
      | _ when !is_silent_mode -> env
      | Some _ -> error_l [
          p, "You cannot use this object as a string";
          Reason.to_pos r2, "This object doesn't implement __toString"]
      )
  | (_, Tany) when Env.is_strict env ->
      error_l [
      p,
      "You cannot use this object as a string, it is an untyped value"
    ]
  | _, Tany ->
      env (* Unifies with anything *)
  | _, Tobject -> env
  | _ -> fst (Unify.unify env (Reason.Rwitness p, Tprim Nast.Tstring) ty2)

and subtype_params env l1 l2 =
  match l1, l2 with
  | [], l | l, [] -> env, l
  | (name1, x1) :: rl1, (name2, x2) :: rl2 ->
      (* We are dissalowing contravariant arguments, they are not supported
       * by the runtime
       *)
      (* However, if we are polymorphic in the upper-class we have to be
         polymorphic in the subclass.
       *)
      let name = if name1 = name2 then name1 else None in
      let env, _ = Unify.unify env x1 x2 in
      let env, rl = Unify.unify_params env rl1 rl2 in
      env, (name, x2) :: rl

let sub_type_ok env ty1 ty2 =
  try ignore (sub_type env ty1 ty2); true with Error _ -> false

let subtype_funs = subtype_funs_generic ~check_return:true
let subtype_funs_no_return = subtype_funs_generic ~check_return:false

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.sub_type_ref := sub_type

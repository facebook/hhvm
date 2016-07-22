(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open Utils
open Typing_defs
open Typing_dependent_type

module Reason = Typing_reason
module Unify = Typing_unify
module Env = Typing_env
module Subst = Decl_subst
module TUtils = Typing_utils
module TUEnv = Typing_unification_env
module SN = Naming_special_names
module Phase = Typing_phase

(* This function checks that the method ft_sub can be used to replace
 * (is a subtype of) ft_super *)
let rec subtype_funs_generic ~check_return env r_sub ft_sub r_super ft_super =
  let p_sub = Reason.to_pos r_sub in
  let p_super = Reason.to_pos r_super in
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

  let add_bound env (_, (_, name), cstrl) =
    List.fold_left cstrl ~init:env ~f:(fun env (ck, ty) ->
      match ck with
      | Ast.Constraint_super ->
        Env.add_lower_bound env name ty
      | Ast.Constraint_as ->
        Env.add_upper_bound env name ty) in

  let env = List.fold_left ft_sub.ft_tparams ~f:add_bound ~init:env in

  (* We are dissallowing contravariant arguments, they are not supported
   * by the runtime *)
  (* However, if we are polymorphic in the upper-class we have to be
   * polymorphic in the subclass. *)
  let env, var_opt = match ft_sub.ft_arity, ft_super.ft_arity with
    | Fvariadic (_, (n_super, var_super)), Fvariadic (_, (_, var_sub)) ->
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
  let env =
    if check_return
    then sub_type env ft_sub.ft_ret ft_super.ft_ret
    else env in
  env

(* Checking subtyping for methods is different than normal functions. Since
 * methods are declarations we do not want to instantiate their function type
 * parameters as unresolved, instead it should stay as a Tgeneric.
 *)
and subtype_method ~check_return env r_sub ft_sub r_super ft_super =
  if not ft_super.ft_abstract && ft_sub.ft_abstract then
    (* It is valid for abstract class to extend a concrete class, but it cannot
     * redefine already concrete members as abstract.
     * See override_abstract_concrete.php test case for example. *)
    Errors.abstract_concrete_override ft_sub.ft_pos ft_super.ft_pos `method_;
  let ety_env = Phase.env_with_self env in
  let env, ft_super_no_tvars =
    Phase.localize_ft ~ety_env ~instantiate_tparams:false env ft_super in
  let env, ft_sub_no_tvars =
    Phase.localize_ft ~ety_env ~instantiate_tparams:false env ft_sub in
  subtype_funs_generic
    ~check_return env
    r_sub ft_sub_no_tvars
    r_super ft_super_no_tvars

and subtype_tparams env c_name variancel children_tyl super_tyl =
  match variancel, children_tyl, super_tyl with
  | [], [], [] -> env
  | [], _, _
  | _, [], _
  | _, _, [] -> env
  | variance :: variancel, child :: childrenl, super :: superl ->
      let env = subtype_tparam env c_name variance child super in
      subtype_tparams env c_name variancel childrenl superl

and invariance_suggestion c_name =
  let open Naming_special_names.Collections in
  if c_name = cVector then
    "\nDid you mean ConstVector instead?"
  else if c_name = cMap then
    "\nDid you mean ConstMap instead?"
  else if c_name = cSet then
    "\nDid you mean ConstSet instead?"
  else ""

and subtype_tparam env c_name variance child (r_super, _ as super) =
  match variance with
  | Ast.Covariant -> sub_type env child super
  | Ast.Contravariant ->
      Errors.try_
        (fun () ->
          Env.invert_grow_super env (fun env -> sub_type env super child))
        (fun err ->
          let pos = Reason.to_pos r_super in
          Errors.explain_contravariance pos c_name err; env)
  | Ast.Invariant ->
      Errors.try_
        (fun () -> fst (Unify.unify env super child))
        (fun err ->
          let suggestion =
            let s = invariance_suggestion c_name in
            let try_fun = (fun () ->
              subtype_tparam env c_name Ast.Covariant child super) in
            if not (s = "") && Errors.has_no_errors try_fun then s else "" in
          let pos = Reason.to_pos r_super in
          Errors.explain_invariance pos c_name suggestion err; env)

(* Distinction b/w sub_type and sub_type_with_uenv similar to unify and
 * unify_with_uenv, see comment there. *)
and sub_type env ty_sub ty_super =
  sub_type_with_uenv env (TUEnv.empty, ty_sub) (TUEnv.empty, ty_super)

(**
 * Checks that ty_sub is a subtype of ty_super, and returns an env.
 *
 * E.g. sub_type env ?int int   => env
 *      sub_type env int alpha  => env where alpha==int
 *      sub_type env ?int alpha => env where alpha==?int
 *      sub_type env int string => error
*)
and sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super) =
  let env, ety_super =
    Env.expand_type env ty_super in
  let env, ety_sub =
    Env.expand_type env ty_sub in

  match ety_sub, ety_super with
  | (_, Tunresolved _), (_, Tunresolved _) ->
      let env, _ =
        Unify.unify_unwrapped env uenv_super.TUEnv.non_null ty_super
                                  uenv_sub.TUEnv.non_null ty_sub in
      env
(****************************************************************************)
(* ### Begin Tunresolved madness ###
 * If grow_super is true (the common case), then if the supertype is a
 * Tunresolved, we allow it to keep growing, which is the desired behavior for
 * e.g. figuring out the type of a generic, but if the subtype is a
 * Tunresolved, then we check that all the members are indeed subtypes of the
 * given supertype, which is the desired behavior for e.g. checking function
 * return values. In general, if a supertype is Tunresolved, then we
 * consider it to be "not yet finalized", but if a subtype is Tunresolved
 * and the supertype isn't, we've probably hit a type annotation
 * and should consider the supertype to be definitive.
 *
 * However, sometimes we want this behavior reversed, e.g. when the type
 * annotation has a contravariant generic parameter or a `super` constraint --
 * now the definitive type is the subtype.
 *
 * I considered splitting this out into a separate function and swapping the
 * order of the super / sub types passed to it, so we would only have to handle
 * one set of cases, but it doesn't look much better since that function still
 * has to recursively call sub_type and therefore needs to remember whether its
 * arguments had been swapped.
 *)
(****************************************************************************)
  | (r_sub, _), (_, Tunresolved _) when Env.grow_super env ->
      let ty_sub = (r_sub, Tunresolved [ty_sub]) in
      let env, _ =
        Unify.unify_unwrapped
          env ~unwrappedToption1:uenv_super.TUEnv.non_null ty_super
              ~unwrappedToption2:uenv_sub.TUEnv.non_null ty_sub in
      env
  | (_, Tunresolved _), (_, Tany) when Env.grow_super env ->
      (* This branch is necessary in the following case:
       * function foo<T as I>(T $x)
       * if I call foo with an intersection type, T is a Tvar,
       * it's expanded version (ety_super in this case) is Tany and what
       * we end up doing is unifying all the elements of the intersection
       * together ...
       * Thanks to this branch, the type variable unifies with the intersection
       * type.
       *)
    fst (Unify.unify_unwrapped
        env ~unwrappedToption1:uenv_super.TUEnv.non_null ty_super
            ~unwrappedToption2:uenv_sub.TUEnv.non_null ty_sub)
  |  (_, Tunresolved tyl), _ when Env.grow_super env ->
      List.fold_left tyl ~f:begin fun env x ->
        sub_type_with_uenv env (uenv_sub, x) (uenv_super, ty_super)
      end ~init:env
(****************************************************************************)
(* Repeat the previous 3 cases but with the super / sub order reversed *)
(****************************************************************************)
  | (_, Tunresolved _), (r_super, _) when not (Env.grow_super env) ->
      let ty_super = (r_super, Tunresolved [ty_super]) in
      let env, _ =
        Unify.unify_unwrapped
          env ~unwrappedToption1:uenv_super.TUEnv.non_null ty_super
              ~unwrappedToption2:uenv_sub.TUEnv.non_null ty_sub in
      env
  | (_, Tany), (_, Tunresolved _) when not (Env.grow_super env) ->
    fst (Unify.unify_unwrapped
        env ~unwrappedToption1:uenv_super.TUEnv.non_null ty_super
            ~unwrappedToption2:uenv_sub.TUEnv.non_null ty_sub)
  | _, (_, Tunresolved tyl) when not (Env.grow_super env) ->
      List.fold_left tyl ~f:begin fun env x ->
        sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, x)
      end ~init:env
(****************************************************************************)
(* OCaml doesn't inspect `when` clauses when checking pattern matching
 * exhaustiveness, so just assert false here *)
(****************************************************************************)
  | (_, Tunresolved _), _
  | _, (_, Tunresolved _) -> assert false
(****************************************************************************)
(* ### End Tunresolved madness ### *)
(****************************************************************************)
  | (_, Tany), _ -> env
  (* This case is for when Tany comes from expanding an empty Tvar - it will
   * result in binding the type variable to the other type. *)
  | _, (_, Tany) -> fst (Unify.unify env ty_super ty_sub)
  | (r_sub,   Tabstract (AKdependent d_sub, Some ty_sub)),
    (r_super, Tabstract (AKdependent d_super, Some ty_super))
        when d_sub = d_super ->
      let uenv_sub =
        { uenv_sub with
          TUEnv.dep_tys = (r_sub, d_sub)::uenv_sub.TUEnv.dep_tys } in
      let uenv_super =
        { uenv_super with
          TUEnv.dep_tys = (r_super, d_super)::uenv_super.TUEnv.dep_tys } in
      sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super)
  (* This is sort of a hack because our handling of Toption is highly
   * dependent on how the type is structured. When we see a bare
   * dependent type we strip it off at this point since it shouldn't be
   * relevant to subtyping any more.
   *)
  | (r, Tabstract (AKdependent (`expr _, [] as d), Some ty_sub)), _ ->
      let uenv_sub =
        { uenv_sub with
          TUEnv.dep_tys = (r, d)::uenv_sub.TUEnv.dep_tys } in
      sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super)
  | (r_sub, Tabstract (AKdependent d_sub, Some sub)),
    (_,     Tabstract (AKdependent d_super, _)) when d_sub <> d_super ->
      let uenv_sub =
        { uenv_sub with
          TUEnv.dep_tys = (r_sub, d_sub)::uenv_sub.TUEnv.dep_tys } in
      (* If an error occurred while subtyping, we produce a unification error
       * so we get the full information on how the dependent type was
       * generated
       *)
      Errors.try_when
        (fun () ->
          sub_type_with_uenv env (uenv_sub, sub) (uenv_super, ty_super))
        ~when_: begin fun () ->
          match sub, TUtils.get_base_type env ty_super with
          | (_, Tclass ((_, y), _)), (_, Tclass ((_, x), _)) when x = y -> false
          | _, _ -> true
        end
        ~do_: (fun _ -> TUtils.simplified_uerror env ty_super ty_sub)
  | (p_sub, (Tclass (x_sub, tyl_sub) as ty_sub_)),
    (p_super, (Tclass (x_super, tyl_super) as ty_super_)) ->
    let cid_super, cid_sub = (snd x_super), (snd x_sub) in
    if cid_super = cid_sub then
      if tyl_super <> [] && List.length tyl_super = List.length tyl_sub
      then
        match Env.get_class env cid_super with
        | None -> fst (Unify.unify env ety_super ety_sub)
        | Some { tc_tparams; _} ->
            let variancel =
              List.map tc_tparams (fun (variance, _, _) -> variance)
            in
            subtype_tparams env cid_super variancel tyl_sub tyl_super
      else fst (Unify.unify env ety_super ety_sub)
    else begin
      let class_ = Env.get_class env cid_sub in
      (match class_ with
        | None -> env
        | Some class_ ->
          let subtype_req_ancestor =
            if class_.tc_kind = Ast.Ctrait || class_.tc_kind = Ast.Cinterface then
              (* a trait is never the runtime type, but it can be used
               * as a constraint if it has requirements for its using
               * classes *)
              let _, ret = List.fold_left ~f:begin fun acc (_p, req_type) ->
                match acc with
                  | _, Some _ -> acc
                  | env, None ->
                    Errors.try_ begin fun () ->
                      let ety_env = {
                        type_expansions = [];
                        substs = SMap.empty;
                        this_ty = ExprDepTy.apply uenv_sub.TUEnv.dep_tys ty_sub;
                        from_class = None;
                      } in
                      let env, req_type =
                        Phase.localize ~ety_env env req_type in
                      let _, req_ty = req_type in
                      env, Some (sub_type env (p_sub, req_ty) ty_super)
                    end (fun _ -> acc)
              end class_.tc_req_ancestors ~init:(env, None) in
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
                    then List.map class_.tc_tparams (fun _ -> (p_sub, Tany))
                    else tyl_sub
                  in
                  if List.length class_.tc_tparams <> List.length tyl_sub
                  then
                    Errors.expected_tparam
                      (Reason.to_pos p_sub) (List.length class_.tc_tparams);
                  (* NOTE: We rely on the fact that we fold all ancestors of
                   * ty_sub in its class_type so we will never hit this case
                   * again. If this ever changes then we would need to store
                   * ty_sub as the 'this_ty' in the uenv and be careful to
                   * thread it through.
                   *
                   * This is covered by test/typecheck/this_tparam2.php
                   *)
                  let ety_env = {
                    type_expansions = [];
                    substs = Subst.make class_.tc_tparams tyl_sub;
                    this_ty = ExprDepTy.apply uenv_sub.TUEnv.dep_tys ty_sub;
                    from_class = None;
                  } in
                  let env, up_obj = Phase.localize ~ety_env env up_obj in
                  sub_type env up_obj ty_super
                | None when class_.tc_members_fully_known ->
                  TUtils.uerror p_super ty_super_ p_sub ty_sub_;
                  env
                | _ -> env
          )
      )
    end
  | _, (_, Tmixed) -> env
  | (_, Tprim (Nast.Tint | Nast.Tfloat)), (_, Tprim Nast.Tnum) -> env
  | (_, Tprim (Nast.Tint | Nast.Tstring)), (_, Tprim Nast.Tarraykey) -> env
  | (r, Tarraykind akind), (_, Tclass ((_, coll), [tv_super]))
    when (coll = SN.Collections.cTraversable ||
        coll = SN.Collections.cContainer) ->
      (match akind with
      | AKany -> env
      | AKempty -> env
      | AKvec tv | AKmap (_, tv) ->
          sub_type env tv tv_super
      | AKshape fdm ->
          Typing_arrays.fold_akshape_as_akmap begin fun env ty_sub ->
            sub_type env ty_sub ty_super
          end env r fdm
      | AKtuple fields ->
          Typing_arrays.fold_aktuple_as_akvec begin fun env ty_sub ->
            sub_type env ty_sub ty_super
          end env r fields
      )
  | (r, Tarraykind akind), (_, Tclass ((_, coll), [tk_super; tv_super]))
    when (coll = SN.Collections.cKeyedTraversable
         || coll = SN.Collections.cKeyedContainer
         || coll = SN.Collections.cIndexish) ->
      (match akind with
      | AKany -> env
      | AKempty -> env
      | AKvec tv ->
        let env = sub_type env (r, Tprim Nast.Tint) tk_super in
        sub_type env tv tv_super
      | AKmap (tk, tv) ->
        let env = sub_type env tk tk_super in
        sub_type env tv tv_super
      | AKshape fdm ->
        Typing_arrays.fold_akshape_as_akmap begin fun env ty_sub ->
          sub_type env ty_sub ty_super
        end env r fdm
      | AKtuple fields ->
          Typing_arrays.fold_aktuple_as_akvec begin fun env ty_sub ->
            sub_type env ty_sub ty_super
          end env r fields
      )
  | (_, Tprim Nast.Tstring), (_, Tclass ((_, stringish), _))
    when stringish = SN.Classes.cStringish -> env
  | (_, Tarraykind _), (_, Tclass ((_, xhp_child), _))
  | (_, Tprim (Nast.Tint | Nast.Tfloat | Nast.Tstring | Nast.Tnum)), (_, Tclass ((_, xhp_child), _))
    when xhp_child = SN.Classes.cXHPChild -> env
  | (_, (Tarraykind (AKvec ty_sub))), (_, (Tarraykind (AKvec ty_super))) ->
      sub_type env ty_sub ty_super
  | (_, Tarraykind AKmap (tk_sub, tv_sub)), (_, (Tarraykind (AKmap (tk_super, tv_super)))) ->
      let env = sub_type env tk_sub tk_super in
      sub_type env tv_sub tv_super
  | (reason, Tarraykind (AKvec elt_ty)), (_, Tarraykind AKmap _) ->
      let int_reason = Reason.Ridx (Reason.to_pos reason, Reason.Rnone) in
      let int_type = int_reason, Tprim Nast.Tint in
      sub_type env (reason, Tarraykind (AKmap (int_type, elt_ty))) ty_super
  | (r, Tarraykind AKshape fdm_sub), _ ->
      Typing_arrays.fold_akshape_as_akmap begin fun env ty_sub ->
        sub_type env ty_sub ty_super
      end env r fdm_sub
  | (r, Tarraykind AKtuple fields_sub), _ ->
      Typing_arrays.fold_aktuple_as_akvec begin fun env ty_sub ->
        sub_type env ty_sub ty_super
      end env r fields_sub
  | _, (_, Toption ty_super) when uenv_super.TUEnv.non_null ->
      sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super)

  (* Subtype is generic parameter
   * We delegate this case to a separate function in order to catch cycles
   * in constraints e.g. <T1 as T2, T2 as T3, T3 as T1>
   * Need to match on this *before* Toption, in order to deal with T<:?t
   *)
  | (_, Tabstract (AKgeneric _, _)), _ ->
    sub_generic_params SSet.empty env (uenv_sub, ty_sub) (uenv_super, ty_super)

  | (_, Toption ty_sub), _ when uenv_sub.TUEnv.non_null ->
    sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super)

  | (_, Toption ty_sub), (_, Toption ty_super) ->
      let uenv_super = {
        TUEnv.non_null = true;
        TUEnv.dep_tys = uenv_super.TUEnv.dep_tys;
      } in
      let uenv_sub = {
        TUEnv.non_null = true;
        TUEnv.dep_tys = uenv_sub.TUEnv.dep_tys;
      } in
      sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super)
  | _, (_, Toption ty_opt) ->
      let uenv_super = {
        TUEnv.non_null = true;
        TUEnv.dep_tys = uenv_super.TUEnv.dep_tys;
      } in
      sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_opt)
  | (_, Ttuple tyl_sub), (_, Ttuple tyl_super)
    when List.length tyl_super = List.length tyl_sub ->
    wfold_left2 sub_type env tyl_sub tyl_super
  | (r_sub, Tfun ft_sub), (r_super, Tfun ft_super) ->
      subtype_funs_generic ~check_return:true env r_sub ft_sub r_super ft_super
  | (r_sub, Tanon (anon_arity, id)), (r_super, Tfun ft)  ->
      (match Env.get_anonymous env id with
      | None ->
          Errors.anonymous_recursive_call (Reason.to_pos r_sub);
          env
      | Some anon ->
          let p_super = Reason.to_pos r_super in
          let p_sub = Reason.to_pos r_sub in
          if not (Unify.unify_arities
                    ~ellipsis_is_variadic:true anon_arity ft.ft_arity)
          then Errors.fun_arity_mismatch p_super p_sub;
          let env, ret = anon env ft.ft_params in
          let env = sub_type env ret ft.ft_ret in
          env
      )
  | (r_sub, Tshape (fields_known_sub, fdm_sub)),
    (r_super, Tshape (fields_known_super, fdm_super)) ->
      fst (TUtils.apply_shape
        ~on_common_field:(fun (env, acc) _ x y -> sub_type env y x, acc)
        ~on_missing_optional_field:(fun acc _ _ -> acc)
        (env, None)
        (r_super, fields_known_super, fdm_super)
        (r_sub, fields_known_sub, fdm_sub))
  | (_, Tabstract (AKnewtype (name_sub, tyl_sub), _)),
    (_, Tabstract (AKnewtype (name_super, tyl_super), _))
    when name_super = name_sub ->
      let td = Env.get_typedef env name_super in
      begin match td with
        | Some {td_tparams; _} ->
          let variancel =
            List.map td_tparams (fun (variance, _, _) -> variance) in
          subtype_tparams env name_super variancel tyl_sub tyl_super
        | _ -> env
      end
  | (_, Tabstract ((AKnewtype (_, _) | AKenum _), Some x)), _ ->
      Errors.try_
        (fun () ->
          fst @@ Unify.unify env ty_super ty_sub
        )
        (fun _ -> sub_type_with_uenv env (uenv_sub, x) (uenv_super, ty_super))
  | (r, Tabstract (AKdependent d, Some ty)), _ ->
      Errors.try_
        (fun () -> fst (Unify.unify env ty_super ty_sub))
        (fun _ ->
          let uenv_sub =
            { uenv_sub with
              TUEnv.dep_tys = (r, d)::uenv_sub.TUEnv.dep_tys } in
          sub_type_with_uenv env (uenv_sub, ty) (uenv_super, ty_super))

  (* Supertype is generic parameter
   * We delegate this case to a separate function in order to catch cycles
   * in constraints e.g. <T1 as T2, T2 as T3, T3 as T1>
   *)
  | _, (_, Tabstract (AKgeneric _, _)) ->
    sub_generic_params SSet.empty env (uenv_sub, ty_sub) (uenv_super, ty_super)

  | _, (_, (Tarraykind _ | Tprim _ | Tvar _
    | Tabstract (_, _) | Ttuple _ | Tanon (_, _) | Tfun _
    | Tobject | Tshape _ | Tclass (_, _))
    ) -> fst (Unify.unify env ty_super ty_sub)

and sub_generic_params seen env (uenv_sub, ty_sub) (uenv_super, ty_super) =
  let env, ety_super = Env.expand_type env ty_super in
  let env, ety_sub = Env.expand_type env ty_sub in
  match ety_sub, ety_super with

  (* Subtype is generic parameter *)
  | (r_sub, Tabstract (AKgeneric name_sub, opt_sub_cstr)), _ ->
    let default () =
         (* If we've seen this type parameter before then we must have gone
         * round a cycle so we fail
         *)
        (if SSet.mem name_sub seen
        then fst (Unify.unify env ty_super ty_sub)
        else
          let seen = SSet.add name_sub seen in
          (* Otherwise, we collect all the upper bounds ("as" constraints) on
             the generic parameter, and check each of these in turn against
             ty_super until one of them succeeds
           *)
          let rec try_bounds tyl =
            match tyl with
            | [] ->
              (* There are no bounds so force an error *)
              fst (Unify.unify env ty_super ty_sub)

            | ty::tyl ->
              Errors.try_
              (fun () ->
                sub_generic_params seen env (uenv_sub, ty)
                                            (uenv_super, ty_super))
              (fun l ->
               (* Right now we report constraint failure based on the last
                * error. This should change when we start supporting
                * multiple constraints *)
                 if List.is_empty tyl
                 then (Reason.explain_generic_constraint
                     env.Env.pos r_sub name_sub l; env)
                 else try_bounds tyl)
          in try_bounds (Option.to_list opt_sub_cstr @
              Env.get_upper_bounds env name_sub)) in

    begin match ety_super with
      (* If supertype is the same generic parameter, we're done *)
      | (_, Tabstract (AKgeneric name_super, _)) when name_sub = name_super
        -> env

      (* Try this first *)
      | (_, Toption ty_super) ->
        Errors.try_
          (fun () ->
          let uenv_super = {
            TUEnv.non_null = true;
            TUEnv.dep_tys = uenv_super.TUEnv.dep_tys;
          } in
          sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super))
          (fun _ ->
              default ())

      | _ -> default ()
    end

  (* Supertype is generic parameter *)
  | _, (r_super, Tabstract (AKgeneric name_super, _)) ->
    (* If we've seen this type parameter before then we must have gone
     * round a cycle so we fail
     *)
    if SSet.mem name_super seen
    then fst (Unify.unify env ty_super ty_sub)
    else
      let seen = SSet.add name_super seen in
      (* Collect all the lower bounds ("super" constraints) on the
       * generic parameter, and check ty_sub against each of them in turn
       * until one of them succeeds *)
      let rec try_bounds tyl =
        match tyl with
        | [] ->
          (* There are no bounds so force an error *)
          fst (Unify.unify env ty_super ty_sub)

        | ty::tyl ->
          Errors.try_
            (fun () -> sub_generic_params seen env (uenv_sub, ty_sub)
                                                   (uenv_super, ty))
          (fun l ->
           (* Right now we report constraint failure based on the last
            * error. This should change when we start supporting
              multiple constraints *)
             if List.is_empty tyl
             then (Reason.explain_generic_constraint
                 env.Env.pos r_super name_super l; env)
             else try_bounds tyl)
      in try_bounds (Env.get_lower_bounds env name_super)

  | _, _ ->
    sub_type_with_uenv env (uenv_sub, ty_sub) (uenv_super, ty_super)

and is_sub_type env ty_sub ty_super =
  Errors.try_
    (fun () -> ignore(sub_type env ty_sub ty_super); true)
    (fun _ -> false)

and sub_string p env ty2 =
  let env, ety2 = Env.expand_type env ty2 in
  match ety2 with
  | (_, Toption ty2) -> sub_string p env ty2
  | (_, Tunresolved tyl) ->
      List.fold_left tyl ~f:(sub_string p) ~init:env
  | (_, Tprim _) ->
      env
  | (_, Tabstract (AKenum _, _)) ->
      (* Enums are either ints or strings, and so can always be used in a
       * stringish context *)
      env
  | (_, Tabstract _) ->
    begin match TUtils.get_concrete_supertypes env ty2 with
      | env, None ->
        fst (Unify.unify env (Reason.Rwitness p, Tprim Nast.Tstring) ty2)
      | env, Some ty ->
        sub_string p env ty
    end
  | (r2, Tclass (x, _)) ->
      let class_ = Env.get_class env (snd x) in
      (match class_ with
      | None -> env
      | Some tc
          (* A Stringish is a string or an object with a __toString method
           * that will be converted to a string *)
          when tc.tc_name = SN.Classes.cStringish
          || SMap.mem SN.Classes.cStringish tc.tc_ancestors ->
        env
      | Some _ ->
        Errors.object_string p (Reason.to_pos r2);
        env
      )
  | _, Tany ->
    env (* Unifies with anything *)
  | _, Tobject -> env
  | _, (Tmixed | Tarraykind _ | Tvar _
    | Ttuple _ | Tanon (_, _) | Tfun _ | Tshape _) ->
      fst (Unify.unify env (Reason.Rwitness p, Tprim Nast.Tstring) ty2)

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.sub_type_ref := sub_type

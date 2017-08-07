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
open Typing_defs

module ExprDepTy = struct
  module N = Nast
  module Env = Typing_env
  module TUtils = Typing_utils

  let to_string dt = AbstractKind.to_string (AKdependent dt)

  let new_() =
    let eid = Ident.tmp() in
    Reason.ERexpr eid, `expr eid

  let from_cid env reason cid =
    let pos = Reason.to_pos reason in
    let pos, expr_dep_reason, dep = match cid with
      | N.CIparent ->
          (match Env.get_parent env with
          | _, Tapply ((_, cls), _) ->
              pos, Reason.ERparent cls, `cls cls
          | _, _ ->
              let ereason, dep = new_() in
              pos, ereason, dep
          )
      | N.CIself ->
          (match Env.get_self env with
          | _, Tclass ((_, cls), _) ->
              pos, Reason.ERself cls, `cls cls
          | _, _ ->
              let ereason, dep = new_() in
              pos, ereason, dep
          )
      | N.CI ((p, cls), _) ->
          p, Reason.ERclass cls, `cls cls
      | N.CIstatic ->
          pos, Reason.ERstatic, `static
      | N.CIexpr (p, N.This) ->
          p, Reason.ERstatic, `static
      (* If it is a local variable then we look up the expression id associated
       * with it. If one doesn't exist we generate a new one. We are being
       * conservative here because the new expression id we create isn't
       * added to the local enviornment.
       *)
      | N.CIexpr (p, N.Lvar (_, x)) ->
          let ereason, dep = match Env.get_local_expr_id env x with
            | Some eid -> Reason.ERexpr eid, `expr eid
            | None -> new_() in
          p, ereason, dep
      (* If all else fails we generate a new identifier for our expression
       * dependent type.
       *)
      | N.CIexpr (p, _) ->
          let ereason, dep = new_() in
          p, ereason, dep in
    (Reason.Rexpr_dep_type (reason, pos, expr_dep_reason), (dep, []))

  (* Takes the given list of dependent types and applies it to the given
   * locl ty to create a new locl ty
   *)
  let apply env dep_tys ty =
    let apply_single env dep_tys ty =
      List.fold_left dep_tys ~f:begin fun (env, ty) (r, dep_ty) ->
        match dep_ty with
        | (_, []) ->
         env, (r, Tabstract (AKdependent dep_ty, Some ty))
        | _ ->
          begin match ty with
          (* If the generic in question is exactly equal to something, the
          expression dependent type collapses to that given type, since
          all constraints of the expression dependent type will get
          transferred to the lower type. *)
          (* For example, if we have the following
            abstract class Box {
              abstract const type T;
            }
            class IntBox { const type T = int; }
            function addFiveToValue<T1 as Box>(T1 $x) : int where T1::T = int {
                return $x->get() + 5;
            }
            Here, $x->get() has type expr#1::T as T1::T as Box::T.
            But T1::T is exactly equal to int, so $x->get() no longer needs
            to be expression dependent. Thus, $x->get() typechecks.
          *)
            | (_, Tabstract (AKgeneric s, _)) when
            Env.get_equal_bounds env s <> [] ->
            (env, ty)
            | _ ->
             let ty_name = to_string dep_ty in
             let new_ty = (r, Tabstract(AKgeneric ty_name, None)) in
              let env = Env.add_upper_bound_global env ty_name ty in
              (env, new_ty)
          end
      end ~init:(env, ty) in
    let env, ety = Env.expand_type env ty in
    match ety with
    | _, Tunresolved [x] when dep_tys = [] ->
       env, x
    | r, Tunresolved tyl ->
      let env, tyl = List.fold tyl ~f:(fun (env, acc) ty ->
        let env, ty = apply_single env dep_tys ty in
        env, ty::acc) ~init:(env, []) in
      env, (r, Tunresolved tyl)
    | _ -> apply_single env dep_tys ety


  (* We do not want to create a new expression dependent type if the type is
   * already expression dependent. However if the type is Tunresolved that
   * contains different expression dependent types then we will want to
   * generate a new dependent type. For example:
   *
   *  if ($cond) {
   *    $x = new A(); // Dependent type (`cls '\A')
   *  } else {
   *    $x = new B(); // Dependent type (`cls '\B')
   *  }
   *  $x; // Tunresolved[(`cls '\A', `cls '\B')]
   *
   *  // When we call the function below, we need to generate
   *  // A new expression dependent type since
   *  // (`cls '\A') <> (\cls '\B')
   *  $x->expression_dependent_function();
   *)
  let rec should_apply env ty =
    let env, ty = Env.expand_type env ty in
    match snd ty with
    | Tabstract(AKgeneric s, _) when AbstractKind.is_generic_dep_ty s ->
      false
    | Tabstract (AKgeneric _, _) ->
      let env, tyl = Typing_utils.get_concrete_supertypes env ty in
      List.exists tyl (should_apply env)
    | Toption ty
    | Tabstract (
        ( AKnewtype _
        | AKenum _
        | AKdependent (`this, [])
        ), Some ty) ->
        should_apply env ty
    | Tabstract (AKdependent _, Some _) ->
        false
    | Tunresolved tyl ->
        List.exists tyl (should_apply env)
    | Tclass ((_, x), _) ->
        let class_ = Env.get_class env x in
        (* If a class is both final and variant, we must treat it as non-final
         * since we can't statically guarantee what the runtime type
         * will be.
         *)
        Option.value_map class_
          ~default:false
          ~f:(fun class_ty ->
              not (TUtils.class_is_final_and_not_contravariant class_ty))
    | Tanon _ | Tobject | Tmixed | Tprim _ | Tshape _ | Ttuple _
    | Tarraykind _ | Tfun _ | Tabstract (_, None) | Tany | Tvar _ | Terr ->
        false

  (****************************************************************************)
  (* A type access "this::T" is translated to "<this>::T" during the
   * naming phase. While typing a body, "<this>" is a type hole that needs to
   * be filled with a final concrete type. Resolution is specified in typing.ml,
   * here is a high level break down:
   *
   * 1) When a class member "bar" is accessed via "[CID]->bar" or "[CID]::bar"
   * we resolve "<this>" in the type of "bar" to "<[CID]>"
   *
   * 2) When typing a method, we resolve "<this>" in the return type to
   * "this"
   *
   * 3) When typing a method, we resolve "<this>" in parameters of the
   * function to "<static>" in static methods or "<$this>" in non-static
   * methods
   *
   * More specific details are explained inline
   *)
  (****************************************************************************)
  let make env cid cid_ty =
    if should_apply env cid_ty then
      apply env [from_cid env (fst cid_ty) cid] cid_ty
    else
      env, cid_ty
end

(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs

module ExprDepTy = struct
  module N = Aast
  module Env = Typing_env
  module TUtils = Typing_utils

  let new_ () =
    let eid = Ident.tmp () in
    (Reason.ERexpr eid, DTexpr eid)

  let from_cid env reason cid =
    let pos = Reason.to_pos reason in
    let (pos, expr_dep_reason, dep) =
      match cid with
      | N.CIparent ->
        (match Env.get_parent_ty env with
        | (_, Tapply ((_, cls), _)) -> (pos, Reason.ERparent cls, DTcls cls)
        | (_, _) ->
          let (ereason, dep) = new_ () in
          (pos, ereason, dep))
      | N.CIself ->
        (match Env.get_self env with
        | (_, Tclass ((_, cls), _, _)) -> (pos, Reason.ERself cls, DTcls cls)
        | (_, _) ->
          let (ereason, dep) = new_ () in
          (pos, ereason, dep))
      | N.CI (p, cls) -> (p, Reason.ERclass cls, DTcls cls)
      | N.CIstatic -> (pos, Reason.ERstatic, DTthis)
      | N.CIexpr (p, N.This) -> (p, Reason.ERstatic, DTthis)
      (* If it is a local variable then we look up the expression id associated
       * with it. If one doesn't exist we generate a new one. We are being
       * conservative here because the new expression id we create isn't
       * added to the local enviornment.
       *)
      | N.CIexpr (p, N.Lvar (_, x)) ->
        let (ereason, dep) =
          match Env.get_local_expr_id env x with
          | Some eid -> (Reason.ERexpr eid, DTexpr eid)
          | None -> new_ ()
        in
        (p, ereason, dep)
      (* If all else fails we generate a new identifier for our expression
       * dependent type.
       *)
      | N.CIexpr (p, _) ->
        let (ereason, dep) = new_ () in
        (p, ereason, dep)
    in
    (Reason.Rexpr_dep_type (reason, pos, expr_dep_reason), dep)

  (* Takes the given list of dependent types and applies it to the given
   * locl ty to create a new locl ty
   *)
  let rec apply env ~dep_ty ty =
    let (env, ety) = Env.expand_type env ty in
    match ety with
    | (r, Toption ty) ->
      let (env, ty) = apply env ~dep_ty ty in
      (env, (r, Toption ty))
    | (r, Tunion tyl) ->
      let (env, tyl) = List.fold_map tyl ~init:env ~f:(apply ~dep_ty) in
      Typing_union.union_list env r tyl
    | (r, Tintersection tyl) ->
      let (env, tyl) = List.fold_map tyl ~init:env ~f:(apply ~dep_ty) in
      Typing_intersection.intersect_list env r tyl
    | (_, Tvar _) -> (env, ty)
    | _ ->
      let (r, dep_ty) = dep_ty in
      (match (dep_ty, ety) with
      (* Always represent exact types without an access path as Tclass(_,Exact,_) *)
      | (DTcls n, (_, Tclass (c, _, tyl))) when n = snd c ->
        (env, (r, Tclass (c, Exact, tyl)))
      | (dep_ty, _) -> (env, (r, Tabstract (AKdependent dep_ty, Some ety))))

  (* We do not want to create a new expression dependent type if the type is
   * already expression dependent. However if the type is Tunion that
   * contains different expression dependent types then we will want to
   * generate a new dependent type. For example:
   *
   *  if ($cond) {
   *    $x = new A(); // Dependent type (DTcls '\A')
   *  } else {
   *    $x = new B(); // Dependent type (DTcls '\B')
   *  }
   *  $x; // Tunion[(DTcls '\A', DTcls '\B')]
   *
   *  // When we call the function below, we need to generate
   *  // A new expression dependent type since
   *  // (DTcls '\A') <> (\cls '\B')
   *  $x->expression_dependent_function();
   *)
  let rec should_apply env ty =
    let (env, ty) = Env.expand_type env ty in
    match snd ty with
    | Tabstract (AKgeneric s, _) when AbstractKind.is_generic_dep_ty s -> false
    | Tabstract (AKgeneric _, _) ->
      let (env, tyl) = Typing_utils.get_concrete_supertypes env ty in
      List.exists tyl (should_apply env)
    | Toption ty
    | Tabstract (AKnewtype _, Some ty) ->
      should_apply env ty
    | Tabstract (AKdependent _, Some _) -> false
    | Tunion tyl -> List.exists tyl (should_apply env)
    | Tintersection tyl -> List.exists tyl (should_apply env)
    | Tclass (_, Exact, _) -> false
    | Tclass ((_, x), _, _) ->
      let class_ = Env.get_class env x in
      (* If a class is both final and variant, we must treat it as non-final
       * since we can't statically guarantee what the runtime type
       * will be.
       *)
      Option.value_map class_ ~default:false ~f:(fun class_ty ->
          not (TUtils.class_is_final_and_not_contravariant class_ty))
    | Tanon _
    | Tobject
    | Tnonnull
    | Tprim _
    | Tshape _
    | Ttuple _
    | Tdynamic
    | Tarraykind _
    | Tfun _
    | Tabstract (_, None)
    | Tany _
    | Tvar _
    | Terr
    | Tdestructure _ ->
      false
    | Tpu _ -> false
    | Tpu_access (ty, _) -> should_apply env ty

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
  let rec make env ~cid cid_ty =
    let (env, cid_ty) = Env.expand_type env cid_ty in
    match cid_ty with
    | (r, Tintersection tyl) ->
      let (env, tyl) = List.fold_map tyl ~init:env ~f:(make ~cid) in
      (env, (r, Tintersection tyl))
    | _ ->
      if should_apply env cid_ty then
        apply env (from_cid env (fst cid_ty) cid) cid_ty
      else
        (env, cid_ty)
end

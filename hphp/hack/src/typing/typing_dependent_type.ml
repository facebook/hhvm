(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
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
        (match Env.get_parent_id env with
        | Some cls -> (pos, Reason.ERparent cls, DTcls cls)
        | None ->
          let (ereason, dep) = new_ () in
          (pos, ereason, dep))
      | N.CIself ->
        (match get_node (Env.get_self env) with
        | Tclass ((_, cls), _, _) -> (pos, Reason.ERself cls, DTcls cls)
        | _ ->
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
  let make env ~cid ty =
    let (r_dep_ty, dep_ty) = from_cid env (get_reason ty) cid in
    let apply env ty = (env, mk (r_dep_ty, Tdependent (dep_ty, ty))) in
    let rec make env ty =
      let (env, ety) = Env.expand_type env ty in
      match deref ety with
      | (_, Tclass (_, Exact, _)) -> (env, ty)
      | (_, Tclass (((_, x) as c), Nonexact, tyl)) ->
        let class_ = Env.get_class env x in
        (* If a class is both final and variant, we must treat it as non-final
        * since we can't statically guarantee what the runtime type
        * will be.
        *)
        if
          Option.value_map class_ ~default:false ~f:(fun class_ty ->
              TUtils.class_is_final_and_not_contravariant class_ty)
        then
          (env, ty)
        else (
          match dep_ty with
          | DTcls n when String.equal n x ->
            (env, mk (r_dep_ty, Tclass (c, Exact, tyl)))
          | _ -> apply env ty
        )
      | (_, Tgeneric s) when DependentKind.is_generic_dep_ty s -> (env, ty)
      | (_, Tgeneric _) ->
        let (env, tyl) = Typing_utils.get_concrete_supertypes env ty in
        let (env, tyl') = List.fold_map tyl ~init:env ~f:make in
        if tyl_equal tyl tyl' then
          (env, ty)
        else
          apply env ty
      | (r, Toption ty) ->
        let (env, ty) = make env ty in
        (env, mk (r, Toption ty))
      | (r, Tnewtype (n, p, ty)) ->
        let (env, ty) = make env ty in
        (env, mk (r, Tnewtype (n, p, ty)))
      | (_, Tdependent (_, _)) -> (env, ty)
      | (r, Tunion tyl) ->
        let (env, tyl) = List.fold_map tyl ~init:env ~f:make in
        (env, mk (r, Tunion tyl))
      | (r, Tintersection tyl) ->
        let (env, tyl) = List.fold_map tyl ~init:env ~f:make in
        (env, mk (r, Tintersection tyl))
      (* TODO(T36532263) check if this is legal *)
      | ( _,
          ( Tobject | Tnonnull | Tprim _ | Tshape _ | Ttuple _ | Tdynamic
          | Tarraykind _ | Tfun _ | Tany _ | Tvar _ | Terr | Tpu _
          | Tpu_type_access _ ) ) ->
        (env, ty)
    in
    make env ty
end

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

  type dep =
    | Dep_This
    | Dep_Cls of string
    | Dep_Expr of Ident_provider.Ident.t

  let new_ env =
    let eid = Env.make_ident env in
    (Reason.ERexpr eid, Dep_Expr eid)

  (* Convert a class_id into a "dependent kind" (dep). This later informs the make_with_dep_kind function
   * how to translate a type suitable for substituting for `this` in a method signature.
   *)
  let from_cid env reason (cid : Nast.class_id_) =
    let pos = Reason.to_pos reason in
    let (pos, expr_dep_reason, dep) =
      match cid with
      | N.CIparent ->
        (match Env.get_parent_id env with
        | Some cls -> (pos, Reason.ERparent cls, Dep_Cls cls)
        | None ->
          let (ereason, dep) = new_ env in
          (pos, ereason, dep))
      | N.CIself ->
        (match Env.get_self_id env with
        | Some cls -> (pos, Reason.ERself cls, Dep_Cls cls)
        | None ->
          let (ereason, dep) = new_ env in
          (pos, ereason, dep))
      | N.CI (p, cls) ->
        (Pos_or_decl.of_raw_pos p, Reason.ERclass cls, Dep_Cls cls)
      | N.CIstatic -> (pos, Reason.ERstatic, Dep_This)
      | N.CIexpr (_, p, N.This) ->
        (Pos_or_decl.of_raw_pos p, Reason.ERstatic, Dep_This)
      (* If it is a local variable then we look up the expression id associated
       * with it. If one doesn't exist we generate a new one. We are being
       * conservative here because the new expression id we create isn't
       * added to the local enviornment.
       *)
      | N.CIexpr (_, p, N.Lvar (_, x)) ->
        let (ereason, dep) =
          match Env.get_local_expr_id env x with
          | Some eid -> (Reason.ERexpr eid, Dep_Expr eid)
          | None -> new_ env
        in
        (Pos_or_decl.of_raw_pos p, ereason, dep)
      (* If all else fails we generate a new identifier for our expression
       * dependent type.
       *)
      | N.CIexpr (_, p, _) ->
        let (ereason, dep) = new_ env in
        (Pos_or_decl.of_raw_pos p, ereason, dep)
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
  let make_with_dep_kind env dep_kind ty =
    let (r_dep_ty, dep_ty) = dep_kind in
    let apply env ty =
      match dep_ty with
      | Dep_Cls _ ->
        (env, mk (r_dep_ty, Tdependent (DTexpr (Env.make_ident env), ty)))
      | Dep_This -> (env, ty)
      | Dep_Expr id -> (env, mk (r_dep_ty, Tdependent (DTexpr id, ty)))
    in
    let rec make ~seen env ty =
      let (env, ty) = Env.expand_type env ty in
      match deref ty with
      | (_, Tclass (_, Exact, _)) -> (env, ty)
      | (_, Tclass (((_, x) as c), Nonexact _, tyl)) ->
        let class_ = Env.get_class env x in
        (* If a class is both final and variant, we must treat it as non-final
           * since we can't statically guarantee what the runtime type
           * will be.
        *)
        if
          Option.value_map
            (Decl_entry.to_option class_)
            ~default:false
            ~f:(fun class_ty -> TUtils.class_is_final_and_invariant class_ty)
        then
          (env, ty)
        else (
          match dep_ty with
          | Dep_Cls n when String.equal n x ->
            (env, mk (r_dep_ty, Tclass (c, Exact, tyl)))
          | _ -> apply env ty
        )
      | (_, Tgeneric (s, _tyargs)) when DependentKind.is_generic_dep_ty s ->
        (* TODO(T69551141) handle type arguments *)
        (env, ty)
      | (_, Tgeneric (name, _)) ->
        if SSet.mem name seen then
          (env, ty)
        else
          (* TODO(T69551141) handle type arguments here? *)
          let (env, tyl) =
            TUtils.get_concrete_supertypes ~abstract_enum:true env ty
          in
          let (env, tyl') =
            List.fold_map tyl ~init:env ~f:(make ~seen:(SSet.add name seen))
          in
          if tyl_equal tyl tyl' then
            (env, ty)
          else
            apply env ty
      | (r, Toption ty) ->
        let (env, ty) = make ~seen env ty in
        (env, mk (r, Toption ty))
      | (_, Tunapplied_alias _) ->
        Typing_defs.error_Tunapplied_alias_in_illegal_context ()
      | (r, Tnewtype (n, p, ty)) ->
        let (env, ty) = make ~seen env ty in
        (env, mk (r, Tnewtype (n, p, ty)))
      | (_, Tdependent (_, _)) -> (env, ty)
      | (r, Tunion tyl) ->
        let (env, tyl) = List.fold_map tyl ~init:env ~f:(make ~seen) in
        (env, mk (r, Tunion tyl))
      | (r, Tintersection tyl) ->
        let (env, tyl) = List.fold_map tyl ~init:env ~f:(make ~seen) in
        (env, mk (r, Tintersection tyl))
      | (r, Taccess (ty, ids)) ->
        let (env, ty) = make ~seen env ty in
        (env, mk (r, Taccess (ty, ids)))
      (* TODO(T36532263) check if this is legal *)
      | ( _,
          ( Tnonnull | Tprim _ | Tshape _ | Ttuple _ | Tdynamic | Tvec_or_dict _
          | Tfun _ | Tany _ | Tvar _ | Tneg _ ) ) ->
        (env, ty)
    in
    make ~seen:SSet.empty env ty

  let make env ~cid ty =
    make_with_dep_kind env (from_cid env (get_reason ty) cid) ty
end

(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Common
open Aast
open Tast
open Ast_defs
open Typing_defs
module Cls = Decl_provider.Class
module Env = Tast_env
module MakeType = Typing_make_type
module SN = Naming_special_names

(** Expand ty to its concrete supertypes if it is abstract.
    Expand abstract types within Tunion.
    Replace empty Tunion with Typing_defs.make_tany (). *)
let rec make_concrete env ty =
  let (env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tgeneric _
  | Tnewtype _
  | Tdependent _ ->
    let (env, tyl) = Env.get_concrete_supertypes ~abstract_enum:true env ty in
    begin
      match tyl with
      | [] -> begin
        (* FIXME: When we have a type parameter with no upper bounds, it may be
           because of T30081797. Until that issue is fixed, just ignore generics
           that have no known upper bounds in the TAST. *)
        match get_node ty with
        | Tgeneric _ -> (env, mk (Reason.Rnone, Typing_defs.make_tany ()))
        | _ -> (env, ty)
      end
      | tyl -> make_concrete env (MakeType.union (get_reason ty) tyl)
    end
  | Tunion tyl -> begin
    match tyl with
    | [] -> (env, mk (Reason.Rnone, Typing_defs.make_tany ()))
    | tyl ->
      let (env, tyl) = List.map_env env tyl ~f:make_concrete in
      Env.union_list env (get_reason ty) tyl
  end
  | _ -> (env, ty)

let enum_base_type env cid =
  match Decl_provider.get_class (Typing_env.get_ctx env) cid with
  | None -> (env, None)
  | Some cls ->
    (match Cls.enum_type cls with
    | None -> (env, None)
    | Some te ->
      let ((env, ty_err_opt), ty) =
        Typing_phase.localize_no_subst env ~ignore_errors:true te.te_base
      in
      Option.iter ~f:Errors.add_typing_error ty_err_opt;
      (env, Some ty))

let opaque_enum_expander expanded =
  object
    inherit Type_mapper.deep_type_mapper as super

    inherit! Type_mapper.tvar_expanding_type_mapper

    method! on_tnewtype env r cid tyl cstr =
      match get_node cstr with
      | Tprim Tarraykey when Typing_env.is_enum env cid ->
        let (env, cstr') = enum_base_type env cid in
        begin
          match cstr' with
          | None -> (env, mk (r, Tnewtype (cid, tyl, cstr)))
          | Some ty ->
            (match get_node ty with
            | Tprim Tarraykey -> (env, mk (r, Tnewtype (cid, tyl, cstr)))
            | _ ->
              expanded := SSet.add cid !expanded;
              (env, ty))
        end
      | _ -> super#on_tnewtype env r cid tyl cstr
  end

let may_be_interface env ty =
  List.exists (Env.get_class_ids env ty) ~f:(fun cid ->
      match Decl_provider.get_class (Env.get_ctx env) cid with
      (* Avoid false positives by assuming missing classes are interfaces *)
      | None -> true
      | Some cid -> Ast_defs.is_c_interface (Cls.kind cid))

(** Returns [true] when one type is a subtype of the other or when one coerces
    to the other. *)
let can_subtype_or_coerce env ty1 ty2 =
  let env = Env.tast_env_as_typing_env env in
  Option.is_some
    (Typing_coercion.try_coerce
       env
       ty1
       { et_type = ty2; et_enforced = Unenforced })
  || Option.is_some
       (Typing_coercion.try_coerce
          env
          ty2
          { et_type = ty1; et_enforced = Unenforced })

let expanded_types_equatable env ty1 ty2 =
  (* If one of these types is an interface type, we cannot conclude anything
     about the equatability of this pair of types from their subtyping behavior,
     since we have multiple interface inheritance. We could only conclude that a
     comparison was trivial if we knew that no class exists which is a subclass
     of both types. Since we don't have a simple way to determine this, we
     ignore comparisons involving interface types. *)
  may_be_interface env ty1
  || may_be_interface env ty2
  || can_subtype_or_coerce env ty1 ty2

(** Given a list of type pairs known not to be equatable, return the set of
    names of opaque enum types which, if made transparent, would make at least
    one of those pairs equatable. *)
let collect_enums_whose_opacity_prevents_equatability env cartesian_product =
  List.fold cartesian_product ~init:SSet.empty ~f:(fun acc (ty1, ty2) ->
      let expanded = ref SSet.empty in
      let expander = opaque_enum_expander expanded in
      let env = Env.tast_env_as_typing_env env in
      let (env, ty1) = expander#on_type env ty1 in
      let (env, ty2) = expander#on_type env ty2 in
      let env = Env.typing_env_as_tast_env env in
      let expanded = !expanded in
      if expanded_types_equatable env ty1 ty2 then
        SSet.union acc expanded
      else
        acc)

type equatability =
  | Equatable
  | NonEquatable
  | NonEquatableDueToOpaqueEnums of string list

let non_null env ty =
  let (env, ty) = Env.expand_type env ty in
  if is_dynamic ty then
    (env, ty)
  else
    Env.non_null env (get_pos ty) ty

(** Attempts to return [Equatable] when the given types have a non-empty
    intersection (or when they are both interface types, and we cannot easily
    determine whether they have a non-empty intersection). Returns
    [NonEquatableDueToOpaqueEnums] when the intersection is empty, but would be
    non-empty if the contained list of opaque enum types were made transparent.
    Otherwise, returns [NonEquatable]. *)
let equatability env ty1 ty2 =
  (* Unwrap Toption, so that when we have A <: B (and not B <: A), we consider
     ?A and B equatable (we wouldn't otherwise, since it is not the case that
     ?A <: B or B <: ?A) *)
  let (env, ty1) = non_null env ty1 in
  let (env, ty2) = non_null env ty2 in
  let (env, ty1) = make_concrete env ty1 in
  let (env, ty2) = make_concrete env ty2 in
  let tyl1 =
    match get_node ty1 with
    | Tunion tyl -> tyl
    | _ -> [ty1]
  in
  let tyl2 =
    match get_node ty2 with
    | Tunion tyl -> tyl
    | _ -> [ty2]
  in
  let product = List.cartesian_product tyl1 tyl2 in
  if List.exists product ~f:(fun (a, b) -> expanded_types_equatable env a b)
  then
    Equatable
  else
    let equatable_if_these_are_made_transparent =
      collect_enums_whose_opacity_prevents_equatability env product
      |> SSet.elements
      |> List.map ~f:Utils.strip_ns
    in
    if List.is_empty equatable_if_these_are_made_transparent then
      NonEquatable
    else
      NonEquatableDueToOpaqueEnums equatable_if_these_are_made_transparent

let error_if_inequatable env p ty1 ty2 err =
  match equatability env ty1 ty2 with
  | Equatable -> ()
  | NonEquatable -> err (Env.print_ty env ty1) (Env.print_ty env ty2)
  | NonEquatableDueToOpaqueEnums enums ->
    Lints_errors.non_equatable_due_to_opaque_types
      p
      (Env.print_ty env ty1)
      (Env.print_ty env ty2)
      enums

let ensure_valid_equality_check env p bop e1 e2 =
  let ty1 = get_type e1 in
  let ty2 = get_type e2 in
  match (get_node ty1, get_node ty2) with
  | (Tprim _, Tprim _) ->
    (* This is already checked in the type checker with
        TrivialStrictEq (error 4118). *)
    ()
  | _ ->
    error_if_inequatable
      env
      p
      ty1
      ty2
      (Lints_errors.non_equatable_comparison p (equal_bop bop Diff2))

let ensure_valid_null_check ~nonnull env p e =
  let ty = get_type e in
  if Tast_utils.type_non_nullable env ty then
    Lints_errors.invalid_null_check p nonnull (Env.print_ty env ty)

let ensure_valid_contains_check env p trv_val_ty val_ty =
  error_if_inequatable
    env
    p
    trv_val_ty
    val_ty
    (Lints_errors.invalid_contains_check p)

let ensure_valid_contains_key_check env p trv_key_ty key_ty =
  error_if_inequatable
    env
    p
    trv_key_ty
    key_ty
    (Lints_errors.invalid_contains_key_check p)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, p, Binop { bop = (Diff2 | Eqeqeq) as bop; lhs = e1; rhs = e2 }) ->
      begin
        match (e1, e2) with
        | ((_, _, Null), e)
        | (e, (_, _, Null)) ->
          ensure_valid_null_check ~nonnull:(equal_bop bop Diff2) env p e
        | _ -> ensure_valid_equality_check env p bop e1 e2
      end
      | (_, p, Call ((_, _, Id (_, id)), [(tv1, _); (tv2, _)], _, _))
        when String.equal id SN.HH.contains ->
        ensure_valid_contains_check env p tv1 tv2
      | (_, p, Call ((_, _, Id (_, id)), [(tk1, _); (tk2, _); _], _, _))
        when String.equal id SN.HH.contains_key ->
        ensure_valid_contains_key_check env p tk1 tk2
      | _ -> ()
  end

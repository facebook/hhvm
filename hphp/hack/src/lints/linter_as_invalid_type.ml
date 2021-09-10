(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Typing_defs
module SSet = Set.Make (String)
module Cls = Decl_provider.Class

let enum_base_type env cid =
  match Decl_provider.get_class (Tast_env.get_ctx env) cid with
  | None -> None
  | Some cls ->
    (match Cls.enum_type cls with
    | None -> None
    | Some te ->
      let (_, ty) =
        Tast_env.localize_no_subst env ~ignore_errors:true te.te_base
      in
      Some ty)

(* If [ty] can be considered as a union of primitive types, return a
   string set of those types. *)
let rec type_as_primitives env ty : SSet.t option =
  match get_node ty with
  | Toption nulltype ->
    (match get_node nulltype with
    | Tnonnull -> None
    | _ ->
      (match type_as_primitives env nulltype with
      | Some primitives -> Some (SSet.add "null" primitives)
      | None -> None))
  | Tprim Aast.Tnull -> Some (SSet.singleton "null")
  | Tprim Aast.Tbool -> Some (SSet.singleton "bool")
  | Tprim Aast.Tfloat -> Some (SSet.singleton "float")
  | Tprim Aast.Tint -> Some (SSet.singleton "int")
  | Tprim Aast.Tstring -> Some (SSet.singleton "string")
  | Tprim Aast.Tarraykey -> Some (SSet.of_list ["string"; "int"])
  | Tprim Aast.Tnum -> Some (SSet.of_list ["int"; "float"])
  | Tnewtype (cid, _, _) when Tast_env.is_enum env cid ->
    let enum_type = enum_base_type env cid in
    begin
      match enum_type with
      | None -> Some (SSet.of_list ["string"; "int"])
      | Some ty ->
        (match get_node ty with
        | Tprim Tarraykey -> Some (SSet.of_list ["string"; "int"])
        | Tprim Tint -> Some (SSet.singleton "int")
        | Tprim Tstring -> Some (SSet.singleton "string")
        | _ -> None)
    end
  | _ -> None

let refinable_check pos env lhs_ty rhs_ty =
  let (env, lhs_ty) = Tast_env.expand_type env lhs_ty in
  let (_, rhs_ty) = Tast_env.expand_type env rhs_ty in
  match (type_as_primitives env lhs_ty, type_as_primitives env rhs_ty) with
  | (Some lhs_primitives, Some rhs_primitives) ->
    if SSet.is_empty (SSet.inter lhs_primitives rhs_primitives) then
      Lints_errors.as_invalid_type
        pos
        (Tast_env.print_ty env lhs_ty)
        (Tast_env.print_ty env rhs_ty)
  | _ -> ()

let handler =
  object
    inherit Tast_visitor.handler_base as super

    method! at_expr env e =
      match e with
      | (_, p, As ((lhs_ty, _, _), hint, _isnull)) ->
        let hint_ty = Tast_env.hint_to_ty env hint in
        let (env, hint_ty) =
          Tast_env.localize_no_subst env ~ignore_errors:true hint_ty
        in
        refinable_check p env lhs_ty hint_ty
      | _ -> super#at_expr env e
  end

(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

module Reason  = Typing_reason
module TUtils  = Typing_utils
module Env     = Typing_env
module Inst    = Decl_instantiate
module TDef    = Typing_tdef
module SubType = Typing_subtype
module Phase   = Typing_phase
module MakeType = Typing_make_type

(*****************************************************************************)
(* Exporting. *)
(*****************************************************************************)

(* Tries to add constraint that ty_sub is subtype of ty_super in envs *)
let sub_type p ur env ty_sub ty_super =
  Typing_log.(log_with_level env "sub" 1 (fun () ->
    log_types p env
    [Log_head ("Typing_ops.sub_type",
       [Log_type ("ty_sub", ty_sub);
        Log_type ("ty_super", ty_super)])]));
  let env = { env with Env.pos = p; Env.outer_pos = p; Env.outer_reason = ur } in
  Errors.try_add_err p (Reason.string_of_ureason ur)
    (fun () -> SubType.sub_type env ty_sub ty_super)
    (fun () -> env)

let coerce_type ?sub_fn:(sub=sub_type) p ur env ty_have ty_expect =
  Typing_coercion.coerce_type ~sub_fn:sub p ur env ty_have ty_expect

let can_coerce env ty_have ty_expect =
  Typing_coercion.can_coerce env ty_have ty_expect

let sub_type_decl p ur env ty_sub ty_super =
  let env, ty_super = Phase.localize_with_self env ty_super in
  let env, ty_sub = Phase.localize_with_self env ty_sub in
  ignore (sub_type p ur env ty_sub ty_super)

(* Ensure that types are equivalent i.e. subtypes of each other *)
let unify_decl p ur env ty1 ty2 =
  let env, ty1 = Phase.localize_with_self env ty1 in
  let env, ty2 = Phase.localize_with_self env ty2 in
  ignore (sub_type p ur env ty2 ty1);
  ignore (sub_type p ur env ty1 ty2)

module LeastUpperBound = struct
  open Typing_defs
  open Nast

  let exact_least_upper_bound e1 e2 =
  match e1, e2 with
  | Exact, Exact -> Exact
  | _, _ -> Nonexact

  let prim_least_up_bound tprim1 tprim2 =
    match tprim1, tprim2 with
    | Tint, Tstring | Tstring, Tint -> Some Tarraykey
    | Tint, Tfloat | Tfloat, Tint -> Some Tnum
    | _ , _ -> None

  let rec type_visitor ~f ~default ty1 ty2 =
  let array_kind ak1 ak2 =
      match ak1, ak2 with
      | AKmap (ty1, ty2), AKmap (ty3, ty4) ->
        let ty1_ = type_visitor ~f ~default ty1 ty3 in
        let ty2_ = type_visitor ~f ~default ty2 ty4 in
        Some (AKmap (ty1_, ty2_))
      | AKdarray (ty1, ty2), AKdarray (ty3, ty4) ->
        let ty1_ = type_visitor ~f ~default ty1 ty3 in
        let ty2_ = type_visitor ~f ~default ty2 ty4 in
        Some (AKdarray (ty1_, ty2_))
      | AKvarray ty1, AKvarray ty2 ->
        let ty = type_visitor ~f ~default ty1 ty2 in
        Some (AKvarray ty)
      | AKvec ty1, AKvec ty2 ->
        let ty = type_visitor ~f ~default ty1 ty2 in
        Some (AKvec ty)
      | _ -> None
    in
    let (r1, ty_1), (_, ty_2) = (ty1, ty2) in
    match ty_1, ty_2 with
      | Ttuple tyl1, Ttuple tyl2 ->
        begin try let tyl = List.map2_exn ~f:(type_visitor ~f ~default) tyl1 tyl2 in
          r1, Ttuple tyl
          with _ -> default
        end
      | Tclass ((p, id1), e1, tyl1), Tclass((_, id2), e2, tyl2) ->
        if id1 = id2 then
          begin try let tyl = List.map2_exn ~f:(type_visitor ~f ~default) tyl1 tyl2 in
            r1, Tclass ((p, id1), exact_least_upper_bound e1 e2, tyl)
            with _ -> default
          end
        else
          default
      | Tarraykind ak1, Tarraykind ak2 ->
        begin match array_kind ak1 ak2 with
        | None -> default
        | Some ak -> r1, Tarraykind ak
        end
      | Tprim tprim1, Tprim tprim2 ->
        begin match prim_least_up_bound tprim1 tprim2 with
        | None -> f ty1 ty2
        | Some ty -> r1, Tprim ty
        end
      | Toption ty1, Toption ty2 ->
        let ty = type_visitor ~f ~default ty1 ty2 in r1, Toption ty
      | Toption ty1 , ty_2 | ty_2, Toption ty1 ->
        let ty = type_visitor ~f ~default ty1 (r1, ty_2) in r1, Toption ty
      | _ -> f ty1 ty2

  (* @TODO expand this match to refine more types*)
  let pairwise_least_upper_bound env ~default ty1 ty2 =
    if SubType.is_sub_type env ty1 ty2 then ty2
    else if SubType.is_sub_type env ty2 ty1 then ty1
    else default

  let rec full env types =
    match types with
    | [] -> None
    | [t] -> Some t
    | (r, _ as ty1) :: ty2 :: ts ->
      let default = MakeType.mixed r in
      let ty =
        type_visitor
          ~f:(pairwise_least_upper_bound env ~default)
          ~default ty1 ty2
      in
      full env (ty :: ts)

  let rec compute types =
    match types with
    | [] -> None
    | [t] ->  Some t
    | (tenv, p, k, (r, _ as ty1)) :: (_, _, _, ty2) :: ts  ->
      let default = MakeType.mixed r in
      let ty =
        type_visitor
          ~f:(pairwise_least_upper_bound tenv ~default)
          ~default ty1 ty2
      in
      compute ((tenv, p, k, ty) :: ts)

  end

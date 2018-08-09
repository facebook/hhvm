(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

module Reason  = Typing_reason
module TUtils  = Typing_utils
module Env     = Typing_env
module Inst    = Decl_instantiate
module Unify   = Typing_unify
module TDef    = Typing_tdef
module SubType = Typing_subtype
module Phase   = Typing_phase

(*****************************************************************************)
(* Exporting. *)
(*****************************************************************************)

(* Tries to add constraint that ty_sub is subtype of ty_super in envs *)
let sub_type p ur env ty_sub ty_super =
  Typing_log.log_types 2 p env
    [Typing_log.Log_sub ("Typing_ops.sub_type",
       [Typing_log.Log_type ("ty_sub", ty_sub);
        Typing_log.Log_type ("ty_super", ty_super)])];

  let env = { env with Env.pos = p; Env.outer_pos = p; Env.outer_reason = ur } in
  Errors.try_add_err p (Reason.string_of_ureason ur)
    (fun () -> SubType.sub_type env ty_sub ty_super)
    (fun () -> env)

(*
 * These are the main coercion functions.
 *
 * There are only a few ways coercion (~>) happens currently:
 * 1. t1 <: t2 |- t1 ~> t2
 *    (you can coerce t1 to any of its supertypes)
 * 2. |- t ~> dynamic
 *    (you can coerce t to dynamic)
 * 3. t1 ~> t2, t2 ~> t3 |- t1 ~> t3
 *    (coercion is transitive)
 * 4. t1 ~> t2 |- Awaitable<t1> ~> Awaitable<t2>
 *    (you can coerce in Awaitable)
 *
 * This boils down to running the normal sub_type procedure whenever possible,
 * and catching the remaining cases. The normal sub_type procedure is important
 * for resolving/inferring various types (e.g. array's union types), as well as
 * useful to the user for error messages. In the cases where we do not want to
 * sub_type, it suffices to do nothing.
 *
 * Supertypes of dynamic *can* occur, for instance as array's union types,
 * generics, and ?dynamic, so it is important to catch them.
 *
 * Roughly, coercion should be used over subtyping in places where a particular
 * type that could be dynamic is required, like parameters and returns.
 *)

(* coercion start *)

(* checks coercion that isn't just subtyping *)
let rec can_coerce ?seen:(seen=[]) env ty_have ty_expect =
  let env, ety_expect = Env.expand_type env ty_expect in
  let env, ety_have = Env.expand_type env ty_have in
  match ety_have, ety_expect with

  (* everything coerces to dynamic *)
  | _, (_, Tdynamic) -> Some env

  (* the Awaitable case *)
  | (_, Tclass ((_, cls1), [ty1])), (_, Tclass ((_, cls2), [ty2]))
    when cls1 = SN.Classes.cAwaitable && cls1 = cls2 ->
    can_coerce env ty1 ty2

  (* The two kinds of "unions" we have are options and unresolveds, the
   * relevant rule being that t1 ~> t2 |- t1 ~> t2|t3, derived from transitivity.
   * However, unresolveds can never be a hard expected type since they expand,
   * (and they can't even be written right now) so we can ignore them as coercion
   * targets. We will also go further here and assume that t1 ~> ?t2 iff
   * t1 ~> t2, though this needn't actually be the case in the future *)
  | _, (_, Toption ty) -> can_coerce env ty_have ty
  | _, (_, Tunresolved _) -> None

  (* We can also find coercion targets through generics, wherein all that
   * matters is that the lower bound is somewhere a valid coercion target *)
  | _, (_, Tabstract (AKgeneric name, _)) ->
    if List.mem name seen (* this will break cycles *)
    then None
    else
    let seen = name::seen in
    let lower_bounds = Env.get_lower_bounds env name in
    let coerce_check ty = ty |> can_coerce ~seen:seen env ty_have |> Option.first_some in
    Typing_set.fold coerce_check lower_bounds None

  (* cannot coerce outside of subtyping *)
  | _ -> None

let coerce_type ?sub_fn:(sub=sub_type) p ur env ty_have ty_expect =
  match can_coerce env ty_have ty_expect with
  | Some e -> e
  | None -> sub p ur env ty_have ty_expect

(* coercion end *)

let unify p ur env ty1 ty2 =
  let env = { env with Env.pos = p; Env.outer_pos = p; Env.outer_reason = ur } in
  Errors.try_add_err p (Reason.string_of_ureason ur)
    (fun () -> Unify.unify env ty1 ty2)
    (fun () -> env, (Reason.Rwitness p, Tany))

let union p ur env ty1 ty2 =
  let env = { env with Env.pos = p; Env.outer_pos = p; Env.outer_reason = ur } in
  Errors.try_add_err p (Reason.string_of_ureason ur)
    (fun () -> Unify.union env ty1 ty2)
    (fun () -> env, (Reason.Rwitness p, Tany))

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
        begin try let tyl = List.map2 (type_visitor ~f ~default) tyl1 tyl2 in
          r1, Ttuple tyl
          with _ -> default
        end
      | Tclass ((p, id1), tyl1), Tclass((_, id2), tyl2) ->
        if id1 = id2 then
          begin try let tyl = List.map2 (type_visitor ~f ~default) tyl1 tyl2 in
            r1, Tclass ((p, id1), tyl)
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
      let default = (r, TUtils.desugar_mixed r) in
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
      let default = (r, TUtils.desugar_mixed r) in
      let ty =
        type_visitor
          ~f:(pairwise_least_upper_bound tenv ~default)
          ~default ty1 ty2
      in
      compute ((tenv, p, k, ty) :: ts)

  end

(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs

module Reason  = Typing_reason
module TUtils  = Typing_utils
module Env     = Typing_env
module Inst    = Decl_instantiate
module Unify   = Typing_unify
module TDef    = Typing_tdef
module SubType = Typing_subtype
module Phase   = Typing_phase

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

(* checks coercion that isn't just subtyping *)
let rec can_coerce ?seen:(seen=[]) env ty_have ty_expect =
  let env, ety_expect = Env.expand_type env ty_expect in
  let env, ety_have = Env.expand_type env ty_have in
  match ety_have, ety_expect with

  (* everything coerces to dynamic *)
  | _, (_, Tdynamic) -> Some env

  (* the Awaitable case *)
  | (_, Tclass ((_, cls1), _, [ty1])), (_, Tclass ((_, cls2), _, [ty2]))
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
    if List.mem ~equal:(=) seen name (* this will break cycles *)
    then None
    else
    let seen = name::seen in
    let lower_bounds = Env.get_lower_bounds env name in
    let coerce_check ty = ty |> can_coerce ~seen:seen env ty_have |> Option.first_some in
    Typing_set.fold coerce_check lower_bounds None

  (* cannot coerce outside of subtyping *)
  | _ -> None

(* this is to break circular dependency on typing_ops *)
let sub_type _ _ env ty_have ty_expect = SubType.sub_type env ty_have ty_expect

(* does coercion, including subtyping *)
let coerce_type ?sub_fn:(sub=sub_type) p ur env ty_have ty_expect =
  match can_coerce env ty_have ty_expect with
  | Some e -> e
  | None -> sub p ur env ty_have ty_expect

(* does coercion if possible, returning Some env with resultant coercion constraints
 * otherwise suppresses errors from attempted coercion and returns None *)
let try_coerce ?sub_fn:(sub=sub_type) p ur env ty_have ty_expect =
  let f = !Errors.is_hh_fixme in
  Errors.is_hh_fixme := (fun _ _ -> false);
  let result =
    Errors.try_
      (fun () -> Some (coerce_type ~sub_fn:sub p ur env ty_have ty_expect))
      (fun _ -> None) in
  Errors.is_hh_fixme := f;
  result

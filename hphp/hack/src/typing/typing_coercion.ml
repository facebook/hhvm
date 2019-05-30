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

module Env     = Typing_env
module SubType = Typing_subtype

(*
* These are the main coercion functions.
*
* There are only a few coercion (~>) rules, documented in hphp/hack/doc/type_system/hack_typing.ott.
*
* 1. |- t ~> dynamic
*    (you can coerce t to dynamic)
* 2. t1 ~> t2 |- Awaitable<t1> ~> Awaitable<t2>
*    (you can coerce in Awaitable)
* 3. t1 ~> t2 |- t1 ~> ?t2
*    (you can coerce t1 to optional type if the inner type is a valid coercion target)
* 4. t1 <: t2 |- t1 ~> t2
*    (you can coerce t1 to any of its supertypes)
*
* This boils down to running the normal sub_type procedure whenever possible,
* and catching the remaining cases. The normal sub_type procedure is important
* for resolving/inferring various types (e.g. array's union types), as well as
* useful to the user for error messages. In the cases where we do not want to
* sub_type, it suffices to do nothing.
*
* Roughly, coercion should be used over subtyping in places where a particular
* type that could be dynamic is required, like parameters and returns.
*)

(* checks coercion that isn't just subtyping *)
let rec can_coerce env ty_have ty_expect =
  let env, ety_expect = Env.expand_type env ty_expect in
  let env, ety_have = Env.expand_type env ty_have in
  match ety_have, ety_expect with

  | _, (_, Tdynamic) -> Some env

  | (_, Tclass ((_, cls1), _, [ty1])), (_, Tclass ((_, cls2), _, [ty2]))
    when cls1 = SN.Classes.cAwaitable && cls1 = cls2 ->
    can_coerce env ty1 ty2

  | _, (_, Toption ty) -> can_coerce env ty_have ty

  | _ -> None

(* does coercion, including subtyping *)
let coerce_type p ?sub_fn:(sub=Typing_ops.sub_type) ur env ty_have ty_expect =
  match can_coerce env ty_have ty_expect with
  | Some e -> e
  | None -> sub p ur env ty_have ty_expect

(* does coercion if possible, returning Some env with resultant coercion constraints
 * otherwise suppresses errors from attempted coercion and returns None *)
let try_coerce ?sub_fn:(sub=Typing_ops.sub_type) p ur env ty_have ty_expect =
  let f = !Errors.is_hh_fixme in
  Errors.is_hh_fixme := (fun _ _ -> false);
  let result =
    Errors.try_
      (fun () -> Some (coerce_type ~sub_fn:sub p ur env ty_have ty_expect))
      (fun _ -> None) in
  Errors.is_hh_fixme := f;
  result

let () = Typing_utils.can_coerce_ref := can_coerce
let () = Typing_utils.coerce_type_ref := coerce_type

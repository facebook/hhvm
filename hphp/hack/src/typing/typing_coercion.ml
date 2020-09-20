(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module MakeType = Typing_make_type

(*
* These are the main coercion functions.
*
* There are only a few coercion (~>) rules, documented in hphp/hack/doc/type_system/hack_typing.ott.
*
* 1. |- t ~> dynamic
*    (you can coerce t to dynamic)
* 2. t is enforceable |- dynamic ~> t
*    (coercion from dynamic to enforceable types is permitted)
* 3. T1 ~> T and T2 ~> T |- T1|T2 ~> T
*    (coercion from a union is valid if coercion from each element is valid)
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

(* does coercion, including subtyping *)
let coerce_type_impl env ty_have ty_expect on_error =
  let complex_coercion =
    TypecheckerOptions.complex_coercion (Typing_env.get_tcopt env)
  in
  let (env, ety_expect) = Typing_env.expand_type env ty_expect.et_type in
  let (env, ety_have) = Typing_env.expand_type env ty_have in
  match (get_node ety_have, get_node ety_expect) with
  | (_, Tdynamic) -> env
  | (Tdynamic, _) when ty_expect.et_enforced -> env
  | _ when ty_expect.et_enforced ->
    Typing_utils.sub_type_with_dynamic_as_bottom
      env
      ty_have
      ty_expect.et_type
      on_error
  | _
    when ( ((not ty_expect.et_enforced) && env.Typing_env_types.pessimize)
         || Typing_utils.is_dynamic env ety_expect )
         && complex_coercion ->
    Typing_utils.sub_type_with_dynamic_as_bottom
      env
      ty_have
      ty_expect.et_type
      on_error
  | _ -> Typing_utils.sub_type env ty_have ty_expect.et_type on_error

let coerce_type p ur env ty_have ty_expect on_error =
  coerce_type_impl env ty_have ty_expect (fun ?code errl ->
      let errl = (p, Reason.string_of_ureason ur) :: errl in
      on_error ?code errl)

(* does coercion if possible, returning Some env with resultant coercion constraints
 * otherwise suppresses errors from attempted coercion and returns None *)
let try_coerce env ty_have ty_expect =
  let f = !Errors.is_hh_fixme in
  (Errors.is_hh_fixme := (fun _ _ -> false));
  let result =
    Errors.try_
      (fun () ->
        Some (coerce_type_impl env ty_have ty_expect Errors.unify_error))
      (fun _ -> None)
  in
  Errors.is_hh_fixme := f;
  result

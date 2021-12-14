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
open Typing_env_types

(*
* These are the main coercion functions. Roughly, coercion should be used over
* subtyping in places where a particular type that could be dynamic is
* required, like parameters and returns.
*
* The old dynamic uses the following ideas:
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
*
* The experimental sound dynamic (--enable-sound-dynamic-type) works
* differently because dynamic is part of the sub-typing relation.
*
*)

(* does coercion, including subtyping *)
let coerce_type_impl
    ~coerce_for_op
    env
    ty_have
    ty_expect
    (on_error : Typing_error.Reasons_callback.t) =
  let is_expected_enforced = equal_enforcement ty_expect.et_enforced Enforced in
  if TypecheckerOptions.enable_sound_dynamic env.genv.tcopt then
    if coerce_for_op && is_expected_enforced then
      (* If the coercion is for a built-in operation, then we want to allow it to apply to
         dynamic *)
      let (env, tunion) =
        Typing_union.union
          env
          ty_expect.et_type
          (Typing_make_type.dynamic Reason.Rnone)
      in
      let tunion =
        with_reason
          tunion
          (Reason.Rdynamic_coercion (get_reason ty_expect.et_type))
      in
      Typing_utils.sub_type_res ~coerce:None env ty_have tunion on_error
    else
      Typing_utils.sub_type_res
        ~coerce:None
        env
        ty_have
        ty_expect.et_type
        on_error
  else
    let complex_coercion =
      TypecheckerOptions.complex_coercion (Typing_env.get_tcopt env)
    in
    let (env, ety_expect) = Typing_env.expand_type env ty_expect.et_type in
    let (env, ety_have) = Typing_env.expand_type env ty_have in
    match (get_node ety_have, get_node ety_expect) with
    | (_, Tdynamic) -> Ok env
    | (Tdynamic, _) when is_expected_enforced -> Ok env
    | _ when is_expected_enforced ->
      Typing_utils.sub_type_with_dynamic_as_bottom_res
        env
        ty_have
        ty_expect.et_type
        on_error
    | _
      when (((not is_expected_enforced) && env.Typing_env_types.pessimize)
           || Typing_utils.is_dynamic env ety_expect)
           && complex_coercion ->
      Typing_utils.sub_type_with_dynamic_as_bottom_res
        env
        ty_have
        ty_expect.et_type
        on_error
    | _ -> Typing_utils.sub_type_res env ty_have ty_expect.et_type on_error

let result t ~on_ok ~on_err =
  match t with
  | Ok x -> on_ok x
  | Error y -> on_err y

let coerce_type
    ?(coerce_for_op = false)
    p
    ur
    env
    ty_have
    ty_expect
    (on_error : Typing_error.Callback.t) =
  result ~on_ok:Fn.id ~on_err:Fn.id
  @@ coerce_type_impl ~coerce_for_op env ty_have ty_expect
  @@ Typing_error.Reasons_callback.with_claim
       on_error
       ~claim:(p, Reason.string_of_ureason ur)

let coerce_type_res
    ?(coerce_for_op = false)
    p
    ur
    env
    ty_have
    ty_expect
    (on_error : Typing_error.Callback.t) =
  coerce_type_impl ~coerce_for_op env ty_have ty_expect
  @@ Typing_error.Reasons_callback.with_claim
       on_error
       ~claim:(p, Reason.string_of_ureason ur)

(* does coercion if possible, returning Some env with resultant coercion constraints
 * otherwise suppresses errors from attempted coercion and returns None *)
let try_coerce env ty_have ty_expect =
  let f = !Errors.is_hh_fixme in
  (Errors.is_hh_fixme := (fun _ _ -> false));
  let result =
    let pos =
      get_pos ty_have
      |> Typing_env.fill_in_pos_filename_if_in_current_decl env
      |> Option.value ~default:Pos.none
    in
    Errors.try_
      (fun () ->
        Some
          (result ~on_ok:Fn.id ~on_err:Fn.id
          @@ coerce_type_impl
               ~coerce_for_op:false
               env
               ty_have
               ty_expect
               (Typing_error.Reasons_callback.unify_error_at pos)))
      (fun _ -> None)
  in
  Errors.is_hh_fixme := f;
  result

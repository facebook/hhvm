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
*)

module Log = struct
  let should_log_coerce_type env =
    Typing_log.should_log env ~category:"typing" ~level:2

  let log_coerce_type env p ~ty_have ~ty_expect =
    Typing_log.log_function
      (Pos_or_decl.of_raw_pos p)
      ~function_name:"Typing_coercion.coerce_type"
      ~arguments:
        [
          ("ty_expect", Typing_print.debug env ty_expect);
          ("ty_have", Typing_print.debug env ty_have);
        ]
      ~result:(fun (_env, err_opt) ->
        Some
          (match err_opt with
          | None -> "ok"
          | Some _ -> "error"))
end

(* does coercion, including subtyping *)
let coerce_type_impl
    ~coerce_for_op
    ~is_dynamic_aware
    ~ignore_readonly
    env
    ty_have
    ty_expect
    ty_expect_enforced
    (on_error : Typing_error.Reasons_callback.t option) =
  let is_expected_enforced = equal_enforcement ty_expect_enforced Enforced in
  if coerce_for_op && is_expected_enforced then
    (* If the coercion is for a built-in operation, then we want to allow it to apply to
       dynamic *)
    let tunion =
      Typing_make_type.locl_like
        (Reason.dynamic_coercion (get_reason ty_expect))
        ty_expect
    in
    Typing_utils.sub_type
      ~is_dynamic_aware:false
      ~ignore_readonly
      env
      ty_have
      tunion
      on_error
  else
    let (env, ety_expect) = Typing_env.expand_type env ty_expect in
    match get_node ety_expect with
    | Tdynamic ->
      Typing_utils.sub_type
        ~is_dynamic_aware:true
        ~ignore_readonly
        env
        ty_have
        ty_expect
        on_error
    | _ ->
      Typing_utils.sub_type
        ~is_dynamic_aware
        ~ignore_readonly
        env
        ty_have
        ty_expect
        on_error

let coerce_type
    ~coerce_for_op
    ~is_dynamic_aware
    ~ignore_readonly
    p
    ur
    env
    ty_have
    ty_expect
    ty_expect_enforced
    (on_error : Typing_error.Callback.t) =
  coerce_type_impl
    ~coerce_for_op
    ~is_dynamic_aware
    ~ignore_readonly
    env
    ty_have
    ty_expect
    ty_expect_enforced
  @@ Some
       (Typing_error.Reasons_callback.with_claim
          on_error
          ~claim:(lazy (p, Reason.string_of_ureason ur)))

let coerce_type
    ?(coerce_for_op = false)
    ?(is_dynamic_aware = false)
    ?(ignore_readonly = false)
    p
    ur
    env
    ty_have
    ty_expect
    ty_expect_enforced
    (on_error : Typing_error.Callback.t) =
  if Log.should_log_coerce_type env then
    Log.log_coerce_type env p ~ty_have ~ty_expect @@ fun () ->
    coerce_type
      ~coerce_for_op
      ~is_dynamic_aware
      ~ignore_readonly
      p
      ur
      env
      ty_have
      ty_expect
      ty_expect_enforced
      on_error
  else
    coerce_type
      ~coerce_for_op
      ~is_dynamic_aware
      ~ignore_readonly
      p
      ur
      env
      ty_have
      ty_expect
      ty_expect_enforced
      on_error

let coerce_type_like_strip
    p ur env ty_have ty_expect (on_error : Typing_error.Callback.t) =
  let (env1, ty_err_opt) =
    coerce_type_impl
      ~coerce_for_op:false
      ~is_dynamic_aware:false
      ~ignore_readonly:false
      env
      ty_have
      ty_expect
      Unenforced
    @@ Some
         (Typing_error.Reasons_callback.with_claim
            on_error
            ~claim:(lazy (p, Reason.string_of_ureason ur)))
  in
  match ty_err_opt with
  | None -> (env1, None, false, ty_have)
  | Some _ ->
    let (env2, fresh_ty) = Typing_env.fresh_type env p in
    let (env2, like_ty) =
      Typing_utils.union
        env2
        fresh_ty
        (Typing_make_type.dynamic (get_reason fresh_ty))
    in
    let cb = Typing_error.Reasons_callback.unify_error_at p in
    let (env2, impossible_error) =
      Typing_utils.sub_type env2 fresh_ty ty_expect (Some cb)
    in
    (* Enforce the invariant - this call should never give us an error *)
    if Option.is_some impossible_error then
      Diagnostics.internal_error p "Subtype of fresh type variable";
    let (env2, ty_err_opt_like) =
      coerce_type
        p
        Reason.URnone
        env2
        ty_have
        like_ty
        Unenforced
        Typing_error.Callback.unify_error
    in
    (match ty_err_opt_like with
    | None -> (env2, None, true, fresh_ty)
    | Some _ ->
      Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env:env1);
      let ty_mismatch =
        Option.map ty_err_opt ~f:Fn.(const (ty_have, ty_expect))
      in
      (env1, ty_mismatch, false, ty_have))

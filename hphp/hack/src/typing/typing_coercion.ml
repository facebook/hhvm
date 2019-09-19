(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs

(* Typing_union.union normalizes a union out of two types, but it creates Toption for unions with
 * null. This function unwraps the null. TODO: remove in accordance with T45650596 *)
let force_null_union env r t =
  let null = Typing_make_type.null r in
  let (_, union) = Typing_union.union env null t in
  match union with
  | (r, Toption (_, Tunion tyl)) -> (r, Tunion (null :: tyl))
  | (r, Toption ty) -> (r, Tunion [null; ty])
  | _ -> union

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
let rec coerce_type_impl ~sub_type p ur env ty_have ty_expect on_error =
  Typing_log.(
    log_with_level env "sub" 1 (fun () ->
        log_types
          p
          env
          [
            Log_head
              ( "can_coerce",
                [
                  Log_type ("ty_have", ty_have);
                  Log_type ("ty_expect", ty_expect.et_type);
                ] );
          ]));
  let coercion_from_dynamic =
    TypecheckerOptions.coercion_from_dynamic (Typing_env.get_tcopt env)
  in
  let coercion_from_union =
    TypecheckerOptions.coercion_from_union (Typing_env.get_tcopt env)
  in
  let coerce_type_impl = coerce_type_impl ~sub_type p ur in
  let (env, ety_expect) = Typing_env.expand_type env ty_expect.et_type in
  let (env, ety_have) = Typing_env.expand_type env ty_have in
  match (ety_have, ety_expect) with
  | (_, (_, Tdynamic)) -> env
  | ((_, Tdynamic), _) when ty_expect.et_enforced && coercion_from_dynamic ->
    env
  | ((_, Tunion tyl), _) when coercion_from_union ->
    (* If coercion and subtyping fail for any of the elements of the union,
     * errors will be emitted *)
    List.fold tyl ~init:env ~f:(fun env ty ->
        coerce_type_impl env ty ty_expect on_error)
  | ((r, Toption t), _) when coercion_from_union ->
    let union : locl_ty = force_null_union env r t in
    coerce_type_impl env union ty_expect on_error
  | _ -> sub_type p ur env ty_have ty_expect.et_type on_error

(* The Errors.try_ allows us to report a union ty_have in an error
 * instead of just elements in the union *)
let coerce_type
    p ?sub_fn:(sub = Typing_ops.sub_type) ur env ty_have ty_expect on_error =
  Errors.try_
    (fun () ->
      coerce_type_impl p ~sub_type:sub ur env ty_have ty_expect on_error)
    (fun _ -> sub p ur env ty_have ty_expect.et_type on_error)

(* does coercion if possible, returning Some env with resultant coercion constraints
 * otherwise suppresses errors from attempted coercion and returns None *)
let try_coerce p ur env ty_have ty_expect =
  let f = !Errors.is_hh_fixme in
  (Errors.is_hh_fixme := (fun _ _ -> false));
  let result =
    Errors.try_
      (fun () ->
        Some
          (coerce_type_impl
             ~sub_type:Typing_ops.sub_type
             p
             ur
             env
             ty_have
             ty_expect
             Errors.unify_error))
      (fun _ -> None)
  in
  Errors.is_hh_fixme := f;
  result

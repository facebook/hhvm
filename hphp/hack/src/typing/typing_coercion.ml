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


let validator =
  let open Type_test_hint_check in
  object(_this)
    inherit [validation_state] Type_visitor.type_visitor as _super

    method! on_tthis acc r = update acc @@ Invalid (r, "the this type")
    method! on_tapply acc r (_, name) tyl =
      (* TODO(T45690473): follow type aliases in the `type` case and allow enforceable targets *)
      if Typing_env.is_typedef name || Typing_env.is_enum (Env.tast_env_as_typing_env acc.env) name
      then update acc @@ Invalid (r, "a typedef or enum")
      else visitor#check_class_targs acc r name tyl
    method! on_tgeneric acc r name = visitor#check_generic acc r name
    method! on_taccess acc r _ = update acc @@ Invalid (r, "a type const")
    method! on_tarray acc r ty1_opt ty2_opt =
      match ty1_opt, ty2_opt with
      | None, None -> acc
      | _ -> update acc @@ Invalid (r, "an array type")
    (* Optimization, don't visit type in dynamic ~> ~T case, fall back to subtyping *)
    method! on_tlike acc r _ = update acc @@ Invalid (r, "a like type")
    method! on_tprim acc r prim = visitor#on_tprim acc r prim
    method! on_tfun acc r _ = update acc @@ Invalid (r, "a function type")
    method! on_ttuple acc r _ = update acc @@ Invalid (r, "a tuple type")
    method! on_tshape acc r _ _ = update acc @@ Invalid (r, "a shape type")
  end

let supports_coercion_from_dynamic env (ty_expect_decl: decl ty) =
  let open Type_test_hint_check in
  let { validity; env; _ } = validator#on_type {
    env = Tast_env.typing_env_as_tast_env env;
    validity = Valid;
  } ty_expect_decl in
  match validity with
  | Valid -> Some (Tast_env.tast_env_as_typing_env env)
  | Invalid (_reason, _kind) -> None

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
* 4. t is enforceable |- dynamic ~> t
*    (coercion from dynamic to enforceable types is permitted)
* 5. t1 <: t2 |- t1 ~> t2
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
let rec can_coerce env ty_have ?ty_expect_decl ty_expect =
  let env, ety_expect = Env.expand_type env ty_expect in
  let env, ety_have = Env.expand_type env ty_have in
  match ety_have, ety_expect with

  | _, (_, Tdynamic) -> Some env

  | _, (_, Toption ty) -> can_coerce env ty_have ?ty_expect_decl ty

  (* dynamic ~> T if T is enforceable
   *
   * We only allow coercion if the coercion function was provided a decl ty
   * target for coercion. The reason is because locl tys lose information about
   * their origin, which can distinguish enforceable and unenforceable types. *)
  | (_, Tdynamic), _
    when (TypecheckerOptions.coercion_from_dynamic (Env.get_tcopt env)) ->
    let open Option in
    ty_expect_decl >>= (supports_coercion_from_dynamic env)

  | _ -> None

(* does coercion, including subtyping *)
let coerce_type p ?sub_fn:(sub=Typing_ops.sub_type) ur env ty_have ?ty_expect_decl ty_expect =
  match can_coerce env ty_have ?ty_expect_decl ty_expect with
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

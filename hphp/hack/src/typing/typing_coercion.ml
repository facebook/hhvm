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
  object(this)
    inherit [validation_state] Type_visitor.type_visitor as _super

    method! on_tthis acc r = update acc @@ Invalid (r, "the this type")
    method! on_tapply acc r (_, name) tyl =
      (* TODO(T45690473): follow type aliases in the `type` case and allow enforceable targets *)
      let not_class = Typing_env.is_typedef name ||
        Typing_env.is_enum (Env.tast_env_as_typing_env acc.env) name in
      if not_class then update acc @@ Invalid (r, "a typedef or enum") else

      match Env.get_class acc.env name with
      | Some tc ->
        let tparams = Cls.tparams tc in
        begin match tyl with
        | [] -> acc (* A class without type arguments is enforceable *)
        | targs ->
          let open List.Or_unequal_lengths in
          begin match List.fold2 ~init:acc targs tparams ~f:(fun acc targ tparam ->
            match targ with
            | _, Tdynamic (* We accept the inner type being dynamic regardless of reification *)
            | _, Tlike _ ->
              acc
            | _ ->
              match tparam.tp_reified with
              | Nast.Erased -> update acc @@ Invalid (r, "a type with an erased generic type argument")
              | Nast.SoftReified -> update acc @@ Invalid (r, "a type with a soft reified type argument")
              | Nast.Reified -> this#on_type acc targ
          ) with
          | Ok new_acc -> new_acc
          | Unequal_lengths -> acc (* arity error elsewhere *)
          end
        end
      | None -> acc
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
    ety_env = Typing_phase.env_with_self env;
    validity = Valid;
  } ty_expect_decl in
  match validity with
  | Valid -> Some (Tast_env.tast_env_as_typing_env env)
  | Invalid (_reason, _kind) -> None

(* Typing_union.union normalizes a union out of two types, but it creates Toption for unions with
 * null. This function unwraps the null. TODO: remove in accordance with T45650596 *)
let force_null_union env r t =
  let null = Typing_make_type.null r in
  let _, union = Typing_union.union env null t in
  match union with
  | r, Toption (_, Tunion tyl) ->
    r, Tunion (null::tyl)
  | r, Toption ty ->
    r, Tunion [null; ty]
  | _ -> union

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
* 5. t is enforceable, t1 <: t2, t2 ~> t |- t1 ~> t
*    (an abstract type coerces to a valid coercion target if one of its upper bounds coerces)
* 6. T1 ~> T and T2 ~> T |- T1|T2 ~> T
*    (coercion from a union is valid if coercion from each element is valid)
* 7. t1 <: t2 |- t1 ~> t2
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
let rec can_coerce env ?(ur=Reason.URnone) ty_have ?ty_expect_decl ty_expect =
  let env, ety_expect = Env.expand_type env ty_expect in
  let env, ety_have = Env.expand_type env ty_have in
  match ety_have, ety_expect with

  | _, (_, Tdynamic) -> Some env

  (* dynamic ~> T if T is enforceable
   *
   * We only allow coercion if the coercion function was provided a decl ty
   * target for coercion. The reason is because locl tys lose information about
   * their origin, which can distinguish enforceable and unenforceable types. *)
  | (_, Tdynamic), _
    when (TypecheckerOptions.coercion_from_dynamic (Env.get_tcopt env)) ->
    let open Option in
    ty_expect_decl >>= (supports_coercion_from_dynamic env)

  (* T1 ~> T2 if T1 is bounded above by T3, T2 is enforceable, and T3 ~> T2 *)
  | (_, Tabstract _), _
    when (TypecheckerOptions.coercion_from_dynamic (Env.get_tcopt env)) ->
    let open Option in
    ty_expect_decl >>= (supports_coercion_from_dynamic env) >>| (fun env ->
      let env, upper_bounds = Typing_utils.get_concrete_supertypes env ety_have in
      Typing_utils.run_on_intersection env ~f:(fun env upper_bound ->
        let p = Reason.to_pos (fst upper_bound) in
        let env = coerce_type p ur env upper_bound ?ty_expect_decl ty_expect in
        env, ()
      ) upper_bounds |> fst
    )

  (* T1|T2 ~> T if T1 ~> T and T2 ~> T *)
  | (_, Tunion tyl), _
    when (TypecheckerOptions.coercion_from_dynamic (Env.get_tcopt env)) ->
    (* If coercion and subtyping fail for any of the elements of the union,
     * errors will be emitted *)
    Some (List.fold tyl ~init:env ~f:(fun env ty ->
      let p = Reason.to_pos (fst ty) in
      coerce_type p ur env ty ?ty_expect_decl ty_expect
    ))

  (* TODO: remove in accordance with T45650596 *)
  | (r, Toption t), _
    when (TypecheckerOptions.coercion_from_dynamic (Env.get_tcopt env)) ->
    let union: locl ty = force_null_union env r t in
    can_coerce env ~ur union ?ty_expect_decl ty_expect

  (* TODO: remove in accordance with T45650596 *)
  | _, (_, Toption ty) -> can_coerce env ty_have ?ty_expect_decl ty

  | _ -> None

(* does coercion, including subtyping *)
and coerce_type p ?sub_fn:(sub=Typing_ops.sub_type) ur env ty_have ?ty_expect_decl ty_expect =
  match can_coerce env ~ur ty_have ?ty_expect_decl ty_expect with
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

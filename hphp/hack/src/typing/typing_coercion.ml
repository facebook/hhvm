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
module MakeType = Typing_make_type

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
* differently because it is not always safe to coerce a type to dynamic.
* The canonical example of something that it is not safe to coerce to dynamic,
* is a Box<T> with a property T, get, and set functions.
* The following leads to putting a string into a Box<int>
* $b = new Box<int>(1); // $b : Box<int>
* $d = $b;
* $d as dynamic;   // $d : dynamic & Box<int>
* $d->set("a");    // fine to call set on $d since it is dynamic
* $b->get();       // returns "a", but the type is int.
*
* Coercing from dynamic happens when the expected type will be enforced by
* hhvm, in this case, if whatever happens to be in the dynamic isn't of the
* expected type, execution will not continue. Hence,
* t1 ~> t2 if t2 is enforced and t1 <: dynamic, without having to
* coerce t1 to dynamic, e.g., t1 = dynamic or t1 = dynamic | C, but
* not t1 = C and C implements dynamic. The latter restriction ensures that
* static type errors are generated where nothing in the program explicitly
* coerces anything to dynamic.
*
* Coercion to dynamic happens in the sub-typing algorithm where it is allowed
* to conclude that C <: dynamic if C implements dynamic. Again, we don't want
* sub-typing to always be able to deduce this, only when we might be checking
* a sub-typing relationship where the super-type could contain a programmer
* supplied dynamic.
*
*)

(* does coercion, including subtyping *)
let coerce_type_impl
    env ty_have ty_expect (on_error : Errors.error_from_reasons_callback) =
  let is_expected_enforced = equal_enforcement ty_expect.et_enforced Enforced in
  if TypecheckerOptions.enable_sound_dynamic env.genv.tcopt then
    match ty_expect.et_enforced with
    | Enforced ->
      (* If the type is enforced, then we also allow dynamic.
         For example, if a function takes an int, then it's ok to pass it
         an int or something of type dynamic.
       *)
      let (env, tunion) =
        Typing_union.union env ty_expect.et_type (MakeType.dynamic Reason.Rnone)
      in
      let tunion =
        with_reason
          tunion
          (Reason.Rdynamic_coercion (get_reason ty_expect.et_type))
      in

      (* It's critical that we don't allow the sub-type check here to coerce to dynamic.
         If we did, then the following would type check:
         function f(string $s) : void {} ... f(1)
         We would check that int is a sub-type of string | dynamic, and it would
         be if we allow int <: dynamic in the check.

         We could imagine that we need the coercion to dynamic in order for
         ty_have to be a sub-type of ty_expect without the extra union with dynamic.
         This would lead to needing to do two different sub-type checks. However, this
         turns out to be unnecessary because of the restricted form that enforced types
         can have. An enforced type must be an atomic type, mixed, nothing,
         an optional enforced type, or a class type where all generics
         are reified and applied to enforced types.

         Thus the only way for dynamic to appear is if the type is
         C<dynamic, ...>, where C's type parameters are reified.
         Critically, reified parameters are invariant, and so we don't have C<int> <: C<dynamic> even
         with coercion to dynamic enabled.
         The computation of the enforcement takes care that top-level dynamic
         and ?dynamic are not considered Enforced.
         *)
      Typing_utils.sub_type_res ~coerce:None env ty_have tunion on_error
    | Unenforced
    | PartiallyEnforced ->
      Typing_utils.sub_type_res
        ~coerce:(Some Typing_logic.CoerceToDynamic)
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
      when ( ((not is_expected_enforced) && env.Typing_env_types.pessimize)
           || Typing_utils.is_dynamic env ety_expect )
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
    p ur env ty_have ty_expect (on_error : Errors.typing_error_callback) =
  result ~on_ok:Fn.id ~on_err:Fn.id
  @@ coerce_type_impl env ty_have ty_expect (fun ?code reasons ->
         on_error ?code (p, Reason.string_of_ureason ur) reasons)

let coerce_type_res
    p ur env ty_have ty_expect (on_error : Errors.typing_error_callback) =
  coerce_type_impl env ty_have ty_expect (fun ?code reasons ->
      on_error ?code (p, Reason.string_of_ureason ur) reasons)

(* does coercion if possible, returning Some env with resultant coercion constraints
 * otherwise suppresses errors from attempted coercion and returns None *)
let try_coerce env ty_have ty_expect =
  let f = !Errors.is_hh_fixme in
  (Errors.is_hh_fixme := (fun _ _ -> false));
  let result =
    Errors.try_
      (fun () ->
        Some
          ( result ~on_ok:Fn.id ~on_err:Fn.id
          @@ coerce_type_impl
               env
               ty_have
               ty_expect
               (Errors.unify_error_at Pos.none) ))
      (fun _ -> None)
  in
  Errors.is_hh_fixme := f;
  result

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
* expected type execution will not continue. Hence,
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
    env ty_have ty_expect (on_error : Errors.typing_error_callback) =
  if TypecheckerOptions.enable_sound_dynamic env.genv.tcopt then
    if
      ty_expect.et_enforced
      && Typing_utils.is_sub_type_for_coercion env ty_have ty_expect.et_type
    then
      env
    else
      Typing_utils.sub_type
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

let coerce_type
    p ur env ty_have ty_expect (on_error : Errors.typing_error_callback) =
  coerce_type_impl env ty_have ty_expect (fun ?code claim reasons ->
      on_error ?code (p, Reason.string_of_ureason ur) (claim :: reasons))

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

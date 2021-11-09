(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
open Typing_helpers
module Reason = Typing_reason
module MakeType = Typing_make_type
module Phase = Typing_phase
module TUtils = Typing_utils
module Partial = Partial_provider
module Env = Typing_env

(* This function is used to determine the type of an argument.
 * When we want to type-check the body of a function, we need to
 * introduce the type of the arguments of the function in the environment
 * Let's take an example, we want to check the code of foo:
 *
 * function foo(int $x): int {
 *   // CALL TO make_param_type on (int $x)
 *   // Now we know that the type of $x is int
 *
 *   return $x; // in the environment $x is an int, the code is correct
 * }
 *
 * When we localize, we want to resolve to "static" or "$this" depending on
 * the context. Even though we are passing in CIstatic, resolve_with_class_id
 * is smart enough to know what to do. Why do this? Consider the following
 *
 * abstract class C {
 *   abstract const type T;
 *
 *   private this::T $val;
 *
 *   final public function __construct(this::T $x) {
 *     $this->val = $x;
 *   }
 *
 *   public static function create(this::T $x): this {
 *     return new static($x);
 *   }
 * }
 *
 * class D extends C { const type T = int; }
 *
 * In __construct() we want to be able to assign $x to $this->val. The type of
 * $this->val will expand to '$this::T', so we need $x to also be '$this::T'.
 * We can do this soundly because when we construct a new class such as,
 * 'new D(0)' we can determine the late static bound type (D) and resolve
 * 'this::T' to 'D::T' which is int.
 *
 * A similar line of reasoning is applied for the static method create.
 *)
let make_param_local_ty env decl_hint param =
  let r = Reason.Rwitness param.param_pos in
  let (env, ty) =
    match decl_hint with
    | None -> (env, mk (r, TUtils.tany env))
    | Some ty ->
      let { et_type = ty; et_enforced } =
        Typing_enforceability.compute_enforced_and_pessimize_ty
          ~explicitly_untrusted:param.param_is_variadic
          env
          ty
      in
      (* If a parameter hint has the form ~t, where t is enforced,
       * then we know that the parameter has type t after enforcement *)
      let ty =
        match (get_node ty, et_enforced) with
        | (Tlike ty, Enforced) -> ty
        | _ -> ty
      in
      Phase.localize_no_subst env ~ignore_errors:false ty
  in
  let ty =
    match get_node ty with
    | t when param.param_is_variadic ->
      (* when checking the body of a function with a variadic
       * argument, "f(C ...$args)", $args is a varray<C> *)
      let r = Reason.Rvar_param param.param_pos in
      let arr_values = mk (r, t) in
      MakeType.varray r arr_values
    | _ -> ty
  in
  (env, ty)

let enforce_param_not_disposable env param ty =
  if has_accept_disposable_attribute param then
    ()
  else
    let p = param.param_pos in
    match Typing_disposable.is_disposable_type env ty with
    | Some class_name ->
      Errors.invalid_disposable_hint p (Utils.strip_ns class_name)
    | None -> ()

(* In strict mode, we force you to give a type declaration on a parameter *)
(* But the type checker is nice: it makes a suggestion :-) *)
let check_param_has_hint env param ty =
  let mode = Env.get_mode env in
  match hint_of_type_hint param.param_type_hint with
  | None when param.param_is_variadic && Partial.should_check_error mode 4033 ->
    Errors.expecting_type_hint_variadic param.param_pos
  | None when Partial.should_check_error mode 4032 ->
    Errors.expecting_type_hint param.param_pos
  | Some _ when Partial.should_check_error mode 4010 ->
    (* We do not permit hints to implement IDisposable or IAsyncDisposable *)
    enforce_param_not_disposable env param ty
  | _ -> ()

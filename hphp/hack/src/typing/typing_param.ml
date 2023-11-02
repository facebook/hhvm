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
open Common
open Typing_defs
open Typing_helpers
module Reason = Typing_reason
module MakeType = Typing_make_type
module Phase = Typing_phase
module Env = Typing_env

let enforce_param_not_disposable env param ty =
  if has_accept_disposable_attribute param then
    None
  else
    Option.map
      (Typing_disposable.is_disposable_type env ty)
      ~f:(fun class_name ->
        Typing_error.Primary.Invalid_disposable_hint
          { pos = param.param_pos; class_name = Utils.strip_ns class_name })

let check_param_has_hint env param =
  let prim_err_opt =
    if Option.is_none (hint_of_type_hint param.param_type_hint) then
      Some
        (if param.param_is_variadic then
          Typing_error.Primary.Expecting_type_hint_variadic param.param_pos
        else
          Typing_error.Primary.Expecting_type_hint param.param_pos)
    else
      None
  in
  Option.iter prim_err_opt ~f:(fun err ->
      Typing_error_utils.add_typing_error ~env @@ Typing_error.primary err)

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
let make_param_local_ty ~dynamic_mode ~no_auto_likes env decl_hint param =
  (* Don't check (again) for existence of hint in dynamic mode *)
  if not dynamic_mode then check_param_has_hint env param;
  match decl_hint with
  | None -> (env, None)
  | Some hint ->
    let ((env, ty_err_opt), ty) =
      let { et_type = ty; et_enforced } =
        Typing_enforceability.compute_enforced_and_pessimize_ty
          ~this_class:(Env.get_self_class env)
          ~explicitly_untrusted:param.param_is_variadic
          env
          hint
      in
      (match et_enforced with
      | Unenforced ->
        Typing_log.log_pessimise_param
          env
          ~is_promoted_property:(Option.is_some param.param_visibility)
          param.param_pos
          param.param_callconv
          param.param_name
      | Enforced -> ());
      (* If a non-inout parameter hint has the form ~t, where t is enforced,
       * then we know that the parameter has type t after enforcement.
       *)
      let ty =
        match (get_node ty, et_enforced, param.param_callconv) with
        | (Tlike ty, Enforced, Ast_defs.Pnormal) -> ty
        | _ -> ty
      in
      let ty =
        if TypecheckerOptions.everything_sdt Typing_env_types.(env.genv.tcopt)
        then
          let ty =
            (* For implicit pessimisation, wrap supportdyn around parameters with function types. *)
            match get_node ty with
            | Tfun _ ->
              Typing_utils.make_supportdyn_decl_type
                (get_pos ty)
                (get_reason ty)
                ty
            | _ -> ty
          in
          (* For implicit pessimisation, without <<__NoAutoLikes>>, wrap like around non-enforced inout parameters *)
          match (et_enforced, param.param_callconv) with
          | (Unenforced, Ast_defs.Pinout _) when not no_auto_likes ->
            MakeType.like (Reason.Rpessimised_inout (get_pos ty)) ty
          | _ -> ty
        else
          ty
      in
      Phase.localize_no_subst env ~ignore_errors:false ty
    in
    Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
    let ty =
      match get_node ty with
      | t when param.param_is_variadic ->
        (* when checking the body of a function with a variadic
         * argument, "f(C ...$args)", $args is a vec<C> *)
        let r = Reason.Rvar_param param.param_pos in
        let arr_values = mk (r, t) in
        MakeType.vec r arr_values
      | _ -> ty
    in
    (* We do not permit hints to implement IDisposable or IAsyncDisposable *)
    let prim_err_opt = enforce_param_not_disposable env param ty in
    Option.iter prim_err_opt ~f:(fun err ->
        Typing_error_utils.add_typing_error ~env @@ Typing_error.primary err);
    (env, Some ty)

let make_param_local_tys ~dynamic_mode ~no_auto_likes env decl_tys params =
  List.zip_exn params decl_tys
  |> List.map_env env ~f:(fun env (param, hint) ->
         let ty =
           if dynamic_mode then
             let dyn_ty =
               MakeType.dynamic
                 (Reason.Rsupport_dynamic_type
                    (Pos_or_decl.of_raw_pos param.param_pos))
             in
             match hint with
             | Some ty
               when Typing_enforceability.is_enforceable
                      ~this_class:(Env.get_self_class env)
                      env
                      ty ->
               Some
                 (MakeType.intersection
                    (Reason.Rsupport_dynamic_type Pos_or_decl.none)
                    [ty; dyn_ty])
             | _ -> Some dyn_ty
           else
             hint
         in
         make_param_local_ty ~dynamic_mode ~no_auto_likes env ty param)

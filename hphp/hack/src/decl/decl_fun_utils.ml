(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Nast
open Typing_defs

let conditionally_reactive_attribute_to_hint env { ua_params = l; _ } =
  match l with
  (* convert class const expression to non-generic type hint *)
  | [p, Class_const ((_, CI cls), (_, name))]
    when name = SN.Members.mClass ->
      (* set Extends dependency for between class that contains
         method and condition type *)
      Decl_env.add_extends_dependency env (snd cls);
      Decl_hint.hint env (p, Happly (cls, []))
  | _ ->
    (* error for invalid argument list was already reported during the
       naming step, do nothing *)
    Reason.none, Tany

let condition_type_from_attributes env user_attributes =
  Attributes.find SN.UserAttributes.uaOnlyRxIfImpl user_attributes
  |> Option.map ~f:(conditionally_reactive_attribute_to_hint env)

let fun_reactivity_opt env user_attributes =
  let has attr = Attributes.mem attr user_attributes in
  let module UA = SN.UserAttributes in

  let rx_condition = condition_type_from_attributes env user_attributes in

  if has UA.uaReactive then Some (Reactive rx_condition)
  else if has UA.uaShallowReactive then Some (Shallow rx_condition)
  else if has UA.uaLocalReactive then Some (Local rx_condition)
  else if has UA.uaNonRx then Some Nonreactive
  else None

let fun_reactivity env user_attributes =
  fun_reactivity_opt env user_attributes
  |> Option.value ~default:Nonreactive

let has_accept_disposable_attribute user_attributes =
  Attributes.mem SN.UserAttributes.uaAcceptDisposable user_attributes

let has_return_disposable_attribute user_attributes =
  Attributes.mem SN.UserAttributes.uaReturnDisposable user_attributes

let fun_returns_mutable user_attributes =
  Attributes.mem SN.UserAttributes.uaMutableReturn user_attributes

let fun_returns_void_to_rx user_attributes =
  Attributes.mem SN.UserAttributes.uaReturnsVoidToRx user_attributes

let get_param_mutability user_attributes =
  if Attributes.mem SN.UserAttributes.uaOwnedMutable user_attributes
  then Some Param_owned_mutable
  else if Attributes.mem SN.UserAttributes.uaMutable user_attributes
  then Some Param_borrowed_mutable
  else if Attributes.mem SN.UserAttributes.uaMaybeMutable user_attributes
  then Some Param_maybe_mutable
  else None

let make_param_ty env param =
  let ty = match param.param_hint with
    | None ->
      let r = Reason.Rwitness param.param_pos in
      (r, Tany)
      (* if the code is strict, use the type-hint *)
    | Some x ->
      Decl_hint.hint env x
  in
  let ty = match ty with
    | _, t when param.param_is_variadic ->
      (* When checking a call f($a, $b) to a function f(C ...$args),
       * both $a and $b must be of type C *)
      Reason.Rvar_param param.param_pos, t
    | x -> x
  in
  let module UA = SN.UserAttributes in
  let has_at_most_rx_as_func =
    Attributes.mem UA.uaAtMostRxAsFunc param.param_user_attributes
  in
  let ty =
    if has_at_most_rx_as_func then make_function_type_rxvar ty
    else ty in
  let mode = get_param_mode param.param_is_reference param.param_callconv in
  let rx_annotation =
    if has_at_most_rx_as_func then Some Param_rx_var
    else
      Attributes.find UA.uaOnlyRxIfImpl param.param_user_attributes
      |> Option.map ~f:(fun v -> Param_rx_if_impl (conditionally_reactive_attribute_to_hint env v))
    in
  {
    fp_pos  = param.param_pos;
    fp_name = Some param.param_name;
    fp_type = ty;
    fp_kind = mode;
    fp_mutability = get_param_mutability param.param_user_attributes;
    fp_accept_disposable =
      has_accept_disposable_attribute param.param_user_attributes;
    fp_rx_annotation = rx_annotation;
  }

let ret_from_fun_kind pos kind =
  let ty_any = (Reason.Rwitness pos, Tany) in
  match kind with
    | Ast.FGenerator ->
      let r = Reason.Rret_fun_kind (pos, kind) in
      r, Tapply ((pos, SN.Classes.cGenerator), [ty_any ; ty_any ; ty_any])
    | Ast.FAsyncGenerator ->
      let r = Reason.Rret_fun_kind (pos, kind) in
      r, Tapply ((pos, SN.Classes.cAsyncGenerator), [ty_any ; ty_any ; ty_any])
    | Ast.FAsync ->
      let r = Reason.Rret_fun_kind (pos, kind) in
      r, Tapply ((pos, SN.Classes.cAwaitable), [ty_any])
    | Ast.FSync
    | Ast.FCoroutine -> ty_any

let type_param env (t: Nast.tparam) =
{
  Typing_defs.tp_variance = t.tp_variance;
  tp_name = t.tp_name;
  tp_constraints = List.map t.tp_constraints (fun (ck, h) -> (ck, Decl_hint.hint env h));
  tp_reified = t.tp_reified;
  tp_user_attributes = t.tp_user_attributes;
}

let where_constraint env (ty1, ck, ty2) =
  (Decl_hint.hint env ty1, ck, Decl_hint.hint env ty2)

(* Functions building the types for the parameters of a function *)
(* It's not completely trivial because of optional arguments  *)

let minimum_arity paraml =
  (* We're looking for the minimum number of arguments that must be specified
  in a call to this method. Variadic "..." parameters need not be specified,
  parameters with default values need not be specified, so this method counts
  non-default-value, non-variadic parameters. *)
  let f param = (not param.param_is_variadic) && param.param_expr = None in
  List.count paraml f

let check_params env paraml =
  (* We wish to give an error on the first non-default parameter
  after a default parameter. That is:
  function foo(int $x, ?int $y = null, int $z)
  is an error on $z. *)
  (* TODO: This check doesn't need to be done at type checking time; it is
  entirely syntactic. When we switch over to the FFP, remove this code. *)
  let rec loop seen_default paraml =
    match paraml with
    | [] -> ()
    | param :: rl ->
        if param.param_is_variadic then
          () (* Assume that a variadic parameter is the last one we need
            to check. We've already given a parse error if the variadic
            parameter is not last. *)
        else if seen_default && param.param_expr = None then
          Errors.previous_default param.param_pos
          (* We've seen at least one required parameter, and there's an
          optional parameter after it.  Given an error, and then stop looking
          for more errors in this parameter list. *)
        else
          loop (param.param_expr <> None) rl
  in
  (* PHP allows non-default valued parameters after default valued parameters. *)
  if (env.Decl_env.mode <> FileInfo.Mphp) then
    loop false paraml

let make_params env paraml =
  List.map paraml ~f:(make_param_ty env)

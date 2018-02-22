(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Typing_defs

module Phase   = Typing_phase
module Env     = Typing_env
module SubType = Typing_subtype

let check_call env receiver_type pos reason ft =
  let localize decl_ty =
    let ety_env = Phase.env_with_self env in
    snd @@ Phase.localize ~ety_env env decl_ty in
  let type_to_str ty =
    (* strip expression dependent types to make error message clearer *)
    let rec unwrap = function
    | _, Tabstract (AKdependent (`static, []), Some ty) -> unwrap ty
    | _, Tabstract (AKdependent (`this, []), Some ty) -> unwrap ty
    | ty -> ty in
    Typing_print.full env (unwrap ty) in
  let get_reactivity_condition_type r =
    match r with
    | Nonreactive -> None
    | Local o | Shallow o | Reactive o -> Option.map o ~f:localize in
  (* localize reactivity condition types *)
  let caller_t = get_reactivity_condition_type @@ Env.env_reactivity env in
  let callee_t = get_reactivity_condition_type @@ ft.ft_reactive in
  (* check if callee has any conditition on it *)
  let callee_condition_matches =
    begin match callee_t, caller_t, receiver_type with
    | Some callee_ty, Some caller_ty, _
      when ty_equal caller_ty callee_ty -> true
    | Some callee_ty, _, Some receiver_ty
      when SubType.is_sub_type env receiver_ty callee_ty -> true
    | _ -> false
    end in
  (* if lambda is trying to call function that is not
     strictly reactive - mark lambda as non-reactive *)
  begin match ft.ft_reactive, Env.env_reactivity env with
  (* calling reactive function - no need to reset lambda reactivity *)
  | Reactive None, _ -> ()
  (* calling conditionally reactive function is ok if current function is
     conditionally reactive and condition type is the same *)
  | Reactive _, _ when callee_condition_matches -> ()
  (* for all other cases mark lambda as non-reactive *)
  | _ -> Env.not_lambda_reactive ()
  end;
  (* Strictly reactive functions can only call stricly reactive functions *)
  (* Shallow reactive functions can call strictly/shallow/local reactive functions *)
  (* Local reactive functions can call anything *)
  begin match Env.env_reactivity env, ft.ft_reactive with
  (* anything can be called from non-reactive functions *)
  | Nonreactive, _ -> ()
  (* error if reactive function is calling into something non-strictly-reactive.
    conditional reactivity is also handled here since:
     - when condition type on caller and callee match - call will be shallow
       or local which is not allowed
     - when condition type does not match - call will be non-reactive which is
       again not allowed *)
  | Reactive _, (Shallow _ | Local _ | Nonreactive) ->
    Errors.nonreactive_function_call pos (Reason.to_pos reason)
  (* error if shallow reactive function (conditional or unconditional)
    calls into nonreactive *)
  | Shallow _, Nonreactive ->
    Errors.nonreactive_call_from_shallow pos (Reason.to_pos reason)
  (* conditionally reactive call is allowed only:
    - if it is performed from conditionally reactive method and
      type specified as a condition on method being called matches
      condition type on caller
    - static type of method call receiver is a subtype of condition
      type on method being called *)
  | Reactive _, Reactive (Some _)
  | Shallow _, (Reactive (Some _) | Shallow (Some _))
    when not callee_condition_matches ->
    let condition_type_str =
      Option.value_map ~default:"" callee_t ~f:type_to_str in
    let receiver_type_str =
      Option.value_map receiver_type ~default:"" ~f:type_to_str in
    Errors.invalid_conditionally_reactive_call (Reason.to_pos reason)
      condition_type_str
      receiver_type_str;
  | _ -> ()
  end

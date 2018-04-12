(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

module Phase   = Typing_phase
module Env     = Typing_env
module SubType = Typing_subtype

let type_to_str: type a. Env.env -> a ty -> string = fun env ty ->
  (* strip expression dependent types to make error message clearer *)
  let rec unwrap: type a. a ty -> a ty = function
  | _, Tabstract (AKdependent (`static, []), Some ty) -> unwrap ty
  | _, Tabstract (AKdependent (`this, []), Some ty) -> unwrap ty
  | ty -> ty in
  Typing_print.full env (unwrap ty)

let check_call env receiver_type pos reason ft =
  (* call is allowed if reactivity of callee is a subtype of reactivity of
     enlosing environment *)
  let allow_call =
    let receiver_type = Option.map receiver_type (fun t -> LoclTy t) in
    SubType.subtype_reactivity
      ~extra_info: SubType.({ empty_extra_info with class_ty = receiver_type })
      ~is_call_site:true
      env
      ft.ft_reactive
      (Env.env_reactivity env) in
  let allow_call =
    if not allow_call && Env.is_checking_lambda () then begin
      (* if we are inferring reactivity of lambda - now we know it is non-reactive *)
      Env.not_lambda_reactive ();
      true
    end
    else allow_call in
  (* call is not allowed, report error *)
  if not allow_call then begin
    begin match Env.env_reactivity env, ft.ft_reactive with
    | Reactive _, (Shallow _ | Local _ | Nonreactive) ->
      Errors.nonreactive_function_call pos (Reason.to_pos reason)
    | Shallow _, Nonreactive ->
      Errors.nonreactive_call_from_shallow pos (Reason.to_pos reason)
    | Reactive _, Reactive (Some t)
    | Shallow _, (Reactive (Some t) | Shallow (Some t) | Local (Some t)) ->
      let condition_type_str = type_to_str env t in
      let receiver_type_str =
        Option.value_map receiver_type ~default:"" ~f:(type_to_str env) in
      Errors.invalid_conditionally_reactive_call pos (Reason.to_pos reason)
        condition_type_str
        receiver_type_str;
    | _ -> ()
    end
  end

let rec get_name = function
(* name *)
| _, Nast.Lvar (_, id) -> Local_id.to_string id
(* name = initializer *)
| _, Nast.Binop (_, lhs, _) -> get_name lhs
| _ -> "_"

let disallow_static_or_global_in_reactive_context ~is_static env el =
  Env.error_if_reactive_context env @@ begin fun () ->
    (Core_list.hd el) |> Option.iter ~f:(fun n ->
      let p = fst n in
      let name = get_name n in
      if is_static then Errors.static_in_reactive_context p name
      else Errors.global_in_reactive_context p name)
  end

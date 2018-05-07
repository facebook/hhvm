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

let rec strip_maybe_reactive t =
  match t with
  | r, Tfun ({ ft_reactive = MaybeReactive reactive; _ } as ft) ->
    r, Tfun { ft with ft_reactive = reactive }
  | r, Toption ty -> r, Toption (strip_maybe_reactive ty)
  | t -> t
let check_call env receiver_type pos reason ft arg_types =
  let callee_reactivity =
    (* if function we are about to call is maybe reactive with reactivity flavor R
      - check its arguments: call to maybe reactive function is treated as reactive
       if arguments that correspond to parameters marked with <<__OnlyRxIfRxFunc>> are functions
       with reactivity <: R *)
    let callee_has_rx_condition_parameters =
      Core_list.exists ft.ft_params ~f:begin function
      | { fp_rx_condition; _ } -> Option.is_some fp_rx_condition
      end in
    if not callee_has_rx_condition_parameters then ft.ft_reactive
    else
    let is_reactive =
      match Core_list.zip ft.ft_params arg_types with
      | None -> false
      | Some l -> Core_list.for_all l ~f:(function
        | { fp_rx_condition = None; _ }, _ ->  true
        | { fp_rx_condition = Some Param_rx_if_impl ty; _ }, arg_ty ->
          let _, cond_ty = Phase.localize (Phase.env_with_self env) env ty in
          SubType.is_sub_type env arg_ty cond_ty
        | { fp_rx_condition = Some Param_rxfunc; fp_type; _ }, arg_ty ->
          let param_type = strip_maybe_reactive fp_type in
          let arg_ty = strip_maybe_reactive arg_ty in
          SubType.is_sub_type env arg_ty param_type) in
    if is_reactive then ft.ft_reactive else Nonreactive in
  (* call is allowed if reactivity of callee is a subtype of reactivity of
     enlosing environment *)
  let allow_call =
    let receiver_type = Option.map receiver_type (fun t -> LoclTy t) in
    SubType.subtype_reactivity
      ~extra_info: SubType.({ empty_extra_info with class_ty = receiver_type })
      ~is_call_site:true
      env
      callee_reactivity
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
    begin match Env.env_reactivity env, callee_reactivity with
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

let disallow_onlyrx_if_rxfunc_on_non_functions env param param_ty =
  let module UA = Naming_special_names.UserAttributes in
  if Attributes.mem UA.uaOnlyRxIfRxFunc param.Nast.param_user_attributes
  then begin
    if param.Nast.param_hint = None
    then Errors.missing_annotation_for_onlyrx_if_rxfunc_parameter param.Nast.param_pos
    else match Typing_utils.non_null env param_ty with
    (* if parameter has <<__OnlyRxIfRxFunc>> annotation then:
       - parameter should be typed as function *)
    | _, (_, Tfun _) -> ()
    | _ ->
      Errors.invalid_type_for_onlyrx_if_rxfunc_parameter
        (Reason.to_pos (fst param_ty))
        (Typing_print.full env param_ty)
  end

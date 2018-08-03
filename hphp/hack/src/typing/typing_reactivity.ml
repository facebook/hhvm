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
module TU      = Typing_utils
module CT      = SubType.ConditionTypes

type method_call_info = {
  receiver_type: locl ty;
  receiver_is_self: bool;
  is_static: bool;
  method_name: string;
}

let make_call_info ~receiver_is_self ~is_static receiver_type method_name =
  { receiver_type; receiver_is_self; is_static; method_name; }

let type_to_str: type a. Env.env -> a ty -> string = fun env ty ->
  (* strip expression dependent types to make error message clearer *)
  let rec unwrap: type a. a ty -> a ty = function
  | _, Tabstract (AKdependent (`static, []), Some ty) -> unwrap ty
  | _, Tabstract (AKdependent (`this, []), Some ty) -> unwrap ty
  | ty -> ty in
  Typing_print.full env (unwrap ty)

let localize env ty =
  let _, t = Phase.localize (Phase.env_with_self env) env ty in
  t

let rec condition_type_from_reactivity r =
  match r with
  | Reactive (Some t) | Shallow (Some t) | Local (Some t) -> Some t
  | MaybeReactive r -> condition_type_from_reactivity r
  | _ -> None

(* Obtains condition type associated with ty
   - for types in [$this; self; static] it tries to extract condition type from the
   current reactivity context
   - for parameter types with OnlyRxIfImpl annotation it will get associated condition
   type that was specified as parameter in attribute
   - for all other cases return None *)
let get_associated_condition_type env ~is_self ty =
  match ty with
  | _, Tabstract (AKgeneric n, _) ->
    Env.get_condition_type env n
  | _, Tabstract (AKdependent (`static, []), _)
  | _, Tabstract (AKdependent (`this, []), _) ->
    condition_type_from_reactivity (Env.env_reactivity env)
  | _ when is_self ->
    condition_type_from_reactivity (Env.env_reactivity env)
  | _ -> None

(* removes condition type from given reactivity flavor *)
let rec strip_conditional_reactivity r =
  match r with
  | Reactive (Some _) -> Reactive None
  | Shallow (Some _) -> Shallow None
  | Local (Some _) -> Local None
  | MaybeReactive r -> MaybeReactive (strip_conditional_reactivity r)
  | r -> r

(* checks if condition type associated with ty matches the condition
   specified by cond_ty *)
let condition_type_matches ~is_self env ty cond_ty =
  get_associated_condition_type ~is_self env ty
  |> Option.value_map ~default:false ~f:(fun arg_cond_ty ->
    let arg_cond_ty = CT.localize_condition_type env arg_cond_ty in
    SubType.is_sub_type env arg_cond_ty cond_ty)

(* checks if ty matches the criteria specified by argument of __OnlyRxIfImpl *)
let check_only_rx_if_impl env ~is_receiver ~is_self pos reason ty cond_ty =
  (* __OnlyRxIfImpl condition is true if either
    - ty is a subtype of condition type
    - type has linked condition type which is a subtype of condition type *)
  let cond_ty = CT.localize_condition_type env cond_ty in
  let ok =
    SubType.is_sub_type env ty cond_ty ||
    condition_type_matches ~is_self env ty cond_ty in
  if not ok
  then begin
    let condition_type_str = type_to_str env cond_ty in
    let arg_type_str = type_to_str env ty in
    let arg_pos = Reason.to_pos (fst ty) in
    Errors.invalid_argument_type_for_condition_in_rx ~is_receiver
      pos (Reason.to_pos reason) arg_pos condition_type_str arg_type_str
  end;
  ok

(* checks if function type (arg_ty) mets the __OnlyRxIfRxFunc criteria
   specified by param_ty *)
let check_only_rx_if_rx_func env pos reason caller_r arg_ty param_ty =
  (*  strip options and expand type aliases *)
  let env, arg_ty = TU.non_null env arg_ty in
  let env, arg_ty = Env.expand_type env arg_ty in
  let env, param_ty = TU.non_null env param_ty in
  let env, param_ty = Env.expand_type env param_ty in
  let error arg_r param_r =
    let arg_str = TU.reactivity_to_string env arg_r in
    let param_str = TU.reactivity_to_string env param_r in
    Errors.invalid_function_type_for_condition_in_rx
      pos (Reason.to_pos reason) (Reason.to_pos (fst arg_ty))
      arg_str param_str in
  (* __OnlyRxIfRxFunc condition for some argument type matches if
   - argument type is definitely reactive or argument type is
     maybe reactive and current reactivity context is maybe reactive
   - reactivity of function type is a subtype of required reactivity for the argument  *)
  match arg_ty, param_ty with
  | (_, Tfun { ft_reactive = arg_r; _ }),
    (_, Tfun { ft_reactive = MaybeReactive param_r; _ }) ->
    begin match arg_r, caller_r with
    (* argument is non-reactive - check failed *)
    | Nonreactive, _ ->
      error arg_r param_r;
      false
    (* argument is maybe reactive and caller context is maybe reactive
       - might be ok *)
    | MaybeReactive arg_r, MaybeReactive _ ->
      if SubType.subtype_reactivity ~is_call_site:true env arg_r param_r then true
      else begin
        error arg_r param_r;
        false
      end
    | MaybeReactive _, _ ->
      error arg_r param_r;
      false
    | arg_r, _ ->
      if SubType.subtype_reactivity ~is_call_site:true env arg_r param_r then true
      else begin
        error arg_r param_r;
        false
      end
    end
  | _ -> false

let bind o ~f = Option.bind o f

let try_get_method_from_condition_type env receiver_info =
  receiver_info
  |> bind ~f:begin
      fun { receiver_type; receiver_is_self = is_self; is_static; method_name; _ } ->
        get_associated_condition_type ~is_self env receiver_type
        |> Option.map ~f:(fun t -> t, is_static, method_name)
     end
  |> bind ~f:begin
      fun (t, is_static, method_name) ->
        CT.try_get_method_from_condition_type env t is_static method_name
      end
  |> bind ~f:begin
      function
      | { ce_type = lazy (_, Typing_defs.Tfun f); _  } -> Some f
      | _ -> None
      end

let try_get_reactivity_from_condition_type env receiver_info =
  try_get_method_from_condition_type env receiver_info
  |> Option.map ~f:begin
    function
    | { ft_reactive = Nonreactive; _ } -> Nonreactive
    | { ft_reactive = MaybeReactive _ as r; _ } -> r
    | { ft_reactive = r; _ } -> MaybeReactive r
    end

let check_reactivity_matches env pos reason caller_reactivity callee_reactivity =
  let callee_reactivity = strip_conditional_reactivity callee_reactivity in
  let ok = SubType.subtype_reactivity ~is_call_site:true env callee_reactivity caller_reactivity in
  if ok then true
  else begin
    begin match caller_reactivity, callee_reactivity with
    | (MaybeReactive (Reactive _) | Reactive _),
      (MaybeReactive (Shallow _ | Local _ | Nonreactive) | (Shallow _ | Local _ | Nonreactive)) ->
      Errors.nonreactive_function_call pos (Reason.to_pos reason)
    | (MaybeReactive (Shallow _) | Shallow _), Nonreactive ->
      Errors.nonreactive_call_from_shallow pos (Reason.to_pos reason)
    | _ ->
      Errors.callsite_reactivity_mismatch
        pos (Reason.to_pos reason)
        (TU.reactivity_to_string env callee_reactivity)
        (TU.reactivity_to_string env caller_reactivity)
      end;
    false
  end

let check_call env method_info pos reason ft arg_types =
  (* do nothing if unsafe_rx is set *)
  if TypecheckerOptions.unsafe_rx (Env.get_options env) then ()
  else
  match Env.env_reactivity env with
  (* non reactive and locally reactive functions can call pretty much anything
     - do nothing *)
  | Nonreactive | Local _ -> ()
  | _ ->
  (* check steps:
     1. ensure that conditions for all parameters (including receiver) are met
     2. check that reactivity of the callee matches reactivity of the caller with
       stripped condition types (they were checked on step 1) *)
  let caller_reactivity =
    match Env.env_reactivity env with
    | Reactive (Some _)
    | MaybeReactive (Reactive (Some _)) -> MaybeReactive (Reactive None)
    | Shallow (Some _)
    | MaybeReactive (Shallow (Some _)) -> MaybeReactive (Shallow None)
    | Local (Some _)
    | MaybeReactive (Local (Some _)) -> MaybeReactive (Local None)
    | r -> r in
  (* check that all conditions are met if we are calling something
     conditionally reactive *)
  let callee_is_conditionally_reactive =
    (* receiver is conditionally reactive *)
    Option.is_some (condition_type_from_reactivity ft.ft_reactive) ||
    (* one of arguments is conditionally reactive *)
    Core_list.exists ft.ft_params ~f:begin function
    | { fp_rx_condition; _ } -> Option.is_some fp_rx_condition
    end in
  let allow_call =
    if callee_is_conditionally_reactive then begin
      let allow_call =
        (* check that condition for receiver is met *)
        match condition_type_from_reactivity ft.ft_reactive, method_info with
        | Some cond_ty, Some { receiver_type; receiver_is_self = is_self; _ } ->
          check_only_rx_if_impl env
            ~is_receiver:true
            ~is_self
            pos reason
            receiver_type cond_ty
        | _ ->
          true in
      allow_call &&
      (* check that conditions for all arguments are met *)
      begin match Core_list.zip ft.ft_params arg_types with
      | None -> false
      | Some l ->
        Core_list.for_all l ~f:begin function
        | { fp_rx_condition = None; _ }, _ ->  true
        | { fp_rx_condition = Some Param_rx_if_impl ty; _ }, arg_ty ->
          check_only_rx_if_impl env ~is_receiver:false ~is_self:false pos reason arg_ty ty
        | { fp_rx_condition = Some Param_rxfunc; fp_type; _ }, arg_ty ->
          check_only_rx_if_rx_func env pos reason caller_reactivity arg_ty fp_type
        end
      end
    end
    else true in
  (* if call is not allowed - this means that that at least one of conditions
     was not met and since errors were already reported we can bail out. Otherwise
     we need to verify that reactivities for callee and caller are in agreement. *)
  if allow_call then begin
    (* pick the function we are trying to invoke *)
    let ok =
      Option.value_map (try_get_reactivity_from_condition_type env method_info)
        ~f:(check_reactivity_matches env pos reason caller_reactivity)
        ~default: false in
    if not ok then
      check_reactivity_matches env pos reason caller_reactivity ft.ft_reactive
      |> ignore
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

let rxTraversableType =
  Reason.none,
  Tclass ((Pos.none, Naming_special_names.Rx.cTraversable), [(Reason.Rnone, Tany)])

let check_foreach_collection env p t =
  (* do nothing if unsafe_rx is set *)
  if TypecheckerOptions.unsafe_rx (Env.get_options env) then ()
  else
  match Env.env_reactivity env with
  | Nonreactive | Local _ -> ()
  | _ ->
  let rec check t =
    let env, t = Env.expand_type env t in
    match t with
    | _, Tunresolved l -> Core_list.for_all l ~f:check
    | t ->
      (* collection type should be subtype or conditioned to Rx\Traversable *)
      if not (SubType.is_sub_type env t rxTraversableType ||
              condition_type_matches ~is_self:false env t rxTraversableType)
      then begin
        Errors.invalid_traversable_in_rx p;
        false
      end
      else true in
  ignore (check t)

let disallow_onlyrx_if_rxfunc_on_non_functions env param param_ty =
  let module UA = Naming_special_names.UserAttributes in
  if Attributes.mem UA.uaOnlyRxIfRxFunc param.Nast.param_user_attributes
  then begin
    if param.Nast.param_hint = None
    then Errors.missing_annotation_for_onlyrx_if_rxfunc_parameter param.Nast.param_pos
    else match TU.non_null env param_ty with
    (* if parameter has <<__OnlyRxIfRxFunc>> annotation then:
       - parameter should be typed as function *)
    | _, (_, Tfun _) -> ()
    | _ ->
      Errors.invalid_type_for_onlyrx_if_rxfunc_parameter
        (Reason.to_pos (fst param_ty))
        (Typing_print.full env param_ty)
  end

(* generate a name that uniquely identifies pair target_type * condition_type *)
let generate_fresh_name_for_target_of_condition_type env target_type condition_type  =
  match condition_type with
  | _, Tapply ((_, cond_name), []) ->
    (* only if condition type is a Tapply with no type parameters *)
    Some ((Typing_print.full env target_type) ^ "#" ^ cond_name)
  | _, Taccess _ ->
    Some ((Typing_print.full env target_type) ^ "#" ^ (Typing_print.full env condition_type))
  | _ -> None

let verify_void_return_to_rx ~is_expr_statement p env ft =
  if ft.ft_returns_void_to_rx && not is_expr_statement
  then Env.error_if_reactive_context env @@ begin fun () ->
    Errors.returns_void_to_rx_function_as_non_expression_statement p ft.ft_pos
  end

let try_substitute_type_with_condition env cond_ty ty =
  generate_fresh_name_for_target_of_condition_type env ty cond_ty
  |> Option.map ~f:begin fun fresh_type_argument_name ->
    let param_ty = Reason.none, Tabstract ((AKgeneric fresh_type_argument_name), None) in
    (* if generic type is already registered this means we already saw
       parameter with the same pair (declared type * condition type) so there
       is no need to add condition type to env again  *)
    if Env.is_generic_parameter env fresh_type_argument_name
    then env, param_ty
    else begin
      (* constraint type argument to hint *)
      let env = Env.add_upper_bound_global env fresh_type_argument_name ty in
      (* link type argument name to condition type *)
      let env = Env.set_condition_type env fresh_type_argument_name cond_ty in
      env, param_ty
    end
  end

let get_adjusted_return_type env receiver_info ret_ty =
  match try_get_method_from_condition_type env receiver_info with
  | None -> env, ret_ty
  | Some cond_fty ->
    try_substitute_type_with_condition env cond_fty.ft_ret ret_ty
    |> Option.value ~default:(env, ret_ty)

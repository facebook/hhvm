(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs

module Phase   = Typing_phase
module Env     = Typing_env
module SubType = Typing_subtype
module TU      = Typing_utils
module CT      = SubType.ConditionTypes
module MakeType     = Typing_make_type

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
  | RxVar v -> Option.bind v condition_type_from_reactivity
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
  | RxVar v -> RxVar (Option.map v strip_conditional_reactivity)
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

let check_reactivity_matches env pos reason caller_reactivity (callee_reactivity, cause_pos) =
  let callee_reactivity = strip_conditional_reactivity callee_reactivity in
  let ok = SubType.subtype_reactivity ~is_call_site:true env callee_reactivity caller_reactivity in
  if ok then true
  else begin
    (* for better error reporting remove rxvar from caller reactivity *)
    let caller_reactivity =
      match caller_reactivity with
      | MaybeReactive (RxVar (Some r)) -> MaybeReactive r
      | RxVar (Some r) -> r
      | r -> r in
    begin match caller_reactivity, callee_reactivity with
    | (MaybeReactive (Reactive _) | Reactive _),
      (MaybeReactive (Shallow _ | Local _ | Nonreactive) | (Shallow _ | Local _ | Nonreactive)) ->
      Errors.nonreactive_function_call pos
        (Reason.to_pos reason)
        (TU.reactivity_to_string env callee_reactivity)
        cause_pos
    | (MaybeReactive (Shallow _) | Shallow _), Nonreactive ->
      Errors.nonreactive_call_from_shallow pos
        (Reason.to_pos reason)
        (TU.reactivity_to_string env callee_reactivity)
        cause_pos
    | _ ->
      Errors.callsite_reactivity_mismatch
        pos (Reason.to_pos reason)
        (TU.reactivity_to_string env callee_reactivity)
        cause_pos
        (TU.reactivity_to_string env caller_reactivity)
      end;
    false
  end

let get_effective_reactivity env r ft arg_types =
  let go ((res, _) as acc) (p, arg_ty) =
    if p.fp_rx_annotation = Some Param_rx_var
    then begin
      match arg_ty with
      | reason, Tfun { ft_reactive = r; _ } ->
        if SubType.subtype_reactivity env ~is_call_site:true r res
        then acc
        else r, Some (Reason.to_pos reason)
      | _ -> acc
    end
    else acc in
  match r with
  | RxVar (Some rx) | MaybeReactive (RxVar (Some rx)) | rx ->
    begin match List.zip ft.ft_params arg_types with
    | Some l -> List.fold ~init:(rx, None) ~f:go l
    | None -> r, None
    end

let check_call env method_info pos reason ft arg_types =
  (* do nothing if unsafe_rx is set *)
  if TypecheckerOptions.unsafe_rx (Env.get_tcopt env) then ()
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
    let rec go = function
    | Reactive (Some _)
    | MaybeReactive (Reactive (Some _)) -> MaybeReactive (Reactive None)
    | Shallow (Some _)
    | MaybeReactive (Shallow (Some _)) -> MaybeReactive (Shallow None)
    | Local (Some _)
    | MaybeReactive (Local (Some _)) -> MaybeReactive (Local None)
    | MaybeReactive (RxVar (Some v)) -> MaybeReactive (RxVar (Some (go v)))
    | r -> r in
    go (Env.env_reactivity env) in
  (* check that all conditions are met if we are calling something
     conditionally reactive *)
  let callee_is_conditionally_reactive =
    (* receiver is conditionally reactive *)
    Option.is_some (condition_type_from_reactivity ft.ft_reactive) ||
    (* one of arguments is conditionally reactive *)
    List.exists ft.ft_params ~f:begin function
    | { fp_rx_annotation = Some (Param_rx_if_impl _); _ } -> true
    | _ -> false
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
      begin match List.zip ft.ft_params arg_types with
      | None -> false
      | Some l ->
        List.for_all l ~f:begin function
        | { fp_rx_annotation = Some Param_rx_if_impl ty; _ }, arg_ty ->
          check_only_rx_if_impl env ~is_receiver:false ~is_self:false pos reason arg_ty ty
        (* | { fp_rx_condition = Some Param_rxfunc; fp_type; _ }, arg_ty ->
          check_only_rx_if_rx_func env pos reason caller_reactivity arg_ty fp_type *)
        | { fp_rx_annotation = _; _ }, _ ->  true
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
        ~f:(fun r -> check_reactivity_matches env pos reason caller_reactivity (r, None))
        ~default: false in
    if not ok then
      check_reactivity_matches env pos reason caller_reactivity
        (get_effective_reactivity env ft.ft_reactive ft arg_types)
      |> ignore
  end

let rxTraversableType =
  MakeType.class_type Reason.none Naming_special_names.Rx.cTraversable [(Reason.Rnone, Tany)]

let rxAsyncIteratorType =
  MakeType.class_type Reason.none Naming_special_names.Rx.cAsyncIterator [(Reason.Rnone, Tany)]

let check_foreach_collection env p t =
  (* do nothing if unsafe_rx is set *)
  if TypecheckerOptions.unsafe_rx (Env.get_tcopt env) then ()
  else
  match Env.env_reactivity env with
  | Nonreactive | Local _ -> ()
  | _ ->
  let rec check t =
    let env, t = Env.expand_type env t in
    match t with
    | _, Tunresolved l -> List.for_all l ~f:check
    | t ->
      (* collection type should be subtype or conditioned to Rx\Traversable *)
      if not (SubType.is_sub_type env t rxTraversableType ||
              SubType.is_sub_type env t rxAsyncIteratorType ||
              condition_type_matches ~is_self:false env t rxTraversableType ||
              condition_type_matches ~is_self:false env t rxAsyncIteratorType)
      then begin
        Errors.invalid_traversable_in_rx p;
        false
      end
      else true in
  ignore (check t)

let disallow_atmost_rx_as_rxfunc_on_non_functions env param param_ty =
  let module UA = Naming_special_names.UserAttributes in
  if Attributes.mem UA.uaAtMostRxAsFunc param.Nast.param_user_attributes
  then begin
    if param.Nast.param_hint = None
    then Errors.missing_annotation_for_atmost_rx_as_rxfunc_parameter param.Nast.param_pos
    else match TU.non_null env param.Nast.param_pos param_ty with
    (* if parameter has <<__AtMostRxAsFunc>> annotation then:
       - parameter should be typed as function *)
    | _, (_, Tfun _) -> ()
    | _ ->
      Errors.invalid_type_for_atmost_rx_as_rxfunc_parameter
        (Reason.to_pos (fst param_ty))
        (Typing_print.full env param_ty)
  end

(* generate a name that uniquely identifies pair target_type * condition_type *)
let generate_fresh_name_for_target_of_condition_type env target_type condition_type  =
  match condition_type with
  | _, Tapply ((_, cond_name), _) ->
    Some ((Typing_print.full env target_type) ^ "#" ^ cond_name)
  | _, Taccess _ ->
    Some ((Typing_print.full env target_type) ^ "#" ^ (Typing_print.full env condition_type))
  | _ -> None

let try_substitute_type_with_condition env cond_ty ty =
  generate_fresh_name_for_target_of_condition_type env ty cond_ty
  |> Option.map ~f:begin fun fresh_type_argument_name ->
    let param_ty =
      (Reason.Rwitness (Reason.to_pos (fst ty))),
      Tabstract ((AKgeneric fresh_type_argument_name), None) in
    (* if generic type is already registered this means we already saw
       parameter with the same pair (declared type * condition type) so there
       is no need to add condition type to env again  *)
    if Env.is_generic_parameter env fresh_type_argument_name
    then env, param_ty
    else begin
      let param_ty, ty =
        match ty with
        | _, Toption ty -> ((fst param_ty), Toption param_ty), ty
        | _ -> param_ty, ty in
      (* constraint type argument to hint *)
      let env = Env.add_upper_bound_global env fresh_type_argument_name ty in
      (* link type argument name to condition type *)
      let env = Env.set_condition_type env fresh_type_argument_name cond_ty in
      env, param_ty
    end
  end

(* for cases like
   <<__OnlyRxIfImpl(Rx::class)>>
   function f(): this::T {
     if (Rx_ENABLED {
       return get_rx();
     }
    else {
      return get_cached_rx(); // returns this::T
    }
   }
   return type of f will be represented as (<TFresh>#Rx) as this::T
   so we can use name of fresh parameter to track condition type. This in turn will
   lead to error in else branch of if statement because get_cached_rx will return this::T
   and it is not assignable to fresh type parameter. To handle this for returns we reduce
   return type to its upper bound if return type is TFresh and current context is non-reactive *)
let strip_condition_type_in_return env ty =
  if Env.env_reactivity env <> Nonreactive then ty
  else begin match ty with
  | _, Tabstract ((AKgeneric n), _)
    when Option.is_some (Env.get_condition_type env n) ->
    let upper_bounds = Env.get_upper_bounds env n in
    begin match Typing_set.elements upper_bounds with
    | [ty] -> ty
    | _ ->  ty
    end
  | _ -> ty
  end
let get_adjusted_return_type env receiver_info ret_ty =
  match try_get_method_from_condition_type env receiver_info with
  | None -> env, ret_ty
  | Some cond_fty ->
    try_substitute_type_with_condition env cond_fty.ft_ret ret_ty
    |> Option.value ~default:(env, ret_ty)

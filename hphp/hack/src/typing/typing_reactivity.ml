(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
open Typing_env_types
module Phase = Typing_phase
module Env = Typing_env
module SubType = Typing_subtype
module TU = Typing_utils
module CT = SubType.ConditionTypes
module MakeType = Typing_make_type
module TL = Typing_logic

type method_call_info = {
  receiver_type: locl_ty;
  receiver_is_self: bool;
  is_static: bool;
  method_name: string;
}

let make_call_info ~receiver_is_self ~is_static receiver_type method_name =
  { receiver_type; receiver_is_self; is_static; method_name }

let type_to_str env ty =
  (* strip expression dependent types to make error message clearer *)
  let rec unwrap ty =
    match get_node ty with
    | Tdependent (DTthis, ty) -> unwrap ty
    | _ -> ty
  in
  Typing_print.full env (unwrap ty)

let localize env ty =
  let (_, t) = Phase.localize (Phase.env_with_self env) env ty in
  t

let rec condition_type_from_reactivity r =
  match r with
  | Reactive (Some t)
  | Shallow (Some t)
  | Local (Some t) ->
    Some t
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
  let (env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tgeneric n -> Env.get_condition_type env n
  | Tdependent (DTthis, _) ->
    condition_type_from_reactivity (env_reactivity env)
  | _ when is_self -> condition_type_from_reactivity (env_reactivity env)
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
let assert_condition_type_matches ~is_self env ty cond_ty fail =
  match get_associated_condition_type ~is_self env ty with
  | None ->
    fail ();
    env
  | Some arg_cond_ty ->
    let arg_cond_ty = CT.localize_condition_type env arg_cond_ty in
    SubType.sub_type_or_fail env arg_cond_ty cond_ty fail

let condition_type_matches ~is_self env ty cond_ty =
  match get_associated_condition_type ~is_self env ty with
  | None -> false
  | Some arg_cond_ty ->
    let arg_cond_ty = CT.localize_condition_type env arg_cond_ty in
    SubType.is_sub_type env arg_cond_ty cond_ty

(* checks if ty matches the criteria specified by argument of __OnlyRxIfImpl *)
let check_only_rx_if_impl env ~is_receiver ~is_self pos reason ty cond_ty =
  (* __OnlyRxIfImpl condition is true if either
    - ty is a subtype of condition type
    - type has linked condition type which is a subtype of condition type *)
  let cond_ty = CT.localize_condition_type env cond_ty in
  let fail () =
    let condition_type_str = type_to_str env cond_ty in
    let arg_type_str = type_to_str env ty in
    let arg_pos = get_pos ty in
    Errors.invalid_argument_type_for_condition_in_rx
      ~is_receiver
      pos
      (Reason.to_pos reason)
      arg_pos
      condition_type_str
      arg_type_str
  in
  let rec check env ty =
    (* TODO: move caller to be TAST check  *)
    let (env, ty) = Env.expand_type env ty in
    match get_node ty with
    | Tintersection tyl ->
      fst (TU.run_on_intersection env tyl ~f:(fun env ty -> (check env ty, ())))
    | _ ->
      Errors.try_
        (fun () -> SubType.sub_type_or_fail env ty cond_ty fail)
        (fun _ -> assert_condition_type_matches ~is_self env ty cond_ty fail)
  in
  check env ty

let bind o ~f = Option.bind o f

let try_get_method_from_condition_type env receiver_info =
  receiver_info
  |> bind
       ~f:(fun {
                 receiver_type;
                 receiver_is_self = is_self;
                 is_static;
                 method_name;
                 _;
               }
               ->
         get_associated_condition_type ~is_self env receiver_type
         |> Option.map ~f:(fun t -> (t, is_static, method_name)))
  |> bind ~f:(fun (t, is_static, method_name) ->
         CT.try_get_method_from_condition_type env t is_static method_name)
  |> bind ~f:(function { ce_type = (lazy ty); _ } ->
         begin
           match get_node ty with
           | Tfun f -> Some f
           | _ -> None
         end)

let try_get_reactivity_from_condition_type env receiver_info =
  try_get_method_from_condition_type env receiver_info
  |> Option.map ~f:(function
         | { ft_reactive = Nonreactive; _ } -> Nonreactive
         | { ft_reactive = MaybeReactive _ as r; _ } -> r
         | { ft_reactive = r; _ } -> MaybeReactive r)

let check_reactivity_matches
    env pos reason caller_reactivity (callee_reactivity, cause_pos) =
  let callee_reactivity = strip_conditional_reactivity callee_reactivity in
  SubType.subtype_reactivity
    ~is_call_site:true
    env
    (Reason.to_pos reason)
    callee_reactivity
    pos
    caller_reactivity
    (fun ?code:_ _ ->
      (* for better error reporting remove rxvar from caller reactivity *)
      let caller_reactivity =
        match caller_reactivity with
        | MaybeReactive (RxVar (Some r)) -> MaybeReactive r
        | RxVar (Some r) -> r
        | r -> r
      in
      match (caller_reactivity, callee_reactivity) with
      | ( (MaybeReactive (Reactive _) | Reactive _),
          ( MaybeReactive (Shallow _ | Local _ | Nonreactive)
          | Shallow _ | Local _ | Nonreactive ) ) ->
        Errors.nonreactive_function_call
          pos
          (Reason.to_pos reason)
          (TU.reactivity_to_string env callee_reactivity)
          cause_pos
      | ((MaybeReactive (Shallow _) | Shallow _), Nonreactive) ->
        Errors.nonreactive_call_from_shallow
          pos
          (Reason.to_pos reason)
          (TU.reactivity_to_string env callee_reactivity)
          cause_pos
      | _ ->
        Errors.callsite_reactivity_mismatch
          pos
          (Reason.to_pos reason)
          (TU.reactivity_to_string env callee_reactivity)
          cause_pos
          (TU.reactivity_to_string env caller_reactivity))

let get_effective_reactivity env r ft arg_types =
  let go ((env, res, opt_pos) as acc) (p, arg_ty) =
    let (env, arg_ty) = Env.expand_type env arg_ty in
    match (p.fp_rx_annotation, deref arg_ty) with
    | (Some Param_rx_var, (reason, Tfun { ft_reactive = r; _ })) ->
      Errors.try_
        (fun () ->
          let _env =
            SubType.subtype_reactivity
              env
              ~is_call_site:true
              Pos.none
              r
              Pos.none
              res
              Errors.unify_error
          in
          (env, res, opt_pos))
        (fun _ -> (env, r, Some (Reason.to_pos reason)))
    | _ -> acc
  in
  match r with
  | RxVar (Some rx)
  | MaybeReactive (RxVar (Some rx))
  | rx ->
    begin
      match List.zip ft.ft_params arg_types with
      | Some l -> List.fold ~init:(env, rx, None) ~f:go l
      | None -> (env, r, None)
    end

let check_call env method_info pos reason ft arg_types =
  (* do nothing if unsafe_rx is set *)
  if TypecheckerOptions.unsafe_rx (Env.get_tcopt env) then
    env
  else
    match env_reactivity env with
    (* non reactive and locally reactive functions can call pretty much anything
     - do nothing *)
    | Nonreactive
    | Local _ ->
      env
    | _ ->
      (* check steps:
     1. ensure that conditions for all parameters (including receiver) are met
     2. check that reactivity of the callee matches reactivity of the caller with
       stripped condition types (they were checked on step 1) *)
      let caller_reactivity =
        let rec go = function
          | Reactive (Some _)
          | MaybeReactive (Reactive (Some _)) ->
            MaybeReactive (Reactive None)
          | Shallow (Some _)
          | MaybeReactive (Shallow (Some _)) ->
            MaybeReactive (Shallow None)
          | Local (Some _)
          | MaybeReactive (Local (Some _)) ->
            MaybeReactive (Local None)
          | MaybeReactive (RxVar (Some v)) ->
            MaybeReactive (RxVar (Some (go v)))
          | r -> r
        in
        go (env_reactivity env)
      in
      (* check that all conditions are met if we are calling something
     conditionally reactive *)
      let callee_is_conditionally_reactive =
        (* receiver is conditionally reactive *)
        Option.is_some (condition_type_from_reactivity ft.ft_reactive)
        || (* one of arguments is conditionally reactive *)
        List.exists ft.ft_params ~f:(function
            | { fp_rx_annotation = Some (Param_rx_if_impl _); _ } -> true
            | _ -> false)
      in
      let env =
        if callee_is_conditionally_reactive then
          (* check that condition for receiver is met *)
          let env =
            match
              (condition_type_from_reactivity ft.ft_reactive, method_info)
            with
            | ( Some cond_ty,
                Some { receiver_type; receiver_is_self = is_self; _ } ) ->
              check_only_rx_if_impl
                env
                ~is_receiver:true
                ~is_self
                pos
                reason
                receiver_type
                cond_ty
            | _ -> env
          in
          (* check that conditions for all arguments are met *)
          let rec check_params env ft_params arg_types =
            match (ft_params, arg_types) with
            | ([], _) -> env
            | ( { fp_rx_annotation = Some (Param_rx_if_impl ty); fp_type; _ }
                :: tl,
                arg_ty :: arg_tl ) ->
              let ty =
                if Typing_utils.is_option env fp_type.et_type then
                  MakeType.nullable_decl (get_reason ty) ty
                else
                  ty
              in
              (* check if argument type matches condition *)
              let env =
                check_only_rx_if_impl
                  env
                  ~is_receiver:false
                  ~is_self:false
                  pos
                  reason
                  arg_ty
                  ty
              in
              (* check the rest of arguments *)
              check_params env tl arg_tl
            | ( { fp_rx_annotation = Some (Param_rx_if_impl ty); fp_type; _ }
                :: tl,
                [] )
              when Typing_utils.is_option env fp_type.et_type ->
              (* if there are more parameters than actual arguments - assume that remaining parameters
            have default values (actual arity check is performed elsewhere).  *)
              let ty = MakeType.nullable_decl (get_reason ty) ty in
              (* Treat missing arguments as if null was provided explicitly *)
              let arg_ty = MakeType.null Reason.none in
              let env =
                check_only_rx_if_impl
                  env
                  ~is_receiver:false
                  ~is_self:false
                  pos
                  reason
                  arg_ty
                  ty
              in
              check_params env tl []
            | ({ fp_rx_annotation = Some (Param_rx_if_impl _); _ } :: _, []) ->
              (* Missing argument for non-nulalble RxIfImpl parameter - no reasonable defaults are expected.
            TODO: add check that type of parameter annotated with RxIfImpl is class or interface *)
              env
            (*false*)
            | (_ :: tl, _ :: arg_tl) -> check_params env tl arg_tl
            | (_ :: tl, []) -> check_params env tl []
          in
          check_params env ft.ft_params arg_types
        else
          env
      in
      (* if call is not allowed - this means that that at least one of conditions
     was not met and since errors were already reported we can bail out. Otherwise
     we need to verify that reactivities for callee and caller are in agreement. *)
      let env =
        (* pick the function we are trying to invoke *)
        match try_get_reactivity_from_condition_type env method_info with
        | None ->
          let (env, r, opt_pos) =
            get_effective_reactivity env ft.ft_reactive ft arg_types
          in
          check_reactivity_matches env pos reason caller_reactivity (r, opt_pos)
        | Some r ->
          check_reactivity_matches env pos reason caller_reactivity (r, None)
      in
      env

let disallow_atmost_rx_as_rxfunc_on_non_functions env param param_ty =
  let module UA = Naming_special_names.UserAttributes in
  if Naming_attributes.mem UA.uaAtMostRxAsFunc param.Aast.param_user_attributes
  then
    if Option.is_none (Aast.hint_of_type_hint param.Aast.param_type_hint) then
      Errors.missing_annotation_for_atmost_rx_as_rxfunc_parameter
        param.Aast.param_pos
    else
      let rec err_if_not_fun env ty =
        let (env, ty) = Env.expand_type env ty in
        match get_node ty with
        (* if parameter has <<__AtMostRxAsFunc>> annotation then:
           - parameter should be typed as function or a like function *)
        | Tfun _ -> ()
        | Tunion [ty; dty] when is_dynamic dty -> err_if_not_fun env ty
        | Tunion [dty; ty] when is_dynamic dty -> err_if_not_fun env ty
        | Toption ty -> err_if_not_fun env ty
        | _ ->
          Errors.invalid_type_for_atmost_rx_as_rxfunc_parameter
            (get_pos param_ty)
            (Typing_print.full env param_ty)
      in
      err_if_not_fun env param_ty

(* generate a name that uniquely identifies pair target_type * condition_type *)
let generate_fresh_name_for_target_of_condition_type
    env target_type condition_type =
  match get_node condition_type with
  | Tapply ((_, cond_name), _) ->
    Some (Typing_print.full env target_type ^ "#" ^ cond_name)
  | Taccess _ ->
    Some
      ( Typing_print.full env target_type
      ^ "#"
      ^ Typing_print.full_decl (Env.get_ctx env) condition_type )
  | _ -> None

let try_substitute_type_with_condition env cond_ty ty =
  generate_fresh_name_for_target_of_condition_type env ty cond_ty
  |> Option.map ~f:(fun fresh_type_argument_name ->
         let param_ty =
           MakeType.generic
             (Reason.Rwitness (get_pos ty))
             fresh_type_argument_name
         in
         (* if generic type is already registered this means we already saw
       parameter with the same pair (declared type * condition type) so there
       is no need to add condition type to env again  *)
         if Env.is_generic_parameter env fresh_type_argument_name then
           (env, param_ty)
         else
           let (env, ety) = Env.expand_type env ty in
           let (param_ty, ty) =
             match get_node ety with
             | Toption ty ->
               (MakeType.nullable_locl (get_reason param_ty) param_ty, ty)
             | _ -> (param_ty, ty)
           in
           (* constraint type argument to hint *)
           let env =
             Env.add_upper_bound_global env fresh_type_argument_name ty
           in
           (* link type argument name to condition type *)
           let env =
             Env.set_condition_type env fresh_type_argument_name cond_ty
           in
           (env, param_ty))

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
  if not (equal_reactivity (env_reactivity env) Nonreactive) then
    ty
  else
    let (env, ety) = Env.expand_type env ty in
    match get_node ety with
    | Tgeneric n when Option.is_some (Env.get_condition_type env n) ->
      let upper_bounds = Env.get_upper_bounds env n in
      begin
        match Typing_set.elements upper_bounds with
        | [ty] -> ty
        | _ -> ty
      end
    | _ -> ty

let get_adjusted_return_type env receiver_info ret_ty =
  match try_get_method_from_condition_type env receiver_info with
  | None -> (env, ret_ty)
  | Some cond_fty ->
    try_substitute_type_with_condition env cond_fty.ft_ret.et_type ret_ty
    |> Option.value ~default:(env, ret_ty)

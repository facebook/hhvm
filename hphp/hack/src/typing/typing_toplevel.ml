(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This module implements the typing.
 *
 * Given an Nast.program, it infers the type of all the local
 * variables, and checks that all the types are correct (aka
 * consistent) *)
open Hh_prelude
open Common
open Aast
open Typing_defs
open Typing_env_types
open Typing_helpers
open Utils
module Reason = Typing_reason
module Env = Typing_env
module EnvFromDef = Typing_env_from_def
module MakeType = Typing_make_type

let param_has_at_most_rx_as_func p =
  let module UA = SN.UserAttributes in
  Naming_attributes.mem UA.uaAtMostRxAsFunc p.param_user_attributes

let fun_reactivity env attrs params =
  let r = Decl_fun_utils.fun_reactivity env attrs in
  let module UA = Naming_special_names.UserAttributes in
  let r =
    (* if at least one of parameters has <<__AtMostRxAsFunc>> attribute -
      treat function reactivity as generic that is determined from the reactivity
      of arguments annotated with __AtMostRxAsFunc. Declared reactivity is used as a
      upper boundary of the reactivity function can have. *)
    if List.exists params ~f:param_has_at_most_rx_as_func then
      RxVar (Some r)
    else
      r
  in
  let r =
    (* if at least one of arguments have <<__OnlyRxIfImpl>> attribute -
      treat function reactivity as conditional that is determined at the callsite *)
    if
      List.exists params ~f:(fun { param_user_attributes = p; _ } ->
          Naming_attributes.mem UA.uaOnlyRxIfImpl p)
    then
      MaybeReactive r
    else
      r
  in
  r

(* The two following functions enable us to retrieve the function (or class)
  header from the shared mem. Note that they only return a non None value if
  global inference is on *)
let get_decl_function_header env function_id =
  let is_global_inference_on = TCO.global_inference (Env.get_tcopt env) in
  if is_global_inference_on then
    match Decl_provider.get_fun (Env.get_ctx env) function_id with
    | Some { fe_type; _ } ->
      begin
        match get_node fe_type with
        | Tfun fun_type -> Some fun_type
        | _ -> None
      end
    | _ -> None
  else
    None

and get_decl_method_header tcopt cls method_id ~is_static =
  let is_global_inference_on = TCO.global_inference tcopt in
  if is_global_inference_on then
    match Cls.get_any_method ~is_static cls method_id with
    | Some { ce_type = (lazy ty); _ } ->
      begin
        match get_node ty with
        | Tfun fun_type -> Some fun_type
        | _ -> None
      end
    | _ -> None
  else
    None

let enforce_param_not_disposable env param ty =
  if has_accept_disposable_attribute param then
    ()
  else
    let p = param.param_pos in
    match Typing_disposable.is_disposable_type env ty with
    | Some class_name -> Errors.invalid_disposable_hint p (strip_ns class_name)
    | None -> ()

(* In strict mode, we force you to give a type declaration on a parameter *)
(* But the type checker is nice: it makes a suggestion :-) *)
let check_param_has_hint env param ty is_code_error =
  let env =
    if is_code_error 4231 then
      Typing.attributes_check_def
        env
        SN.AttributeKinds.parameter
        param.param_user_attributes
    else
      env
  in
  match hint_of_type_hint param.param_type_hint with
  | None when param.param_is_variadic && is_code_error 4033 ->
    Errors.expecting_type_hint_variadic param.param_pos
  | None when is_code_error 4032 -> Errors.expecting_type_hint param.param_pos
  | Some _ when is_code_error 4010 ->
    (* We do not permit hints to implement IDisposable or IAsyncDisposable *)
    enforce_param_not_disposable env param ty
  | _ -> ()

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
  let ety_env = { (Phase.env_with_self env) with from_class = Some CIstatic } in
  let r = Reason.Rwitness param.param_pos in
  let (env, ty) =
    match decl_hint with
    | None -> (env, mk (r, TUtils.tany env))
    | Some ty ->
      let { et_type = ty; _ } =
        Typing_enforceability.compute_enforced_and_pessimize_ty
          ~explicitly_untrusted:param.param_is_variadic
          env
          ty
      in
      let condition_type =
        Decl_fun_utils.condition_type_from_attributes
          env.decl_env
          param.param_user_attributes
      in
      begin
        match condition_type with
        | Some condition_type ->
          let (env, ty) = Phase.localize ~ety_env env ty in
          begin
            match
              TR.try_substitute_type_with_condition env condition_type ty
            with
            | Some r -> r
            | None -> (env, ty)
          end
        | _
          when Naming_attributes.mem
                 SN.UserAttributes.uaAtMostRxAsFunc
                 param.param_user_attributes ->
          let (env, ty) = Phase.localize ~ety_env env ty in
          (* expand type to track aliased function types *)
          let (env, expanded_ty) = Env.expand_type env ty in
          let adjusted_ty = make_function_type_rxvar expanded_ty in
          ( env,
            if phys_equal adjusted_ty expanded_ty then
              ty
            else
              adjusted_ty )
        | _ -> Phase.localize ~ety_env env ty
      end
  in
  let ty =
    match get_node ty with
    | t when param.param_is_variadic ->
      (* when checking the body of a function with a variadic
       * argument, "f(C ...$args)", $args is a varray<C> *)
      let r = Reason.Rvar_param param.param_pos in
      let arr_values = mk (r, t) in
      mk (r, Tvarray arr_values)
    | _ -> ty
  in
  Typing_reactivity.disallow_atmost_rx_as_rxfunc_on_non_functions env param ty;
  (env, ty)

let get_callable_variadicity
    ?(is_function = false) ~partial_callback ~pos env variadicity_decl_ty =
  function
  | FVvariadicArg vparam ->
    let (env, ty) = make_param_local_ty env variadicity_decl_ty vparam in
    check_param_has_hint env vparam ty partial_callback;
    let (env, t_variadic) = Typing.bind_param env (ty, vparam) in
    (env, Aast.FVvariadicArg t_variadic)
  | FVellipsis p ->
    if is_function && Partial.should_check_error (Env.get_mode env) 4223 then
      Errors.ellipsis_strict_mode ~require:`Type_and_param_name pos;
    (env, Aast.FVellipsis p)
  | FVnonVariadic -> (env, Aast.FVnonVariadic)

let merge_hint_with_decl_hint env type_hint decl_ty =
  let contains_tvar decl_ty =
    match decl_ty with
    | None -> false
    | Some decl_ty -> TUtils.contains_tvar_decl decl_ty
  in
  if contains_tvar decl_ty then
    decl_ty
  else
    Option.map type_hint ~f:(Decl_hint.hint env.decl_env)

(* During the decl phase we can, for global inference, add "improved type hints".
   That is we can say that some missing type hints are in fact global tyvars.
   In that case to get the real type hint we must merge the type hint present
   in the ast with the one we created during the decl phase. This function does
   exactly this for the return type, the parameters and the variadic parameters.
   *)
let merge_decl_header_with_hints ~params ~ret ~variadic decl_header env =
  let ret_decl_ty =
    merge_hint_with_decl_hint
      env
      (hint_of_type_hint ret)
      (Option.map
         ~f:(fun { ft_ret = { et_type; _ }; _ } -> et_type)
         decl_header)
  in
  let params_decl_ty =
    match decl_header with
    | None ->
      List.map
        ~f:(fun h ->
          merge_hint_with_decl_hint
            env
            (hint_of_type_hint h.param_type_hint)
            None)
        params
    | Some { ft_params; _ } ->
      List.zip_exn params ft_params
      |> List.map ~f:(fun (h, { fp_type = { et_type; _ }; _ }) ->
             merge_hint_with_decl_hint
               env
               (hint_of_type_hint h.param_type_hint)
               (Some et_type))
  in
  let variadicity_decl_ty =
    match (decl_header, variadic) with
    | ( Some { ft_arity = Fvariadic { fp_type = { et_type; _ }; _ }; _ },
        FVvariadicArg fp ) ->
      merge_hint_with_decl_hint
        env
        (hint_of_type_hint fp.param_type_hint)
        (Some et_type)
    | (_, FVvariadicArg fp) ->
      merge_hint_with_decl_hint env (hint_of_type_hint fp.param_type_hint) None
    | _ -> None
  in
  (ret_decl_ty, params_decl_ty, variadicity_decl_ty)

(* Given a locl_ty type of params, check, in case it is a Tpu or a
 * Tpu_type_access, if the invocation is correct:
 * - Tpu(base, enum): enum must be a valid PU in base or its upper bounds
 * - Tpu_type_acces(tp, name): name must be a valid PU type name in tp's
 *   upper bounds
 *)
let check_pu_in_locl_ty env lty =
  let check env tp candidate =
    (* TODO(T70090664): added dummy [] here as type arguments *)
    let upper_bounds = Env.get_upper_bounds env tp [] in
    Typing_set.fold
      (fun bound (env, is_pu, is_pu_type_access) ->
        match get_node bound with
        | Tpu (lty, (_, enum)) ->
          let (env, puaccess) =
            Typing_utils.class_get_pu_type env lty enum candidate
          in
          let res = Option.is_some puaccess in
          (env, is_pu, res || is_pu_type_access)
        | _ ->
          let (env, pu) = TUtils.class_get_pu env bound candidate in
          let res = Option.is_some pu in
          (env, res || is_pu, is_pu_type_access))
      upper_bounds
      (env, false, false)
  in
  let check_pu_visitor =
    object
      inherit [env] Type_visitor.locl_type_visitor as super

      method! on_tpu env r base enum =
        let (env, res) = Typing_utils.class_get_pu env base (snd enum) in
        if Option.is_none res then Errors.pu_invalid_access (Reason.to_pos r) "";
        super#on_type env base

      method! on_tpu_type_access env r tp name =
        let (env, is_pu, is_pu_type_access) = check env (snd tp) (snd name) in
        let p = Reason.to_pos r in
        if is_pu && is_pu_type_access then
          Errors.pu_invalid_access p ": type is ambiguous"
        else if not (is_pu || is_pu_type_access) then
          Errors.pu_invalid_access p ": type is unknown";
        env
    end
  in
  check_pu_visitor#on_type env lty

let rec fun_def ctx f :
    (Tast.fun_def * Typing_inference_env.t_global_with_pos) option =
  Counters.count_typecheck @@ fun () ->
  Errors.run_with_span f.f_span @@ fun () ->
  let env = EnvFromDef.fun_env ctx f in
  with_timeout env f.f_name ~do_:(fun env ->
      (* reset the expression dependent display ids for each function body *)
      Reason.expr_display_id_map := IMap.empty;
      let pos = fst f.f_name in
      let decl_header = get_decl_function_header env (snd f.f_name) in
      let env = Env.open_tyvars env (fst f.f_name) in
      let env = Env.set_env_function_pos env pos in
      let env = Env.set_env_pessimize env in
      let env =
        Typing.attributes_check_def env SN.AttributeKinds.fn f.f_user_attributes
      in
      let (env, file_attrs) = Typing.file_attributes env f.f_file_attributes in
      let reactive =
        fun_reactivity env.decl_env f.f_user_attributes f.f_params
      in
      let mut = TUtils.fun_mutable f.f_user_attributes in
      let env = Env.set_env_reactive env reactive in
      let env = Env.set_fun_mutable env mut in
      Typing_check_decls.fun_ env f;
      let env =
        Phase.localize_and_add_ast_generic_parameters_and_where_constraints
          pos
          env
          f.f_tparams
          f.f_where_constraints
      in
      let env = Env.set_fn_kind env f.f_fun_kind in
      let (return_decl_ty, params_decl_ty, variadicity_decl_ty) =
        merge_decl_header_with_hints
          ~params:f.f_params
          ~ret:f.f_ret
          ~variadic:f.f_variadic
          decl_header
          env
      in
      let (env, return_ty) =
        match return_decl_ty with
        | None ->
          (env, Typing_return.make_default_return ~is_method:false env f.f_name)
        | Some ty ->
          let localize env ty = Phase.localize_with_self env ty in
          Typing_return.make_return_type localize env ty
      in
      let env = check_pu_in_locl_ty env return_ty in
      let return =
        Typing_return.make_info
          f.f_fun_kind
          f.f_user_attributes
          env
          ~is_explicit:(Option.is_some (hint_of_type_hint f.f_ret))
          return_ty
          return_decl_ty
      in
      let (env, param_tys) =
        List.zip_exn f.f_params params_decl_ty
        |> List.map_env env ~f:(fun env (param, hint) ->
               make_param_local_ty env hint param)
      in
      let partial_callback = Partial.should_check_error (Env.get_mode env) in
      let check_has_hint p t = check_param_has_hint env p t partial_callback in
      List.iter2_exn ~f:check_has_hint f.f_params param_tys;
      Typing_memoize.check_function env f;
      let (env, typed_params) =
        let bind_param_and_check env param =
          let (env, fun_param) = Typing.bind_param env param in
          let env = check_pu_in_locl_ty env (fst fun_param.param_type_hint) in
          (env, fun_param)
        in
        List.map_env
          env
          (List.zip_exn param_tys f.f_params)
          bind_param_and_check
      in
      let (env, t_variadic) =
        get_callable_variadicity
          ~is_function:true
          ~pos
          ~partial_callback
          env
          variadicity_decl_ty
          f.f_variadic
      in
      let env =
        set_tyvars_variance_in_callable env return_ty param_tys t_variadic
      in
      let local_tpenv = Env.get_tpenv env in
      let disable =
        Naming_attributes.mem
          SN.UserAttributes.uaDisableTypecheckerInternal
          f.f_user_attributes
      in
      let (env, f_cap, f_unsafe_cap) =
        Typing.type_capability env f.f_cap f.f_unsafe_cap (fst f.f_name)
      in
      let (env, _) =
        Typing_coeffects.register_capabilities
          env
          (type_of_type_hint f_cap)
          (type_of_type_hint f_unsafe_cap)
      in
      let (env, tb) =
        Typing.fun_ ~disable env return pos f.f_body f.f_fun_kind
      in
      (* restore original reactivity *)
      let env = Env.set_env_reactive env reactive in
      begin
        match hint_of_type_hint f.f_ret with
        | None ->
          if partial_callback 4030 then Errors.expecting_return_type_hint pos
        | Some hint -> Typing_return.async_suggest_return f.f_fun_kind hint pos
      end;
      let (env, tparams) = List.map_env env f.f_tparams Typing.type_param in
      let (env, user_attributes) =
        List.map_env env f.f_user_attributes Typing.user_attribute
      in
      let env =
        Typing_solver.close_tyvars_and_solve env Errors.bad_function_typevar
      in
      let env =
        Typing_solver.solve_all_unsolved_tyvars env Errors.bad_function_typevar
      in
      let fundef =
        {
          Aast.f_annotation = Env.save local_tpenv env;
          Aast.f_span = f.f_span;
          Aast.f_mode = f.f_mode;
          Aast.f_ret = (return_ty, hint_of_type_hint f.f_ret);
          Aast.f_name = f.f_name;
          Aast.f_tparams = tparams;
          Aast.f_where_constraints = f.f_where_constraints;
          Aast.f_variadic = t_variadic;
          Aast.f_params = typed_params;
          Aast.f_cap;
          Aast.f_unsafe_cap;
          Aast.f_fun_kind = f.f_fun_kind;
          Aast.f_file_attributes = file_attrs;
          Aast.f_user_attributes = user_attributes;
          Aast.f_body = { Aast.fb_ast = tb; fb_annotation = () };
          Aast.f_external = f.f_external;
          Aast.f_namespace = f.f_namespace;
          Aast.f_doc_comment = f.f_doc_comment;
          Aast.f_static = f.f_static;
        }
      in
      let (_env, global_inference_env) = Env.extract_global_inference_env env in
      (fundef, (pos, global_inference_env)))

and method_def env cls m =
  Errors.run_with_span m.m_span @@ fun () ->
  with_timeout env m.m_name ~do_:(fun env ->
      let initial_env = env in
      (* reset the expression dependent display ids for each method body *)
      Reason.expr_display_id_map := IMap.empty;
      let decl_header =
        get_decl_method_header
          (Env.get_tcopt env)
          cls
          (snd m.m_name)
          ~is_static:m.m_static
      in
      let pos = fst m.m_name in
      let env = Env.open_tyvars env (fst m.m_name) in
      let env = Env.reinitialize_locals env in
      let env = Env.set_env_function_pos env pos in
      let env =
        Typing.attributes_check_def
          env
          SN.AttributeKinds.mthd
          m.m_user_attributes
      in
      let reactive =
        fun_reactivity env.decl_env m.m_user_attributes m.m_params
      in
      let mut =
        match TUtils.fun_mutable m.m_user_attributes with
        | None ->
          (* <<__Mutable>> is implicit on constructors  *)
          if String.equal (snd m.m_name) SN.Members.__construct then
            Some Param_borrowed_mutable
          else
            None
        | x -> x
      in
      let env = Env.set_env_reactive env reactive in
      let env = Env.set_fun_mutable env mut in
      let env =
        Phase.localize_and_add_ast_generic_parameters_and_where_constraints
          pos
          env
          m.m_tparams
          m.m_where_constraints
      in
      let env =
        if Env.is_static env then
          env
        else
          Env.set_local env this (Env.get_self env) Pos.none
      in
      let env =
        match Env.get_self_class env with
        | None -> env
        | Some c ->
          (* Mark $this as a using variable if it has a disposable type *)
          if Cls.is_disposable c then
            Env.set_using_var env this
          else
            env
      in
      let env = Env.clear_params env in
      let (ret_decl_ty, params_decl_ty, variadicity_decl_ty) =
        merge_decl_header_with_hints
          ~params:m.m_params
          ~ret:m.m_ret
          ~variadic:m.m_variadic
          decl_header
          env
      in
      let env = Env.set_fn_kind env m.m_fun_kind in
      let (env, locl_ty) =
        match ret_decl_ty with
        | None ->
          (env, Typing_return.make_default_return ~is_method:true env m.m_name)
        | Some ret ->
          (* If a 'this' type appears it needs to be compatible with the
           * late static type
           *)
          let ety_env =
            { (Phase.env_with_self env) with from_class = Some CIstatic }
          in
          Typing_return.make_return_type (Phase.localize ~ety_env) env ret
      in
      let env = check_pu_in_locl_ty env locl_ty in
      let return =
        Typing_return.make_info
          m.m_fun_kind
          m.m_user_attributes
          env
          ~is_explicit:(Option.is_some (hint_of_type_hint m.m_ret))
          locl_ty
          ret_decl_ty
      in
      let (env, param_tys) =
        List.zip_exn m.m_params params_decl_ty
        |> List.map_env env ~f:(fun env (param, hint) ->
               make_param_local_ty env hint param)
      in
      let partial_callback = Partial.should_check_error (Env.get_mode env) in
      let param_fn p t = check_param_has_hint env p t partial_callback in
      List.iter2_exn ~f:param_fn m.m_params param_tys;
      Typing_memoize.check_method env m;
      let (env, typed_params) =
        let bind_param_and_check env param =
          let (env, fun_param) = Typing.bind_param env param in
          let env = check_pu_in_locl_ty env (fst fun_param.param_type_hint) in
          (env, fun_param)
        in
        List.map_env
          env
          (List.zip_exn param_tys m.m_params)
          bind_param_and_check
      in
      let (env, t_variadic) =
        get_callable_variadicity
          ~partial_callback
          ~pos
          env
          variadicity_decl_ty
          m.m_variadic
      in
      let env =
        set_tyvars_variance_in_callable env locl_ty param_tys t_variadic
      in
      let nb = Nast.assert_named_body m.m_body in
      let local_tpenv = Env.get_tpenv env in
      let disable =
        Naming_attributes.mem
          SN.UserAttributes.uaDisableTypecheckerInternal
          m.m_user_attributes
      in
      let (env, m_cap, m_unsafe_cap) =
        Typing.type_capability env m.m_cap m.m_unsafe_cap (fst m.m_name)
      in
      let (env, _) =
        Typing_coeffects.register_capabilities
          env
          (type_of_type_hint m_cap)
          (type_of_type_hint m_unsafe_cap)
      in
      let (env, tb) =
        Typing.fun_
          ~abstract:m.m_abstract
          ~disable
          env
          return
          pos
          nb
          m.m_fun_kind
      in
      (* restore original method reactivity  *)
      let env = Env.set_env_reactive env reactive in
      let type_hint' =
        match hint_of_type_hint m.m_ret with
        | None when String.equal (snd m.m_name) SN.Members.__construct ->
          Some (pos, Hprim Tvoid)
        | None ->
          if partial_callback 4030 then Errors.expecting_return_type_hint pos;
          None
        | Some hint ->
          Typing_return.async_suggest_return m.m_fun_kind hint (fst m.m_name);
          hint_of_type_hint m.m_ret
      in
      let m = { m with m_ret = (fst m.m_ret, type_hint') } in
      let (env, tparams) = List.map_env env m.m_tparams Typing.type_param in
      let (env, user_attributes) =
        List.map_env env m.m_user_attributes Typing.user_attribute
      in
      let env =
        Typing_solver.close_tyvars_and_solve env Errors.bad_method_typevar
      in
      let env =
        Typing_solver.solve_all_unsolved_tyvars env Errors.bad_method_typevar
      in
      let method_def =
        {
          Aast.m_annotation = Env.save local_tpenv env;
          Aast.m_span = m.m_span;
          Aast.m_final = m.m_final;
          Aast.m_static = m.m_static;
          Aast.m_abstract = m.m_abstract;
          Aast.m_visibility = m.m_visibility;
          Aast.m_name = m.m_name;
          Aast.m_tparams = tparams;
          Aast.m_where_constraints = m.m_where_constraints;
          Aast.m_variadic = t_variadic;
          Aast.m_params = typed_params;
          Aast.m_cap;
          Aast.m_unsafe_cap;
          Aast.m_fun_kind = m.m_fun_kind;
          Aast.m_user_attributes = user_attributes;
          Aast.m_ret = (locl_ty, hint_of_type_hint m.m_ret);
          Aast.m_body = { Aast.fb_ast = tb; fb_annotation = () };
          Aast.m_external = m.m_external;
          Aast.m_doc_comment = m.m_doc_comment;
        }
      in
      let (env, global_inference_env) = Env.extract_global_inference_env env in
      let _env = Env.log_env_change "method_def" initial_env env in
      (method_def, (pos, global_inference_env)))

(** Checks that extending this parent is legal - e.g. it is not final and not const. *)
and check_parent env class_def class_type =
  match Env.get_parent_class env with
  | Some parent_type ->
    let position = fst class_def.c_name in
    if Cls.const class_type && not (Cls.const parent_type) then
      Errors.self_const_parent_not position;
    if Cls.final parent_type then
      Errors.extend_final position (Cls.pos parent_type) (Cls.name parent_type)
  | None -> ()

and check_parent_sealed child_type parent_type =
  match Cls.sealed_whitelist parent_type with
  | None -> ()
  | Some whitelist ->
    let parent_pos = Cls.pos parent_type in
    let parent_name = Cls.name parent_type in
    let child_pos = Cls.pos child_type in
    let child_name = Cls.name child_type in
    let check kind action =
      if not (SSet.mem child_name whitelist) then
        Errors.extend_sealed child_pos parent_pos parent_name kind action
    in
    begin
      match (Cls.kind parent_type, Cls.kind child_type) with
      | (Ast_defs.Cinterface, Ast_defs.Cinterface) -> check "interface" "extend"
      | (Ast_defs.Cinterface, _) -> check "interface" "implement"
      | (Ast_defs.Ctrait, _) -> check "trait" "use"
      | (Ast_defs.Cabstract, _)
      | (Ast_defs.Cnormal, _) ->
        check "class" "extend"
      | (Ast_defs.Cenum, _) -> ()
    end

and check_parents_sealed env child_def child_type =
  let parents =
    child_def.c_extends @ child_def.c_implements @ child_def.c_uses
  in
  List.iter parents (function
      | (_, Happly ((_, name), _)) ->
        begin
          match Env.get_class_dep env name with
          | Some parent_type -> check_parent_sealed child_type parent_type
          | None -> ()
        end
      | _ -> ())

(* Reject multiple instantiations of the same generic interface
 * in extends and implements clauses.
 * e.g. disallow class C implements I<string>, I<int>
 *
 * O(n^2) but we don't expect number of instantiated interfaces to be large
 *)
and check_implements_or_extends_unique impl =
  match impl with
  | [] -> ()
  | ty :: rest ->
    (match get_node ty with
    | Tapply ((pos, name), _ :: _) ->
      let (pos_list, rest) =
        List.partition_map rest (fun ty ->
            match get_node ty with
            | Tapply ((pos', name'), _) when String.equal name name' ->
              `Fst pos'
            | _ -> `Snd ty)
      in
      if not (List.is_empty pos_list) then
        Errors.duplicate_interface pos name pos_list;
      check_implements_or_extends_unique rest
    | _ -> check_implements_or_extends_unique rest)

and check_cstr_dep deps =
  List.iter deps (fun dep ->
      match deref dep with
      | (_, Tapply _) -> ()
      | (r, Tgeneric _) ->
        let p = Typing_reason.to_pos r in
        Errors.expected_class ~suffix:" or interface but got a generic" p
      | (r, _) ->
        let p = Typing_reason.to_pos r in
        Errors.expected_class ~suffix:" or interface" p)

and check_const_trait_members pos env use_list =
  let (_, trait, _) = Decl_utils.unwrap_class_hint use_list in
  match Env.get_class env trait with
  | Some c when Ast_defs.(equal_class_kind (Cls.kind c) Ctrait) ->
    List.iter (Cls.props c) (fun (x, ce) ->
        if not (get_ce_const ce) then Errors.trait_prop_const_class pos x)
  | _ -> ()

let check_consistent_enum_inclusion included_cls dest_cls =
  match (Cls.enum_type included_cls, Cls.enum_type dest_cls) with
  | (Some included_e, Some dest_e) ->
    (* ensure that the base types are identical *)
    if not (Typing_defs.equal_decl_ty included_e.te_base dest_e.te_base) then
      Errors.incompatible_enum_inclusion_base
        (Cls.pos dest_cls)
        (Cls.name dest_cls)
        (Cls.name included_cls);
    (* ensure that the visibility constraint are compatible *)
    (match (included_e.te_constraint, dest_e.te_constraint) with
    | (None, Some _) ->
      Errors.incompatible_enum_inclusion_constraint
        (Cls.pos dest_cls)
        (Cls.name dest_cls)
        (Cls.name included_cls)
    | (_, _) -> ());
    (* ensure normal enums can't include enum classes *)
    if included_e.te_enum_class && not dest_e.te_enum_class then
      Errors.wrong_extend_kind
        ~parent_pos:(Cls.pos included_cls)
        ~parent_kind:Ast_defs.Cenum
        ~parent_name:(Cls.name included_cls)
        ~parent_is_enum_class:true
        ~child_pos:(Cls.pos dest_cls)
        ~child_kind:Ast_defs.Cenum
        ~child_name:(Cls.name dest_cls)
        ~child_is_enum_class:false
  | (None, _) ->
    Errors.enum_inclusion_not_enum
      (Cls.pos dest_cls)
      (Cls.name dest_cls)
      (Cls.name included_cls)
  | (_, _) -> ()

let check_enum_includes env cls =
  (* checks that there are no duplicated enum-constants when folded-decls are enabled *)
  if Ast_defs.is_c_enum cls.c_kind then (
    let (dest_class_pos, dest_class_name) = cls.c_name in
    let enum_constant_map = ref SMap.empty in
    (* prepopulate the map with the constants declared in cls *)
    List.iter cls.c_consts ~f:(fun cc ->
        enum_constant_map :=
          SMap.add
            (snd cc.cc_id)
            (fst cc.cc_id, dest_class_name)
            !enum_constant_map);
    (* for all included enums *)
    let included_enums =
      Aast.enum_includes_map cls.c_enum ~f:(fun ce ->
          List.filter_map ce.e_includes ~f:(fun ie ->
              match snd ie with
              | Happly (sid, _) ->
                (match Env.get_class env (snd sid) with
                | None -> None
                | Some ie_cls -> Some (fst ie, ie_cls))
              | _ -> None))
    in
    List.iter included_enums ~f:(fun (ie_pos, ie_cls) ->
        let src_class_name = Cls.name ie_cls in
        (* 1. Check for consistency *)
        (match Env.get_class env dest_class_name with
        | None -> ()
        | Some cls -> check_consistent_enum_inclusion ie_cls cls);
        (* 2. Check for duplicates *)
        List.iter (Cls.consts ie_cls) ~f:(fun (const_name, class_const) ->
            ( if String.equal const_name "class" then
              ()
            else if SMap.mem const_name !enum_constant_map then
              (* distinguish between multiple inherit and redeclare *)
              let (origin_const_pos, origin_class_name) =
                SMap.find const_name !enum_constant_map
              in
              if String.equal origin_class_name dest_class_name then
                (* redeclare *)
                Errors.redeclaring_classish_const
                  dest_class_pos
                  dest_class_name
                  origin_const_pos
                  src_class_name
                  const_name
              else
                (* multiple inherit *)
                Errors.reinheriting_classish_const
                  dest_class_pos
                  dest_class_name
                  ie_pos
                  src_class_name
                  origin_class_name
                  const_name );
            enum_constant_map :=
              SMap.add
                const_name
                (dest_class_pos, class_const.cc_origin)
                !enum_constant_map))
  )

let shallow_decl_enabled (ctx : Provider_context.t) : bool =
  TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)

let class_type_param env ct =
  let (env, tparam_list) = List.map_env env ct Typing.type_param in
  (env, tparam_list)

let rec class_def ctx c =
  Counters.count_typecheck @@ fun () ->
  Errors.run_with_span c.c_span @@ fun () ->
  let env = EnvFromDef.class_env ctx c in
  let tc = Env.get_class env (snd c.c_name) in
  let env = Env.set_env_pessimize env in
  Typing_helpers.add_decl_errors (Option.bind tc Cls.decl_errors);
  Typing_check_decls.class_ env c;
  NastInitCheck.class_ env c;
  match tc with
  | None ->
    (* This can happen if there was an error during the declaration
     * of the class. *)
    None
  | Some tc ->
    (* If there are duplicate definitions of the class then we will end up
       * checking one AST with respect to the decl corresponding to the other definition.
       * Naming has already detected duplicates, so let's just avoid cascading unhelpful
       * typing errors, and also avoid triggering the bad position assert
       *)
    if not (Pos.equal (fst c.c_name) (Cls.pos tc)) then
      None
    else
      let env = Typing_requirements.check_class env tc in
      if shallow_decl_enabled ctx then Typing_inheritance.check_class env tc;
      Some (class_def_ env c tc)

and class_def_ env c tc =
  let env =
    let kind =
      match c.c_kind with
      | Ast_defs.Cenum -> SN.AttributeKinds.enum
      | _ -> SN.AttributeKinds.cls
    in
    Typing.attributes_check_def env kind c.c_user_attributes
  in
  let (env, file_attrs) = Typing.file_attributes env c.c_file_attributes in
  let ctx = Env.get_ctx env in
  if
    ( Ast_defs.(equal_class_kind c.c_kind Cnormal)
    || Ast_defs.(equal_class_kind c.c_kind Cabstract) )
    && not (shallow_decl_enabled ctx)
  then (
    (* These checks are only for eager mode. The same checks are performed
     * for shallow mode in Typing_inheritance *)
    let method_pos ~is_static class_id meth_id =
      let get_meth =
        if is_static then
          Decl_heap.StaticMethods.get
        else
          Decl_heap.Methods.get
      in
      match get_meth (class_id, meth_id) with
      | Some { fe_pos; _ } -> fe_pos
      | None -> Pos.none
    in
    let check_override ~is_static (id, ce) =
      if get_ce_override ce then
        let pos = method_pos ~is_static ce.ce_origin id in
        (* Method is actually defined in this class *)
        if String.equal ce.ce_origin (snd c.c_name) then
          Errors.should_be_override pos (snd c.c_name) id
        else
          match Env.get_class env ce.ce_origin with
          | None -> ()
          | Some parent_class ->
            (* If it's not defined here, then either it's inherited (so we have emitted an error already)
             * or it's in a trait, and so we need to emit the error now *)
            if not Ast_defs.(equal_class_kind (Cls.kind parent_class) Ctrait)
            then
              ()
            else
              Errors.override_per_trait c.c_name id pos
    in

    List.iter (Cls.methods tc) (check_override ~is_static:false);
    List.iter (Cls.smethods tc) (check_override ~is_static:true)
  );
  check_enum_includes env c;
  let (pc, _) = c.c_name in
  let (req_extends, req_implements) = split_reqs c in
  let extends = List.map c.c_extends (Decl_hint.hint env.decl_env) in
  let implements = List.map c.c_implements (Decl_hint.hint env.decl_env) in
  let uses = List.map c.c_uses (Decl_hint.hint env.decl_env) in
  let req_extends = List.map req_extends (Decl_hint.hint env.decl_env) in
  let req_implements = List.map req_implements (Decl_hint.hint env.decl_env) in
  let additional_parents =
    (* In an abstract class or a trait, we assume the interfaces
       will be implemented in the future, so we take them as
       part of the class (as requested by dependency injection implementers) *)
    match c.c_kind with
    | Ast_defs.Cabstract -> implements
    | Ast_defs.Ctrait -> implements @ req_implements
    | _ -> []
  in
  check_implements_or_extends_unique implements;
  check_implements_or_extends_unique extends;
  check_cstr_dep extends;
  check_cstr_dep uses;
  check_cstr_dep req_extends;
  check_cstr_dep additional_parents;
  begin
    match c.c_enum with
    | Some e ->
      check_cstr_dep (List.map e.e_includes (Decl_hint.hint env.decl_env))
    | _ -> ()
  end;
  let impl = extends @ implements @ uses in
  let env =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      (fst c.c_name)
      env
      c.c_tparams
      c.c_where_constraints
  in
  let env =
    Phase.check_where_constraints
      ~in_class:true
      ~use_pos:pc
      ~definition_pos:pc
      ~ety_env:(Phase.env_with_self env)
      env
      (Cls.where_constraints tc)
  in
  Typing_variance.class_ (Env.get_ctx env) (snd c.c_name) tc impl;
  let check_where_constraints env ht =
    let (_, (p, _), _) = TUtils.unwrap_class_type ht in
    let (env, locl_ty) = Phase.localize_with_self env ht in
    match get_node (TUtils.get_base_type env locl_ty) with
    | Tclass (cls, _, tyl) ->
      (match Env.get_class env (snd cls) with
      | Some cls when not (List.is_empty (Cls.where_constraints cls)) ->
        let tc_tparams = Cls.tparams cls in
        let ety_env =
          {
            (Phase.env_with_self env) with
            substs = Subst.make_locl tc_tparams tyl;
          }
        in
        Phase.check_where_constraints
          ~in_class:true
          ~use_pos:pc
          ~definition_pos:p
          ~ety_env
          env
          (Cls.where_constraints cls)
      | _ -> env)
    | _ -> env
  in
  let env = List.fold impl ~init:env ~f:check_where_constraints in
  check_parent env c tc;
  check_parents_sealed env c tc;

  let is_final = Cls.final tc in
  if
    (Ast_defs.(equal_class_kind (Cls.kind tc) Cnormal) || is_final)
    && Cls.members_fully_known tc
  then (
    check_extend_abstract_meth ~is_final pc (Cls.methods tc);
    (match fst (Cls.construct tc) with
    | Some constr ->
      check_extend_abstract_meth ~is_final pc [(SN.Members.__construct, constr)]
    | None -> ());
    check_extend_abstract_meth ~is_final pc (Cls.smethods tc);
    check_extend_abstract_prop ~is_final pc (Cls.sprops tc);
    check_extend_abstract_const ~is_final pc (Cls.consts tc);
    check_extend_abstract_typeconst ~is_final pc (Cls.typeconsts tc)
  );
  if Cls.const tc then List.iter c.c_uses (check_const_trait_members pc env);
  let (static_vars, vars) = split_vars c in
  List.iter static_vars ~f:(fun { cv_id = (p, id); _ } ->
      check_static_class_element (Cls.get_prop tc) ~elt_type:`Property id p);
  List.iter vars ~f:(fun { cv_id = (p, id); _ } ->
      check_dynamic_class_element (Cls.get_sprop tc) ~elt_type:`Property id p);
  let (constructor, static_methods, methods) = split_methods c in
  List.iter static_methods ~f:(fun { m_name = (p, id); _ } ->
      check_static_class_element (Cls.get_method tc) ~elt_type:`Method id p);
  List.iter methods ~f:(fun { m_name = (p, id); _ } ->
      check_dynamic_class_element (Cls.get_smethod tc) ~elt_type:`Method id p);
  let env =
    List.fold ~init:env impl ~f:(fun env -> class_implements_type env c)
  in
  if Cls.is_disposable tc then
    List.iter
      (c.c_extends @ c.c_uses)
      (Typing_disposable.enforce_is_disposable env);
  let (env, typed_vars_and_global_inference_envs) =
    List.map_env env vars (class_var_def ~is_static:false tc)
  in
  let (typed_vars, vars_global_inference_envs) =
    List.unzip typed_vars_and_global_inference_envs
  in
  let (typed_methods, methods_global_inference_envs) =
    List.filter_map methods (method_def env tc) |> List.unzip
  in
  let (env, typed_typeconsts) =
    List.map_env env c.c_typeconsts (typeconst_def c)
  in
  let (env, consts) = List.map_env env c.c_consts (class_const_def c) in
  let (typed_consts, const_types) = List.unzip consts in
  let env = Typing_enum.enum_class_check env tc c.c_consts const_types in
  let typed_constructor = class_constr_def env tc constructor in
  let env = Env.set_static env in
  let (env, typed_static_vars_and_global_inference_envs) =
    List.map_env env static_vars (class_var_def ~is_static:true tc)
  in
  let (typed_static_vars, static_vars_global_inference_envs) =
    List.unzip typed_static_vars_and_global_inference_envs
  in
  let (typed_static_methods, static_methods_global_inference_envs) =
    List.filter_map static_methods (method_def env tc) |> List.unzip
  in
  let (methods, constr_global_inference_env) =
    match typed_constructor with
    | None -> (typed_static_methods @ typed_methods, [])
    | Some (m, global_inference_env) ->
      ((m :: typed_static_methods) @ typed_methods, [global_inference_env])
  in
  let pu_enums =
    try List.map c.c_pu_enums ~f:(pu_enum_def env (snd c.c_name))
    with InvalidPocketUniverse -> []
  in
  let (env, tparams) = class_type_param env c.c_tparams in
  let (env, user_attributes) =
    List.map_env env c.c_user_attributes Typing.user_attribute
  in
  let env =
    Typing_solver.solve_all_unsolved_tyvars env Errors.bad_class_typevar
  in
  ( {
      Aast.c_span = c.c_span;
      Aast.c_annotation = Env.save (Env.get_tpenv env) env;
      Aast.c_mode = c.c_mode;
      Aast.c_final = c.c_final;
      Aast.c_is_xhp = c.c_is_xhp;
      Aast.c_has_xhp_keyword = c.c_has_xhp_keyword;
      Aast.c_kind = c.c_kind;
      Aast.c_name = c.c_name;
      Aast.c_tparams = tparams;
      Aast.c_extends = c.c_extends;
      Aast.c_uses = c.c_uses;
      (* c_use_as_alias and c_insteadof_alias are PHP features not supported
       * in Hack but are required since we have runtime support for it
       *)
      Aast.c_use_as_alias = [];
      Aast.c_insteadof_alias = [];
      Aast.c_xhp_attr_uses = c.c_xhp_attr_uses;
      Aast.c_xhp_category = c.c_xhp_category;
      Aast.c_reqs = c.c_reqs;
      Aast.c_implements = c.c_implements;
      Aast.c_where_constraints = c.c_where_constraints;
      Aast.c_consts = typed_consts;
      Aast.c_typeconsts = typed_typeconsts;
      Aast.c_vars = typed_static_vars @ typed_vars;
      Aast.c_methods = methods;
      Aast.c_file_attributes = file_attrs;
      Aast.c_user_attributes = user_attributes;
      Aast.c_namespace = c.c_namespace;
      Aast.c_enum = c.c_enum;
      Aast.c_doc_comment = c.c_doc_comment;
      Aast.c_attributes = [];
      Aast.c_xhp_children = c.c_xhp_children;
      Aast.c_xhp_attrs = [];
      Aast.c_pu_enums = pu_enums;
      Aast.c_emit_id = c.c_emit_id;
    },
    methods_global_inference_envs
    @ static_methods_global_inference_envs
    @ constr_global_inference_env
    @ static_vars_global_inference_envs
    @ vars_global_inference_envs )

(** Checks that a dynamic element is also dynamic in the parents. *)
and check_dynamic_class_element get_static_elt element_name dyn_pos ~elt_type =
  (* The non-static properties that we get passed do not start with '$', but the
     static properties we want to look up do, so add it. *)
  let id =
    match elt_type with
    | `Method -> element_name
    | `Property -> "$" ^ element_name
  in
  match get_static_elt id with
  | None -> ()
  | Some static_element ->
    let (lazy ty) = static_element.ce_type in
    Errors.static_redeclared_as_dynamic
      dyn_pos
      (get_pos ty)
      element_name
      ~elt_type

(** Checks that a static element is also static in the parents. *)
and check_static_class_element get_dyn_elt element_name static_pos ~elt_type =
  (* The static properties that we get passed in start with '$', but the
     non-static properties we're matching against don't, so we need to detect
     that and remove it if present. *)
  let element_name = String_utils.lstrip element_name "$" in
  match get_dyn_elt element_name with
  | None -> ()
  | Some dyn_element ->
    let (lazy ty) = dyn_element.ce_type in
    Errors.dynamic_redeclared_as_static
      static_pos
      (get_pos ty)
      element_name
      ~elt_type

(** Error if there are abstract methods that this class is supposed to provide
    implementation for. *)
and check_extend_abstract_meth ~is_final p seq =
  List.iter seq (fun (x, ce) ->
      match ce.ce_type with
      | (lazy ty) when get_ce_abstract ce && is_fun ty ->
        Errors.implement_abstract ~is_final p (get_pos ty) "method" x
      | _ -> ())

and check_extend_abstract_prop ~is_final p seq =
  List.iter seq (fun (x, ce) ->
      if get_ce_abstract ce then
        let ce_pos = Lazy.force ce.ce_type |> get_pos in
        Errors.implement_abstract ~is_final p ce_pos "property" x)

(* Type constants must be bound to a concrete type for non-abstract classes.
 *)
and check_extend_abstract_typeconst ~is_final p seq =
  List.iter seq (fun (x, tc) ->
      if Option.is_none tc.ttc_type then
        Errors.implement_abstract
          ~is_final
          p
          (fst tc.ttc_name)
          "type constant"
          x)

and check_extend_abstract_const ~is_final p seq =
  List.iter seq (fun (x, cc) ->
      if cc.cc_abstract && not cc.cc_synthesized then
        let cc_pos = get_pos cc.cc_type in
        Errors.implement_abstract ~is_final p cc_pos "constant" x)

and get_decl_prop_ty env cls ~is_static prop_id =
  let is_global_inference_on = TCO.global_inference (Env.get_tcopt env) in
  if is_global_inference_on then
    let prop_opt =
      if is_static then
        (* this is very ad-hoc, but this is how we do it in the decl-heap *)
        Cls.get_sprop cls ("$" ^ prop_id)
      else
        Cls.get_prop cls prop_id
    in
    match prop_opt with
    | None -> failwith "error: could not find property in decl heap"
    | Some { ce_type; _ } -> Some (Lazy.force ce_type)
  else
    None

and typeconst_def
    cls
    env
    {
      c_tconst_abstract;
      c_tconst_name = (pos, _) as id;
      c_tconst_constraint;
      c_tconst_type = hint;
      c_tconst_user_attributes;
      c_tconst_span;
      c_tconst_doc_comment;
    } =
  begin
    match cls.c_kind with
    | Ast_defs.Ctrait
    | Ast_defs.Cenum ->
      let kind =
        match cls.c_kind with
        | Ast_defs.Ctrait -> `trait
        | Ast_defs.Cenum -> `enum
        | _ -> assert false
      in
      Errors.cannot_declare_constant kind pos cls.c_name
    | _ -> ()
  end;
  let (env, cstr) = opt Phase.localize_hint_with_self env c_tconst_constraint in
  let (env, ty) =
    match hint with
    | None -> (env, None)
    | Some hint ->
      let ty = Decl_hint.hint env.decl_env hint in
      (* We want to report cycles through the definition *)
      let name = snd cls.c_name ^ "::" ^ snd id in
      let (env, ty) =
        Phase.localize_with_self env ~pos ~report_cycle:(pos, name) ty
      in
      (env, Some ty)
  in
  let check env t c =
    Type.sub_type pos Reason.URtypeconst_cstr env t c Errors.unify_error
  in
  let env = Option.value ~default:env @@ Option.map2 ty cstr ~f:(check env) in
  let env =
    match hint with
    | Some (pos, Hshape { nsi_field_map; _ }) ->
      let get_name sfi = sfi.sfi_name in
      Typing.check_shape_keys_validity
        env
        pos
        (List.map ~f:get_name nsi_field_map)
    | _ -> env
  in
  let env =
    Typing.attributes_check_def
      env
      SN.AttributeKinds.typeconst
      c_tconst_user_attributes
  in
  let (env, user_attributes) =
    List.map_env env c_tconst_user_attributes Typing.user_attribute
  in
  ( env,
    {
      Aast.c_tconst_abstract;
      Aast.c_tconst_name = id;
      Aast.c_tconst_constraint;
      Aast.c_tconst_type = hint;
      Aast.c_tconst_user_attributes = user_attributes;
      Aast.c_tconst_span;
      Aast.c_tconst_doc_comment;
    } )

and pu_enum_def
    env
    c_name
    {
      pu_name;
      pu_user_attributes;
      pu_is_final;
      pu_case_types;
      pu_case_values;
      pu_members;
      _;
    } =
  (* What is a well-formed pocket universe?
    - pu_name is unique
    - pu_case_types are well-formed if all names are unique
    - pu_case_values are well-formed if all types are well-formed in an
       environment where all names in pu_case_types are bound to abstract types
    - pu_members are well-formed if:
      - each name is unique
      - all types are defined one and only one times in pum_types, and
        are well-formed in an environment
      - all values are defined one and only one times in pum_types, and
        are correctly typed according to pum_types instances.
      - all types satisfy their case type constraints. This is actually
        done in Typing_check_decls

    Note: Structural correctness (mostly uniqueness and exhaustivity)
    is checked during Nast check.
    If the Nast check fails, this function might raise InvalidPocketUniverse,
    that we silently ignore not to spam with duplicated errors.

    Here we check that all definitions are well-typed.
 *)
  let pos = fst pu_name in
  (* Query the decl provider to get all case type and case expressions
   * from the parent hierarchy, flattened
   *)
  let cls = Decl_provider.get_class (Env.get_ctx env) c_name in
  let pu_enum =
    Option.bind cls ~f:(fun cls -> Cls.get_pu_enum cls (snd pu_name))
  in
  let (pu_enum_case_types, pu_enum_case_values) =
    match pu_enum with
    | None ->
      Errors.internal_error pos "Missing PU decl in provider";
      (SMap.empty, SMap.empty)
    | Some pu_enum -> (pu_enum.tpu_case_types, pu_enum.tpu_case_values)
  in
  let (env, pu_user_attributes) =
    List.map_env env pu_user_attributes Typing.user_attribute
  in
  (* Adds all of the PU case types (not just the local one) as generics in the environment. *)
  let case_types =
    SMap.fold (fun _ (_, case_ty) acc -> case_ty :: acc) pu_enum_case_types []
  in
  let env = Phase.localize_and_add_generic_parameters pos env case_types in
  (* Localize local case values, to check they are correct types *)
  let () =
    List.iter pu_case_values ~f:(fun (_sid, hint) ->
        let (_ : env * locl_ty) = Phase.localize_hint_with_self env hint in
        ())
  in
  (* Localize all case values once, since checking members might need non
   * local information *)
  let (env, pu_enum_case_values) =
    SMap.map_env
      (fun env _key (_, sid, decl_ty) ->
        let (env, locl_ty) = Phase.localize_with_self env decl_ty in
        (env, (sid, locl_ty)))
      env
      pu_enum_case_values
  in
  (* Now we are going to check that each member is well-typed. *)
  let (_, members) =
    let process_member env pum =
      (* As mentioned above, we only check that the expressions have the
       * right type. Types constraints have already been validated in
       * Typing_check_decls
       *)
      (* Check that case expression are correctly typed *)
      let make_aast_tparam (sid, hint) =
        let hint_ty = Decl_hint.hint env.decl_env hint in
        {
          tp_variance = Ast_defs.Invariant;
          tp_name = sid;
          tp_tparams = [];
          tp_constraints = [(Ast_defs.Constraint_eq, hint_ty)];
          tp_reified = Aast.Erased;
          tp_user_attributes = [];
        }
      in
      let pum_types = List.map ~f:make_aast_tparam pum.pum_types in
      let env =
        Phase.localize_and_add_generic_parameters
          (fst pum.pum_atom)
          env
          pum_types
      in
      let process_mapping env (sid, map_expr) =
        (* Fetch expected type from the case types map *)
        let (ty, expected) =
          let ty =
            match SMap.find_opt (snd sid) pu_enum_case_values with
            | None -> raise InvalidPocketUniverse
            | Some (_, locl_ty) -> locl_ty
          in
          (ty, Some (ExpectedTy.make (fst sid) Reason.URhint ty))
        in
        (* Infer the type of the expression *)
        let (env, expr, ty') = Typing.expr ?expected env map_expr in
        (* Type checking *)
        let env =
          Typing_ops.sub_type
            (fst sid)
            Reason.URhint
            env
            ty'
            ty
            Errors.pocket_universes_typing
        in
        (env, (sid, expr))
      in
      let (env, pum_exprs) = List.map_env env pum.pum_exprs process_mapping in
      let members =
        {
          Aast.pum_atom = pum.pum_atom;
          Aast.pum_types = pum.pum_types;
          Aast.pum_exprs;
        }
      in
      (env, members)
    in
    List.fold_map ~init:env ~f:process_member pu_members
  in
  (* Localize local case types *)
  let (env, locl_case_types) =
    List.map_env env pu_case_types Typing.type_param
  in
  let local_tpenv = Env.get_tpenv env in
  let local_tpenv =
    List.fold
      ~init:local_tpenv
      ~f:(fun tpenv { tp_name; tp_reified; _ } ->
        let (def_pos, name) = tp_name in
        let tpinfo =
          Typing_kinding_defs.
            {
              lower_bounds = TySet.empty;
              upper_bounds = TySet.empty;
              reified = tp_reified;
              enforceable = false;
              (* TODO(T35357243) improve to support that *)
              newable = false (* TODO(T35357243) improve to support that *);
              (* TODO(T70090664) Is this correct? *)
              parameters = [];
            }
        in
        TPEnv.add ~def_pos name tpinfo tpenv)
      pu_case_types
  in
  Aast.
    {
      pu_annotation = Env.save local_tpenv env;
      pu_name;
      pu_user_attributes;
      pu_is_final;
      pu_case_types = locl_case_types;
      pu_case_values;
      pu_members = members;
    }

(* This should agree with the set of expressions whose type can be inferred in
 * Decl_utils.infer_const
 *)
and is_literal_expr e =
  match snd e with
  | String _
  | True
  | False
  | Int _
  | Float _
  | Null ->
    true
  | Unop ((Ast_defs.Uminus | Ast_defs.Uplus), (_, (Int _ | Float _))) -> true
  | _ -> false

and class_const_def c env cc =
  let { cc_type = h; cc_id = id; cc_expr = e; _ } = cc in
  begin
    match c.c_kind with
    | Ast_defs.Ctrait -> Errors.cannot_declare_constant `trait (fst id) c.c_name
    | _ -> ()
  end;
  let (env, ty, opt_expected) =
    match h with
    | None ->
      begin
        match e with
        | None -> ()
        | Some e ->
          if
            (not (is_literal_expr e))
            && Partial.should_check_error c.c_mode 2035
            && not Ast_defs.(equal_class_kind c.c_kind Cenum)
          then
            Errors.missing_typehint (fst id)
      end;
      let (env, ty) = Env.fresh_type env (fst id) in
      (env, MakeType.unenforced ty, None)
    | Some h ->
      let ty = Decl_hint.hint env.decl_env h in
      let ty = Typing_enforceability.compute_enforced_ty env ty in
      let (env, ty) = Phase.localize_possibly_enforced_with_self env ty in
      ( env,
        ty,
        Some (ExpectedTy.make_and_allow_coercion (fst id) Reason.URhint ty) )
  in
  let (env, eopt, ty) =
    match e with
    | Some e ->
      let (env, te, ty') = Typing.expr ?expected:opt_expected env e in
      let env =
        Typing_coercion.coerce_type
          (fst id)
          Reason.URhint
          env
          ty'
          ty
          Errors.class_constant_value_does_not_match_hint
      in
      (env, Some te, ty')
    | None -> (env, None, ty.et_type)
  in
  ( env,
    ( {
        Aast.cc_type = cc.cc_type;
        Aast.cc_id = cc.cc_id;
        Aast.cc_expr = eopt;
        Aast.cc_doc_comment = cc.cc_doc_comment;
      },
      ty ) )

and class_constr_def env cls constructor =
  let env = { env with inside_constructor = true } in
  Option.bind constructor (method_def env cls)

and class_implements_type env c1 ctype2 =
  let params =
    List.map c1.c_tparams (fun { tp_name = (p, s); _ } ->
        mk (Reason.Rwitness p, Tgeneric (s, [])))
  in
  let r = Reason.Rwitness (fst c1.c_name) in
  let ctype1 = mk (r, Tapply (c1.c_name, params)) in
  Typing_extends.check_implements env ctype2 ctype1

(** Type-check a property declaration, with optional initializer *)
and class_var_def ~is_static cls env cv =
  (* First pick up and localize the hint if it exists *)
  let decl_cty =
    merge_hint_with_decl_hint
      env
      (hint_of_type_hint cv.cv_type)
      (get_decl_prop_ty env cls ~is_static (snd cv.cv_id))
  in
  let (env, expected) =
    match decl_cty with
    | None -> (env, None)
    | Some decl_cty ->
      let decl_cty = Typing_enforceability.compute_enforced_ty env decl_cty in
      let (env, cty) =
        Phase.localize_possibly_enforced_with_self env decl_cty
      in
      let expected =
        Some (ExpectedTy.make_and_allow_coercion cv.cv_span Reason.URhint cty)
      in
      (env, expected)
  in
  (* Next check the expression, passing in expected type if present *)
  let (env, typed_cv_expr) =
    match cv.cv_expr with
    | None -> (env, None)
    | Some e ->
      let (env, te, ty) = Typing.expr ?expected env e in
      (* Check that the inferred type is a subtype of the expected type.
       * Eventually this will be the responsibility of `expr`
       *)
      let env =
        match expected with
        | None -> env
        | Some ExpectedTy.{ pos = p; reason = ur; ty = cty } ->
          Typing_coercion.coerce_type
            p
            ur
            env
            ty
            cty
            Errors.class_property_initializer_type_does_not_match_hint
      in
      (env, Some te)
  in

  let env =
    if is_static then
      Typing.attributes_check_def
        env
        SN.AttributeKinds.staticProperty
        cv.cv_user_attributes
    else
      Typing.attributes_check_def
        env
        SN.AttributeKinds.instProperty
        cv.cv_user_attributes
  in
  let (env, user_attributes) =
    List.map_env env cv.cv_user_attributes Typing.user_attribute
  in
  if
    Option.is_none (hint_of_type_hint cv.cv_type)
    && Partial.should_check_error (Env.get_mode env) 2001
  then
    Errors.prop_without_typehint
      (string_of_visibility cv.cv_visibility)
      cv.cv_id;
  let (env, global_inference_env) = Env.extract_global_inference_env env in
  let cv_type =
    match expected with
    | Some expected ->
      (expected.ExpectedTy.ty.et_type, hint_of_type_hint cv.cv_type)
    | None -> Tast.dummy_type_hint (hint_of_type_hint cv.cv_type)
  in
  ( env,
    ( {
        Aast.cv_final = cv.cv_final;
        Aast.cv_xhp_attr = cv.cv_xhp_attr;
        Aast.cv_abstract = cv.cv_abstract;
        Aast.cv_visibility = cv.cv_visibility;
        Aast.cv_type;
        Aast.cv_id = cv.cv_id;
        Aast.cv_expr = typed_cv_expr;
        Aast.cv_user_attributes = user_attributes;
        Aast.cv_is_promoted_variadic = cv.cv_is_promoted_variadic;
        Aast.cv_doc_comment = cv.cv_doc_comment;
        (* Can make None to save space *)
        Aast.cv_is_static = is_static;
        Aast.cv_span = cv.cv_span;
      },
      (cv.cv_span, global_inference_env) ) )

let gconst_def ctx cst =
  Counters.count_typecheck @@ fun () ->
  Errors.run_with_span cst.cst_span @@ fun () ->
  let env = EnvFromDef.gconst_env ctx cst in
  let env = Env.set_env_pessimize env in
  let (typed_cst_value, env) =
    let value = cst.cst_value in
    match cst.cst_type with
    | Some hint ->
      let ty = Decl_hint.hint env.decl_env hint in
      let ty = Typing_enforceability.compute_enforced_ty env ty in
      let (env, dty) = Phase.localize_possibly_enforced_with_self env ty in
      let (env, te, value_type) =
        let expected =
          ExpectedTy.make_and_allow_coercion (fst hint) Reason.URhint dty
        in
        Typing.expr ~expected env value
      in
      let env =
        Typing_coercion.coerce_type
          (fst hint)
          Reason.URhint
          env
          value_type
          dty
          Errors.unify_error
      in
      (te, env)
    | None ->
      if
        (not (is_literal_expr value))
        && Partial.should_check_error cst.cst_mode 2035
      then
        Errors.missing_typehint (fst cst.cst_name);
      let (env, te, _value_type) = Typing.expr env value in
      (te, env)
  in
  {
    Aast.cst_annotation = Env.save (Env.get_tpenv env) env;
    Aast.cst_mode = cst.cst_mode;
    Aast.cst_name = cst.cst_name;
    Aast.cst_type = cst.cst_type;
    Aast.cst_value = typed_cst_value;
    Aast.cst_namespace = cst.cst_namespace;
    Aast.cst_span = cst.cst_span;
    Aast.cst_emit_id = cst.cst_emit_id;
  }

let record_field env f =
  let (id, hint, e) = f in
  let ((p, _) as cty) = hint in
  let (env, cty) =
    let cty = Decl_hint.hint env.decl_env cty in
    Phase.localize_with_self env cty
  in
  let expected = ExpectedTy.make p Reason.URhint cty in
  match e with
  | Some e ->
    let (env, te, ty) = Typing.expr ~expected env e in
    let env =
      Typing_coercion.coerce_type
        p
        Reason.URhint
        env
        ty
        (MakeType.unenforced cty)
        Errors.record_init_value_does_not_match_hint
    in
    (env, (id, hint, Some te))
  | None -> (env, (id, hint, None))

let record_def_parent env rd parent_hint =
  match snd parent_hint with
  | Aast.Happly ((parent_pos, parent_name), []) ->
    (match Decl_provider.get_record_def (Env.get_ctx env) parent_name with
    | Some parent_rd ->
      (* We can only inherit from abstract records. *)
      ( if not parent_rd.rdt_abstract then
        let (parent_pos, parent_name) = parent_rd.rdt_name in
        Errors.extend_non_abstract_record
          parent_name
          (fst rd.rd_name)
          parent_pos );

      (* Ensure we aren't defining fields that overlap with
         inherited fields. *)
      let inherited_fields = Typing_helpers.all_record_fields env parent_rd in
      List.iter rd.rd_fields ~f:(fun ((pos, name), _, _) ->
          match SMap.find_opt name inherited_fields with
          | Some ((prev_pos, _), _) ->
            Errors.repeated_record_field name pos prev_pos
          | None -> ())
    | None ->
      (* Something exists with this name (naming succeeded), but it's
         not a record. *)
      Errors.unbound_name parent_pos parent_name Errors.RecordContext)
  | _ ->
    failwith
      "Record parent was not an Happly. This should have been a syntax error."

(* Report an error if we have inheritance cycles in record declarations. *)
let check_record_inheritance_cycle env ((rd_pos, rd_name) : Aast.sid) : unit =
  let rec worker name trace seen =
    match Decl_provider.get_record_def (Env.get_ctx env) name with
    | Some rd ->
      (match rd.rdt_extends with
      | Some (_, parent_name) when String.equal parent_name rd_name ->
        (* This record is in an inheritance cycle.*)
        Errors.cyclic_record_def trace rd_pos
      | Some (_, parent_name) when SSet.mem parent_name seen ->
        (* There's an inheritance cycle higher in the chain. *)
        ()
      | Some (_, parent_name) ->
        worker parent_name (parent_name :: trace) (SSet.add parent_name seen)
      | None -> ())
    | None -> ()
  in
  worker rd_name [rd_name] (SSet.singleton rd_name)

let record_def_def ctx rd =
  Counters.count_typecheck @@ fun () ->
  let env = EnvFromDef.record_def_env ctx rd in
  (match rd.rd_extends with
  | Some parent -> record_def_parent env rd parent
  | None -> ());

  check_record_inheritance_cycle env rd.rd_name;

  let (env, attributes) =
    List.map_env env rd.rd_user_attributes Typing.user_attribute
  in
  let (_env, fields) = List.map_env env rd.rd_fields record_field in
  {
    Aast.rd_annotation = Env.save (Env.get_tpenv env) env;
    Aast.rd_name = rd.rd_name;
    Aast.rd_extends = rd.rd_extends;
    Aast.rd_abstract = rd.rd_abstract;
    Aast.rd_fields = fields;
    Aast.rd_user_attributes = attributes;
    Aast.rd_namespace = rd.rd_namespace;
    Aast.rd_span = rd.rd_span;
    Aast.rd_doc_comment = rd.rd_doc_comment;
    Aast.rd_emit_id = rd.rd_emit_id;
  }

let nast_to_tast_gienv ~(do_tast_checks : bool) ctx nast :
    _ * Typing_inference_env.t_global_with_pos list =
  let convert_def = function
    (* Sometimes typing will just return `None` but that should only be the case
     * if an error had already been registered e.g. in naming
     *)
    | Fun f ->
      begin
        match fun_def ctx f with
        | Some (f, env) -> Some (Aast.Fun f, [env])
        | None -> None
      end
    | Constant gc -> Some (Aast.Constant (gconst_def ctx gc), [])
    | Typedef td -> Some (Aast.Typedef (Typing.typedef_def ctx td), [])
    | Class c ->
      begin
        match class_def ctx c with
        | Some (c, envs) -> Some (Aast.Class c, envs)
        | None -> None
      end
    | RecordDef rd -> Some (Aast.RecordDef (record_def_def ctx rd), [])
    (* We don't typecheck top level statements:
     * https://docs.hhvm.com/hack/unsupported/top-level
     * so just create the minimal env for us to construct a Stmt.
     *)
    | Stmt s ->
      let env = Env.empty ctx Relative_path.default None in
      Some (Aast.Stmt (snd (Typing.stmt env s)), [])
    | Namespace _
    | NamespaceUse _
    | SetNamespaceEnv _
    | FileAttributes _ ->
      failwith
        "Invalid nodes in NAST. These nodes should be removed during naming."
  in
  Nast_check.program ctx nast;
  let (tast, envs) = List.unzip @@ List.filter_map nast convert_def in
  let envs = List.concat envs in
  if do_tast_checks then Tast_check.program ctx tast;
  (tast, envs)

let nast_to_tast ~do_tast_checks ctx nast =
  let (tast, _gienvs) = nast_to_tast_gienv ~do_tast_checks ctx nast in
  tast

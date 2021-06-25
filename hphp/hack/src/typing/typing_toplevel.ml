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
module FunUtils = Decl_fun_utils
module Reason = Typing_reason
module Env = Typing_env
module MakeType = Typing_make_type
module Type = Typing_ops
module Phase = Typing_phase
module Subst = Decl_subst
module EnvFromDef = Typing_env_from_def
module Partial = Partial_provider
module TUtils = Typing_utils
module TCO = TypecheckerOptions
module Cls = Decl_provider.Class
module SN = Naming_special_names

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

let get_callable_variadicity ~partial_callback ~pos env variadicity_decl_ty =
  function
  | FVvariadicArg vparam ->
    let (env, ty) = make_param_local_ty env variadicity_decl_ty vparam in
    check_param_has_hint env vparam ty partial_callback;
    let (env, t_variadic) = Typing.bind_param env (ty, vparam) in
    (env, Aast.FVvariadicArg t_variadic)
  | FVellipsis p ->
    if Partial.should_check_error (Env.get_mode env) 4223 then
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

(* Checking this with List.exists will be a single op in the vast majority of cases (empty) *)
let get_ctx_vars ctxs =
  Option.value_map
    ~f:(fun (_, cs) ->
      List.filter_map cs ~f:(function
          | (_, Haccess ((_, Hvar n), _)) -> Some n
          | _ -> None))
    ~default:[]
    ctxs

let function_dynamically_callable
    env f params_decl_ty variadicity_decl_ty ret_locl_ty =
  let env = { env with in_support_dynamic_type_method_check = true } in
  let interface_check =
    Typing_dynamic.sound_dynamic_interface_check
      env
      (variadicity_decl_ty :: params_decl_ty)
      ret_locl_ty
  in
  let function_body_check () =
    (* Here the body of the function is typechecked again to ensure it is safe
     * to call it from a dynamic context (eg. under dyn..dyn->dyn assumptions).
     * The code below must be kept in sync with with the fun_def checks.
     *)
    let make_dynamic pos =
      Typing_make_type.dynamic (Reason.Rsupport_dynamic_type pos)
    in
    let dynamic_return_ty = make_dynamic (get_pos ret_locl_ty) in
    let dynamic_return_info =
      Typing_env_return_info.
        {
          return_type = MakeType.unenforced dynamic_return_ty;
          return_disposable = false;
          return_explicit = true;
          return_dynamically_callable = true;
        }
    in
    let (env, param_tys) =
      List.zip_exn f.f_params params_decl_ty
      |> List.map_env env ~f:(fun env (param, hint) ->
             let dyn_ty =
               make_dynamic @@ Pos_or_decl.of_raw_pos param.param_pos
             in
             let ty =
               match hint with
               | Some ty when Typing_enforceability.is_enforceable env ty ->
                 Typing_make_type.intersection
                   (Reason.Rsupport_dynamic_type Pos_or_decl.none)
                   [ty; dyn_ty]
               | _ -> dyn_ty
             in
             make_param_local_ty env (Some ty) param)
    in
    let params_need_immutable = get_ctx_vars f.f_ctxs in
    let (env, _) =
      (* In this pass, bind_param_and_check receives a pair where the lhs is
       * either Tdynamic or TInstersection of the original type and TDynamic,
       * but the fun_param is still referencing the source hint. We amend
       * the source hint to keep in in sync before calling bind_param
       * so the right enforcement is computed.
       *)
      let bind_param_and_check env lty_and_param =
        let (ty, param) = lty_and_param in
        let name = param.param_name in
        let (hi, hopt) = param.param_type_hint in
        let hopt =
          Option.map hopt ~f:(fun (p, h) ->
              if Typing_utils.is_tintersection env ty then
                (p, Hintersection [(p, h); (p, Hdynamic)])
              else
                (p, Hdynamic))
        in
        let param_type_hint = (hi, hopt) in
        let param = (ty, { param with param_type_hint }) in
        let immutable =
          List.exists ~f:(String.equal name) params_need_immutable
        in
        let (env, fun_param) = Typing.bind_param ~immutable env param in
        (env, fun_param)
      in
      List.map_env
        env
        (List.zip_exn param_tys f.f_params)
        ~f:bind_param_and_check
    in

    let pos = fst f.f_name in
    let (env, t_variadic) =
      get_callable_variadicity
        ~partial_callback:(Partial.should_check_error (Env.get_mode env))
        ~pos
        env
        (Some (make_dynamic @@ Pos_or_decl.of_raw_pos pos))
        f.f_variadic
    in
    let env =
      set_tyvars_variance_in_callable env dynamic_return_ty param_tys t_variadic
    in
    let disable =
      Naming_attributes.mem
        SN.UserAttributes.uaDisableTypecheckerInternal
        f.f_user_attributes
    in

    Errors.try_
      (fun () ->
        let (_ : env * Tast.stmt list) =
          Typing.fun_ ~disable env dynamic_return_info pos f.f_body f.f_fun_kind
        in
        ())
      (fun error ->
        Errors.function_is_not_dynamically_callable pos (snd f.f_name) error)
  in
  if not interface_check then function_body_check ()

let fun_def ctx fd :
    (Tast.fun_def * Typing_inference_env.t_global_with_pos) option =
  let f = fd.fd_fun in
  Counters.count Counters.Category.Typecheck @@ fun () ->
  Errors.run_with_span f.f_span @@ fun () ->
  let env = EnvFromDef.fun_env ~origin:Decl_counters.TopLevel ctx fd in
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
      let (env, file_attrs) =
        Typing.file_attributes env fd.fd_file_attributes
      in
      let (env, cap_ty, unsafe_cap_ty) =
        Typing.type_capability env f.f_ctxs f.f_unsafe_ctxs (fst f.f_name)
      in
      let env =
        Env.set_module
          env
          (Naming_attributes_params.get_module_attribute f.f_user_attributes)
      in
      let env =
        Env.set_internal
          env
          (Naming_attributes.mem
             SN.UserAttributes.uaInternal
             f.f_user_attributes)
      in
      Typing_type_wellformedness.fun_ env f;
      let env =
        Phase.localize_and_add_ast_generic_parameters_and_where_constraints
          env
          ~ignore_errors:false
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
          let localize env ty =
            Phase.localize_no_subst env ~ignore_errors:false ty
          in
          Typing_return.make_return_type localize env ty
      in
      let ret_pos =
        match snd f.f_ret with
        | Some (ret_pos, _) -> ret_pos
        | None -> fst f.f_name
      in
      let (env, return_ty) =
        Typing_return.force_return_kind env ret_pos return_ty
      in
      let return =
        Typing_return.make_info
          ret_pos
          f.f_fun_kind
          f.f_user_attributes
          env
          ~is_explicit:(Option.is_some (hint_of_type_hint f.f_ret))
          return_ty
          return_decl_ty
      in
      let sound_dynamic_check_saved_env = env in
      let (env, param_tys) =
        List.zip_exn f.f_params params_decl_ty
        |> List.map_env env ~f:(fun env (param, hint) ->
               make_param_local_ty env hint param)
      in
      let partial_callback = Partial.should_check_error (Env.get_mode env) in
      let check_has_hint p t = check_param_has_hint env p t partial_callback in
      List.iter2_exn ~f:check_has_hint f.f_params param_tys;
      let params_need_immutable = get_ctx_vars f.f_ctxs in
      let (env, typed_params) =
        let bind_param_and_check env param =
          let name = (snd param).param_name in
          let immutable =
            List.exists ~f:(String.equal name) params_need_immutable
          in
          let (env, fun_param) = Typing.bind_param ~immutable env param in
          (env, fun_param)
        in
        List.map_env
          env
          (List.zip_exn param_tys f.f_params)
          ~f:bind_param_and_check
      in
      let (env, t_variadic) =
        get_callable_variadicity
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
      let (env, _) =
        Typing_coeffects.register_capabilities env cap_ty unsafe_cap_ty
      in
      Typing_memoize.check_function env f;
      let (env, tb) =
        Typing.fun_ ~disable env return pos f.f_body f.f_fun_kind
      in
      begin
        match hint_of_type_hint f.f_ret with
        | None ->
          if partial_callback 4030 then Errors.expecting_return_type_hint pos
        | Some _ -> ()
      end;
      let (env, tparams) = List.map_env env f.f_tparams ~f:Typing.type_param in
      let (env, user_attributes) =
        List.map_env env f.f_user_attributes ~f:Typing.user_attribute
      in
      let env = Typing_solver.close_tyvars_and_solve env in
      let env = Typing_solver.solve_all_unsolved_tyvars env in

      let check_support_dynamic_type =
        Naming_attributes.mem
          SN.UserAttributes.uaSupportDynamicType
          f.f_user_attributes
      in
      if
        TypecheckerOptions.enable_sound_dynamic
          (Provider_context.get_tcopt (Env.get_ctx env))
        && check_support_dynamic_type
      then
        function_dynamically_callable
          sound_dynamic_check_saved_env
          f
          params_decl_ty
          variadicity_decl_ty
          return_ty;

      let fun_ =
        {
          Aast.f_annotation = Env.save local_tpenv env;
          Aast.f_readonly_this = f.f_readonly_this;
          Aast.f_span = f.f_span;
          Aast.f_readonly_ret = f.f_readonly_ret;
          Aast.f_ret = (return_ty, hint_of_type_hint f.f_ret);
          Aast.f_name = f.f_name;
          Aast.f_tparams = tparams;
          Aast.f_where_constraints = f.f_where_constraints;
          Aast.f_variadic = t_variadic;
          Aast.f_params = typed_params;
          Aast.f_ctxs = f.f_ctxs;
          Aast.f_unsafe_ctxs = f.f_unsafe_ctxs;
          Aast.f_fun_kind = f.f_fun_kind;
          Aast.f_user_attributes = user_attributes;
          Aast.f_body = { Aast.fb_ast = tb; fb_annotation = () };
          Aast.f_external = f.f_external;
          Aast.f_doc_comment = f.f_doc_comment;
        }
      in
      let fundef =
        {
          Aast.fd_mode = fd.fd_mode;
          Aast.fd_fun = fun_;
          Aast.fd_file_attributes = file_attrs;
          Aast.fd_namespace = fd.fd_namespace;
        }
      in
      let (_env, global_inference_env) = Env.extract_global_inference_env env in
      (fundef, (pos, global_inference_env)))

let method_dynamically_callable
    env cls m params_decl_ty variadicity_decl_ty ret_locl_ty =
  let env = { env with in_support_dynamic_type_method_check = true } in
  (* Add `dynamic` lower and upper bound to any type parameters that are not marked <<__NoRequireDynamic>> *)
  let env_with_require_dynamic =
    Typing_dynamic.add_require_dynamic_bounds env cls
  in
  let interface_check =
    Typing_dynamic.sound_dynamic_interface_check
      env_with_require_dynamic
      (variadicity_decl_ty :: params_decl_ty)
      ret_locl_ty
  in
  let method_body_check () =
    (* Here the body of the method is typechecked again to ensure it is safe
     * to call it from a dynamic context (eg. under dyn..dyn->dyn assumptions).
     * The code below must be kept in sync with with the method_def checks.
     *)
    let make_dynamic pos =
      Typing_make_type.dynamic (Reason.Rsupport_dynamic_type pos)
    in
    let dynamic_return_ty = make_dynamic (get_pos ret_locl_ty) in
    let dynamic_return_info =
      Typing_env_return_info.
        {
          return_type = MakeType.unenforced dynamic_return_ty;
          return_disposable = false;
          return_explicit = true;
          return_dynamically_callable = true;
        }
    in
    let (env, param_tys) =
      List.zip_exn m.m_params params_decl_ty
      |> List.map_env env ~f:(fun env (param, hint) ->
             let dyn_ty =
               make_dynamic @@ Pos_or_decl.of_raw_pos param.param_pos
             in
             let ty =
               match hint with
               | Some ty when Typing_enforceability.is_enforceable env ty ->
                 Typing_make_type.intersection
                   (Reason.Rsupport_dynamic_type Pos_or_decl.none)
                   [ty; dyn_ty]
               | _ -> dyn_ty
             in
             make_param_local_ty env (Some ty) param)
    in
    let params_need_immutable = get_ctx_vars m.m_ctxs in
    let (env, _) =
      (* In this pass, bind_param_and_check receives a pair where the lhs is
       * either Tdynamic or TInstersection of the original type and TDynamic,
       * but the fun_param is still referencing the source hint. We amend
       * the source hint to keep in in sync before calling bind_param
       * so the right enforcement is computed.
       *)
      let bind_param_and_check env lty_and_param =
        let (ty, param) = lty_and_param in
        let name = param.param_name in
        let (hi, hopt) = param.param_type_hint in
        let hopt =
          Option.map hopt ~f:(fun (p, h) ->
              if Typing_utils.is_tintersection env ty then
                (p, Hintersection [(p, h); (p, Hdynamic)])
              else
                (p, Hdynamic))
        in
        let param_type_hint = (hi, hopt) in
        let param = (ty, { param with param_type_hint }) in
        let immutable =
          List.exists ~f:(String.equal name) params_need_immutable
        in
        let (env, fun_param) = Typing.bind_param ~immutable env param in
        (env, fun_param)
      in
      List.map_env
        env
        (List.zip_exn param_tys m.m_params)
        ~f:bind_param_and_check
    in

    let pos = fst m.m_name in
    let (env, t_variadic) =
      get_callable_variadicity
        ~partial_callback:(Partial.should_check_error (Env.get_mode env))
        ~pos
        env
        (Some (make_dynamic @@ Pos_or_decl.of_raw_pos pos))
        m.m_variadic
    in
    let env =
      set_tyvars_variance_in_callable env dynamic_return_ty param_tys t_variadic
    in

    let env =
      if Cls.get_support_dynamic_type cls then
        let this_ty =
          Typing_make_type.intersection
            (Reason.Rsupport_dynamic_type Pos_or_decl.none)
            [Env.get_local env this; make_dynamic Pos_or_decl.none]
        in
        Env.set_local env this this_ty Pos.none
      else
        env
    in

    let disable =
      Naming_attributes.mem
        SN.UserAttributes.uaDisableTypecheckerInternal
        m.m_user_attributes
    in

    Errors.try_
      (fun () ->
        let (_ : env * Tast.stmt list) =
          Typing.fun_
            ~abstract:m.m_abstract
            ~disable
            env
            dynamic_return_info
            pos
            m.m_body
            m.m_fun_kind
        in
        ())
      (fun error ->
        Errors.method_is_not_dynamically_callable
          pos
          (snd m.m_name)
          (Cls.name cls)
          ( Naming_attributes.mem
              SN.UserAttributes.uaSupportDynamicType
              m.m_user_attributes
          || Cls.get_support_dynamic_type cls )
          None
          (Some error))
  in
  if not interface_check then method_body_check ()

let method_def env cls m =
  Errors.run_with_span m.m_span @@ fun () ->
  with_timeout env m.m_name ~do_:(fun env ->
      FunUtils.check_params m.m_params;
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
      let (env, cap_ty, unsafe_cap_ty) =
        Typing.type_capability env m.m_ctxs m.m_unsafe_ctxs (fst m.m_name)
      in
      let (env, _) =
        Typing_coeffects.register_capabilities env cap_ty unsafe_cap_ty
      in
      let is_ctor = String.equal (snd m.m_name) SN.Members.__construct in
      let env = Env.set_fun_is_constructor env is_ctor in
      let env =
        Phase.localize_and_add_ast_generic_parameters_and_where_constraints
          env
          ~ignore_errors:false
          m.m_tparams
          m.m_where_constraints
      in
      let env =
        match Env.get_self_ty env with
        | Some ty when not (Env.is_static env) ->
          Env.set_local env this (MakeType.this (get_reason ty)) Pos.none
        | _ -> env
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
            empty_expand_env_with_on_error
              (Env.invalid_type_hint_assert_primary_pos_in_current_decl env)
          in
          Typing_return.make_return_type (Phase.localize ~ety_env) env ret
      in
      let ret_pos =
        match snd m.m_ret with
        | Some (ret_pos, _) -> ret_pos
        | None -> fst m.m_name
      in
      let (env, locl_ty) =
        Typing_return.force_return_kind env ret_pos locl_ty
      in
      let return =
        Typing_return.make_info
          ret_pos
          m.m_fun_kind
          m.m_user_attributes
          env
          ~is_explicit:(Option.is_some (hint_of_type_hint m.m_ret))
          locl_ty
          ret_decl_ty
      in
      let sound_dynamic_check_saved_env = env in
      let (env, param_tys) =
        List.zip_exn m.m_params params_decl_ty
        |> List.map_env env ~f:(fun env (param, hint) ->
               make_param_local_ty env hint param)
      in
      let partial_callback = Partial.should_check_error (Env.get_mode env) in
      let param_fn p t = check_param_has_hint env p t partial_callback in
      List.iter2_exn ~f:param_fn m.m_params param_tys;
      Typing_memoize.check_method env m;
      let params_need_immutable = get_ctx_vars m.m_ctxs in
      let (env, typed_params) =
        let bind_param_and_check env param =
          let name = (snd param).param_name in
          let immutable =
            List.exists ~f:(String.equal name) params_need_immutable
          in
          let (env, fun_param) = Typing.bind_param ~immutable env param in
          (env, fun_param)
        in
        List.map_env
          env
          (List.zip_exn param_tys m.m_params)
          ~f:bind_param_and_check
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
      let type_hint' =
        match hint_of_type_hint m.m_ret with
        | None when String.equal (snd m.m_name) SN.Members.__construct ->
          Some (pos, Hprim Tvoid)
        | None ->
          if partial_callback 4030 then Errors.expecting_return_type_hint pos;
          None
        | Some _ -> hint_of_type_hint m.m_ret
      in
      let m = { m with m_ret = (fst m.m_ret, type_hint') } in
      let (env, tparams) = List.map_env env m.m_tparams ~f:Typing.type_param in
      let (env, user_attributes) =
        List.map_env env m.m_user_attributes ~f:Typing.user_attribute
      in
      let env = Typing_solver.close_tyvars_and_solve env in
      let env = Typing_solver.solve_all_unsolved_tyvars env in

      (* if the enclosing class implements dynamic, or the method is annotated with
       * <<__SupportDynamicType>>, check that the method is dynamically callable *)
      let check_support_dynamic_type =
        (not env.inside_constructor)
        && ( Cls.get_support_dynamic_type cls
             && not (Aast.equal_visibility m.m_visibility Private)
           || Naming_attributes.mem
                SN.UserAttributes.uaSupportDynamicType
                m.m_user_attributes )
      in
      if
        TypecheckerOptions.enable_sound_dynamic
          (Provider_context.get_tcopt (Env.get_ctx env))
        && check_support_dynamic_type
      then
        method_dynamically_callable
          sound_dynamic_check_saved_env
          cls
          m
          params_decl_ty
          variadicity_decl_ty
          locl_ty;
      let method_def =
        {
          Aast.m_annotation = Env.save local_tpenv env;
          Aast.m_span = m.m_span;
          Aast.m_final = m.m_final;
          Aast.m_static = m.m_static;
          Aast.m_abstract = m.m_abstract;
          Aast.m_visibility = m.m_visibility;
          Aast.m_readonly_this = m.m_readonly_this;
          Aast.m_name = m.m_name;
          Aast.m_tparams = tparams;
          Aast.m_where_constraints = m.m_where_constraints;
          Aast.m_variadic = t_variadic;
          Aast.m_params = typed_params;
          Aast.m_ctxs = m.m_ctxs;
          Aast.m_unsafe_ctxs = m.m_unsafe_ctxs;
          Aast.m_fun_kind = m.m_fun_kind;
          Aast.m_user_attributes = user_attributes;
          Aast.m_readonly_ret = m.m_readonly_ret;
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
let check_parent env class_def class_type =
  match Env.get_parent_class env with
  | Some parent_type ->
    let position = fst class_def.c_name in
    if Cls.const class_type && not (Cls.const parent_type) then
      Errors.self_const_parent_not position;
    if Cls.final parent_type then
      Errors.extend_final position (Cls.pos parent_type) (Cls.name parent_type)
  | None -> ()

let sealed_subtype ctx (c : Nast.class_) ~is_enum =
  let parent_name = snd c.c_name in
  let is_sealed (attr : Nast.user_attribute) =
    String.equal (snd attr.ua_name) SN.UserAttributes.uaSealed
  in
  match List.find c.c_user_attributes ~f:is_sealed with
  | None -> ()
  | Some sealed_attr ->
    let iter_item (param : Nast.expr) =
      match snd param with
      | Class_const (cid, _) ->
        let klass_name = Nast.class_id_to_str (snd cid) in
        let klass = Decl_provider.get_class ctx klass_name in
        (match klass with
        | None -> ()
        | Some decl ->
          let includes_ancestor =
            if is_enum then
              match Cls.enum_type decl with
              | None -> true
              | Some enum_ty ->
                let check x =
                  match get_node x with
                  | Tapply ((_, name), _) -> String.equal name parent_name
                  | _ -> true
                in
                List.exists enum_ty.te_includes ~f:check
            else
              Cls.has_ancestor decl parent_name
          in
          if not includes_ancestor then
            let parent_pos = fst param in
            let child_pos = Cls.pos decl in
            let child_name = Cls.name decl in
            let (child_kind, verb) =
              match Cls.kind decl with
              | Ast_defs.Cabstract
              | Ast_defs.Cnormal ->
                ("Class", "extend")
              | Ast_defs.Cinterface -> ("Interface", "implement")
              | Ast_defs.Ctrait -> ("Trait", "use")
              | Ast_defs.Cenum -> ("Enum", "use")
            in
            Errors.sealed_not_subtype
              verb
              parent_pos
              child_pos
              parent_name
              child_name
              child_kind)
      (* unit below is fine because error cases are handled as Parsing[1002] *)
      | _ -> ()
    in
    List.iter sealed_attr.ua_params ~f:iter_item

let check_parent_sealed
    ~(is_enum_class : bool) (child_pos, child_type) parent_type =
  match Cls.sealed_whitelist parent_type with
  | None -> ()
  | Some whitelist ->
    let parent_pos = Cls.pos parent_type in
    let parent_name = Cls.name parent_type in
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
      | (Ast_defs.Cenum, _) when is_enum_class -> check "enum class" "extend"
      | (Ast_defs.Cenum, _) -> check "enum" "use"
    end

let check_parents_sealed env child_def child_type =
  let parents =
    match child_def.c_enum with
    | Some enum -> enum.e_includes
    | None -> child_def.c_extends
  in
  let parents = parents @ child_def.c_implements @ child_def.c_uses in
  let is_enum_class = Aast.is_enum_class child_def in
  List.iter parents ~f:(function
      | (_, Happly ((_, name), _)) ->
        begin
          match Env.get_class_dep env name with
          | Some parent_type ->
            check_parent_sealed
              ~is_enum_class
              (fst child_def.c_name, child_type)
              parent_type
          | None -> ()
        end
      | _ -> ())

(* Reject multiple instantiations of the same generic interface
 * in extends and implements clauses.
 * e.g. disallow class C implements I<string>, I<int>
 *
 * O(n^2) but we don't expect number of instantiated interfaces to be large
 *)
let rec check_implements_or_extends_unique impl =
  match impl with
  | [] -> ()
  | (hint, ty) :: rest ->
    (match get_node ty with
    | Tapply ((_, name), _ :: _) ->
      let (pos_list, rest) =
        List.partition_map rest ~f:(fun (h, ty) ->
            match get_node ty with
            | Tapply ((pos', name'), _) when String.equal name name' ->
              First pos'
            | _ -> Second (h, ty))
      in
      if not (List.is_empty pos_list) then
        Errors.duplicate_interface (fst hint) name pos_list;
      check_implements_or_extends_unique rest
    | _ -> check_implements_or_extends_unique rest)

let check_cstr_dep env deps =
  List.iter deps ~f:(fun ((p, _dep_hint), dep) ->
      match get_node dep with
      | Tapply ((_, class_name), _) ->
        Env.make_depend_on_constructor env class_name
      | Tgeneric _ ->
        Errors.expected_class ~suffix:" or interface but got a generic" p
      | _ -> Errors.expected_class ~suffix:" or interface" p)

let check_const_trait_members pos env use_list =
  let (_, trait, _) = Decl_utils.unwrap_class_hint use_list in
  match Env.get_class env trait with
  | Some c when Ast_defs.(equal_class_kind (Cls.kind c) Ctrait) ->
    List.iter (Cls.props c) ~f:(fun (x, ce) ->
        if not (get_ce_const ce) then Errors.trait_prop_const_class pos x)
  | _ -> ()

let check_consistent_enum_inclusion
    included_cls ((dest_cls_pos, dest_cls) : Pos.t * Cls.t) =
  match (Cls.enum_type included_cls, Cls.enum_type dest_cls) with
  | (Some included_e, Some dest_e) ->
    (* ensure that the base types are identical *)
    if not (Typing_defs.equal_decl_ty included_e.te_base dest_e.te_base) then
      Errors.incompatible_enum_inclusion_base
        dest_cls_pos
        (Cls.name dest_cls)
        (Cls.name included_cls);
    (* ensure that the visibility constraint are compatible *)
    (match (included_e.te_constraint, dest_e.te_constraint) with
    | (None, Some _) ->
      Errors.incompatible_enum_inclusion_constraint
        dest_cls_pos
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
        ~child_pos:dest_cls_pos
        ~child_kind:Ast_defs.Cenum
        ~child_name:(Cls.name dest_cls)
        ~child_is_enum_class:false
  | (None, _) ->
    Errors.enum_inclusion_not_enum
      dest_cls_pos
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
        | Some cls -> check_consistent_enum_inclusion ie_cls (ie_pos, cls));
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
              else if String.( <> ) origin_class_name class_const.cc_origin then
                (* check for diamond inclusion, if not raise an error about multiple inherit *)
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
  let (env, tparam_list) = List.map_env env ct ~f:Typing.type_param in
  (env, tparam_list)

(** Checks that a dynamic element is also dynamic in the parents. *)
let check_dynamic_class_element get_static_elt element_name dyn_pos ~elt_type =
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
let check_static_class_element get_dyn_elt element_name static_pos ~elt_type =
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
let check_extend_abstract_meth ~is_final p seq =
  List.iter seq ~f:(fun (x, ce) ->
      match ce.ce_type with
      | (lazy ty) when get_ce_abstract ce && is_fun ty ->
        Errors.implement_abstract ~is_final p (get_pos ty) "method" x
      | _ -> ())

let check_extend_abstract_prop ~is_final p seq =
  List.iter seq ~f:(fun (x, ce) ->
      if get_ce_abstract ce then
        let ce_pos = Lazy.force ce.ce_type |> get_pos in
        Errors.implement_abstract ~is_final p ce_pos "property" x)

(* Type constants must be bound to a concrete type for non-abstract classes.
 *)
let check_extend_abstract_typeconst ~is_final p seq =
  List.iter seq ~f:(fun (x, tc) ->
      match tc.ttc_kind with
      | TCAbstract _ ->
        Errors.implement_abstract
          ~is_final
          p
          (fst tc.ttc_name)
          "type constant"
          x
      | _ -> ())

let check_extend_abstract_const ~is_final p seq =
  List.iter seq ~f:(fun (x, cc) ->
      match cc.cc_abstract with
      | CCAbstract _ when not cc.cc_synthesized ->
        let cc_pos = get_pos cc.cc_type in
        Errors.implement_abstract ~is_final p cc_pos "constant" x
      | _ -> ())

exception Found of Pos_or_decl.t

let contains_generic : Typing_defs.decl_ty -> Pos_or_decl.t option =
 fun ty ->
  let visitor =
    object
      inherit [_] Type_visitor.decl_type_visitor as super

      method! on_type env ty =
        match get_node ty with
        | Tgeneric _ -> raise (Found (get_pos ty))
        | _ -> super#on_type env ty
    end
  in
  try
    visitor#on_type () ty;
    None
  with Found p -> Some p

let check_no_generic_static_property env tc =
  if
    (* Check whether the type of a static property (class variable) contains
     * any generic type parameters. Outside of traits, this is illegal as static
     * properties are shared across all generic instantiations.
     * Although not strictly speaking a variance check, it fits here because
     * it concerns the presence of generic type parameters in types.
     *)
    Ast_defs.(equal_class_kind (Cls.kind tc) Ctrait)
  then
    ()
  else
    Cls.sprops tc
    |> List.iter ~f:(fun (_prop_name, prop) ->
           let (lazy ty) = prop.ce_type in
           let var_type_pos = get_pos ty in
           let class_pos = Cls.pos tc in
           match contains_generic ty with
           | None -> ()
           | Some generic_pos ->
             Option.iter
               (* If the static property is inherited from another trait, the position may be
                * in a different file. *)
               (Env.fill_in_pos_filename_if_in_current_decl env generic_pos)
               ~f:(fun generic_pos ->
                 Errors.static_property_type_generic_param
                   ~class_pos
                   ~var_type_pos
                   ~generic_pos))

let get_decl_prop_ty env cls ~is_static prop_id =
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

let typeconst_def
    cls
    env
    {
      c_tconst_name = (pos, _) as id;
      c_tconst_kind;
      c_tconst_user_attributes;
      c_tconst_span;
      c_tconst_doc_comment;
      c_tconst_is_ctx;
    } =
  if Ast_defs.is_c_enum cls.c_kind then
    Errors.cannot_declare_constant `enum pos cls.c_name;

  let name = snd cls.c_name ^ "::" ^ snd id in
  (* Check constraints and report cycles through the definition *)
  let env =
    match c_tconst_kind with
    | TCAbstract
        { c_atc_as_constraint; c_atc_super_constraint; c_atc_default = Some ty }
      ->
      let (env, ty) =
        Phase.localize_hint_no_subst
          ~ignore_errors:false
          ~report_cycle:(pos, name)
          env
          ty
      in
      let env =
        match c_atc_as_constraint with
        | Some as_ ->
          let (env, as_) =
            Phase.localize_hint_no_subst ~ignore_errors:false env as_
          in
          Type.sub_type
            pos
            Reason.URtypeconst_cstr
            env
            ty
            as_
            Errors.unify_error
        | None -> env
      in
      let env =
        match c_atc_super_constraint with
        | Some super ->
          let (env, super) =
            Phase.localize_hint_no_subst ~ignore_errors:false env super
          in
          Type.sub_type
            pos
            Reason.URtypeconst_cstr
            env
            super
            ty
            Errors.unify_error
        | None -> env
      in
      env
    | TCPartiallyAbstract { c_patc_constraint = cstr; c_patc_type = ty } ->
      let (env, cstr) =
        Phase.localize_hint_no_subst ~ignore_errors:false env cstr
      in
      let (env, ty) =
        Phase.localize_hint_no_subst
          ~ignore_errors:false
          ~report_cycle:(pos, name)
          env
          ty
      in
      Type.sub_type pos Reason.URtypeconst_cstr env ty cstr Errors.unify_error
    | TCConcrete { c_tc_type = ty } ->
      let (env, _ty) =
        Phase.localize_hint_no_subst
          ~ignore_errors:false
          ~report_cycle:(pos, name)
          env
          ty
      in
      env
    | _ -> env
  in

  (* TODO(T88552052): should this check be happening for defaults
   * Does this belong here at all? *)
  let env =
    match c_tconst_kind with
    | TCConcrete { c_tc_type = (pos, Hshape { nsi_field_map; _ }) }
    | TCPartiallyAbstract
        { c_patc_type = (pos, Hshape { nsi_field_map; _ }); _ }
    | TCAbstract { c_atc_default = Some (pos, Hshape { nsi_field_map; _ }); _ }
      ->
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
    List.map_env env c_tconst_user_attributes ~f:Typing.user_attribute
  in
  ( env,
    {
      Aast.c_tconst_name = id;
      Aast.c_tconst_kind;
      Aast.c_tconst_user_attributes = user_attributes;
      Aast.c_tconst_span;
      Aast.c_tconst_doc_comment;
      Aast.c_tconst_is_ctx;
    } )

(* This should agree with the set of expressions whose type can be inferred in
 * Decl_utils.infer_const
 *)
let is_literal_expr e =
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

let class_const_def ~in_enum_class c env cc =
  let { cc_type = h; cc_id = id; cc_kind = k; _ } = cc in
  let (env, hint_ty, opt_expected) =
    match h with
    | None ->
      begin
        match k with
        | CCAbstract None -> ()
        | CCAbstract (Some e (* default *))
        | CCConcrete e ->
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
      let (env, ty) =
        Phase.localize_possibly_enforced_no_subst env ~ignore_errors:false ty
      in
      (* Removing the HH\MemberOf wrapper in case of enum classes so the
       * following call to expr_* has the right expected type
       *)
      let opt_ty =
        if in_enum_class then
          match get_node ty.et_type with
          | Tnewtype (memberof, [_; et_type], _)
            when String.equal memberof SN.Classes.cMemberOf ->
            { ty with et_type }
          | _ -> ty
        else
          ty
      in
      ( env,
        ty,
        Some (ExpectedTy.make_and_allow_coercion (fst id) Reason.URhint opt_ty)
      )
  in
  let check env ty' =
    (* Lifted out to closure to illustrate which variables are captured *)
    Typing_coercion.coerce_type
      (fst id)
      Reason.URhint
      env
      ty'
      hint_ty
      Errors.class_constant_value_does_not_match_hint
  in
  let type_and_check env e =
    let (env, (te, ty')) =
      Typing.(
        expr_with_pure_coeffects ?expected:opt_expected env e |> triple_to_pair)
    in
    let env = check env ty' in
    (env, te, ty')
  in
  let (env, kind, ty) =
    match k with
    | CCConcrete ((e_pos, _) as e) when in_enum_class ->
      let (env, cap, unsafe_cap) =
        (* Enum class constant initializers are restricted to be `write_props` *)
        let make_hint pos s = (pos, Aast.Happly ((pos, s), [])) in
        let enum_class_ctx =
          Some (e_pos, [make_hint e_pos SN.Capabilities.writeProperty])
        in
        Typing.type_capability env enum_class_ctx enum_class_ctx e_pos
      in
      let (env, (te, ty')) =
        Typing.(
          with_special_coeffects env cap unsafe_cap @@ fun env ->
          expr env ?expected:opt_expected e |> triple_to_pair)
      in
      let (te, ty') =
        match deref hint_ty.et_type with
        | (r, Tnewtype (memberof, [enum_name; _], _))
          when String.equal memberof SN.Classes.cMemberOf ->
          let lift r ty = mk (r, Tnewtype (memberof, [enum_name; ty], ty)) in
          let ((p, te_ty), te) = te in
          let te = ((p, lift (get_reason te_ty) te_ty), te) in
          let ty' = lift r ty' in
          (te, ty')
        | _ -> (te, ty')
      in
      let env = check env ty' in
      (env, Aast.CCConcrete te, ty')
    | CCConcrete e ->
      let (env, te, ty') = type_and_check env e in
      (env, Aast.CCConcrete te, ty')
    | CCAbstract (Some default) ->
      let (env, tdefault, ty') = type_and_check env default in
      (env, CCAbstract (Some tdefault), ty')
    | CCAbstract None -> (env, CCAbstract None, hint_ty.et_type)
  in
  ( env,
    ( {
        Aast.cc_type = cc.cc_type;
        Aast.cc_id = cc.cc_id;
        Aast.cc_kind = kind;
        Aast.cc_doc_comment = cc.cc_doc_comment;
      },
      ty ) )

let class_constr_def env cls constructor =
  let env = { env with inside_constructor = true } in
  Option.bind constructor ~f:(method_def env cls)

let class_implements_type env implements c1 (_hint2, ctype2) =
  let params =
    List.map c1.c_tparams ~f:(fun { tp_name = (p, s); _ } ->
        mk
          ( Reason.Rwitness_from_decl (Pos_or_decl.of_raw_pos p),
            Tgeneric (s, []) ))
  in
  let c1_name_pos = fst c1.c_name in
  let r = Reason.Rwitness_from_decl (Pos_or_decl.of_raw_pos c1_name_pos) in
  let ctype1 =
    mk (r, Tapply (Positioned.of_raw_positioned c1.c_name, params))
  in
  Typing_extends.check_implements env implements ctype2 (c1_name_pos, ctype1)

(** Type-check a property declaration, with optional initializer *)
let class_var_def ~is_static cls env cv =
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
        Phase.localize_possibly_enforced_no_subst
          env
          ~ignore_errors:false
          decl_cty
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
      let (env, te, ty) = Typing.expr_with_pure_coeffects env ?expected e in
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
    List.map_env env cv.cv_user_attributes ~f:Typing.user_attribute
  in
  if
    Option.is_none (hint_of_type_hint cv.cv_type)
    && Partial.should_check_error (Env.get_mode env) 2001
  then
    Errors.prop_without_typehint
      (string_of_visibility cv.cv_visibility)
      cv.cv_id;
  let (env, global_inference_env) = Env.extract_global_inference_env env in
  let ((cv_type_ty, _) as cv_type) =
    match expected with
    | Some expected ->
      (expected.ExpectedTy.ty.et_type, hint_of_type_hint cv.cv_type)
    | None -> Tast.dummy_type_hint (hint_of_type_hint cv.cv_type)
  in
  (* if the class implements dynamic, then check that the type of the property
   * is enforceable (for writing) and coerces to dynamic (for reading) *)
  if
    TypecheckerOptions.enable_sound_dynamic
      (Provider_context.get_tcopt (Env.get_ctx env))
    && Cls.get_support_dynamic_type cls
    && not (Aast.equal_visibility cv.cv_visibility Private)
  then begin
    let env_with_require_dynamic =
      Typing_dynamic.add_require_dynamic_bounds env cls
    in
    Option.iter decl_cty ~f:(fun ty ->
        Typing_dynamic.check_property_sound_for_dynamic_write
          ~on_error:Errors.property_is_not_enforceable
          env_with_require_dynamic
          (Cls.name cls)
          cv.cv_id
          ty);
    Typing_dynamic.check_property_sound_for_dynamic_read
      ~on_error:Errors.property_is_not_dynamic
      env_with_require_dynamic
      (Cls.name cls)
      cv.cv_id
      cv_type_ty
  end;
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
        Aast.cv_readonly = cv.cv_readonly;
      },
      (cv.cv_span, global_inference_env) ) )

let class_def_ env c tc =
  let env =
    let kind =
      match c.c_kind with
      | Ast_defs.Cenum ->
        (match c.c_enum with
        | Some enum when enum.e_enum_class -> SN.AttributeKinds.enumcls
        | _ -> SN.AttributeKinds.enum)
      | _ -> SN.AttributeKinds.cls
    in
    Typing.attributes_check_def env kind c.c_user_attributes
  in
  let (env, file_attrs) = Typing.file_attributes env c.c_file_attributes in
  let ctx = Env.get_ctx env in
  if
    (not Ast_defs.(equal_class_kind c.c_kind Ctrait))
    && not (shallow_decl_enabled ctx)
  then (
    (* These checks are only for eager mode. The same checks are performed
     * for shallow mode in Typing_inheritance *)
    let method_pos ~is_static class_id meth_id =
      let get_meth =
        if is_static then
          Decl_store.((get ()).get_static_method)
        else
          Decl_store.((get ()).get_method)
      in
      match get_meth (class_id, meth_id) with
      | Some { fe_pos; _ } -> fe_pos
      | None -> Pos_or_decl.none
    in
    let check_override ~is_static (id, ce) =
      if get_ce_override ce then
        let pos = method_pos ~is_static ce.ce_origin id in
        (* Method is actually defined in this class *)
        if String.equal ce.ce_origin (snd c.c_name) then
          Errors.should_be_override
            pos
            (snd c.c_name)
            id
            ~current_decl_and_file:(Env.get_current_decl_and_file env)
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
              Errors.override_per_trait c.c_name id ce.ce_origin pos
    in

    List.iter (Cls.methods tc) ~f:(check_override ~is_static:false);
    List.iter (Cls.smethods tc) ~f:(check_override ~is_static:true)
  );
  check_enum_includes env c;
  let (pc, c_name) = c.c_name in
  let (req_extends, req_implements) = split_reqs c in
  let hints_and_decl_tys hints =
    List.map hints ~f:(fun hint -> (hint, Decl_hint.hint env.decl_env hint))
  in
  let extends = hints_and_decl_tys c.c_extends in
  let implements = hints_and_decl_tys c.c_implements in
  let uses = hints_and_decl_tys c.c_uses in
  let req_extends = hints_and_decl_tys req_extends in
  let req_implements = hints_and_decl_tys req_implements in
  let additional_parents =
    (* In an abstract class or a trait, we assume the interfaces
       will be implemented in the future, so we take them as
       part of the class (as requested by dependency injection implementers) *)
    match c.c_kind with
    | Ast_defs.Cabstract -> implements
    | Ast_defs.Ctrait -> implements @ req_implements
    | _ -> []
  in
  let check_cstr_dep = check_cstr_dep env in
  check_implements_or_extends_unique implements;
  check_implements_or_extends_unique extends;
  check_cstr_dep extends;
  check_cstr_dep uses;
  check_cstr_dep req_extends;
  check_cstr_dep additional_parents;
  begin
    match c.c_enum with
    | Some e -> check_cstr_dep (hints_and_decl_tys e.e_includes)
    | _ -> ()
  end;
  let env =
    Phase.localize_and_add_ast_generic_parameters_and_where_constraints
      env
      ~ignore_errors:false
      c.c_tparams
      c.c_where_constraints
  in
  let env =
    Phase.check_where_constraints
      ~in_class:true
      ~use_pos:pc
      ~definition_pos:(Pos_or_decl.of_raw_pos pc)
      ~ety_env:
        (empty_expand_env_with_on_error
           (Env.unify_error_assert_primary_pos_in_current_decl env))
      env
      (Cls.where_constraints tc)
  in
  Typing_variance.class_def env c;
  check_no_generic_static_property env tc;
  let check_where_constraints env ((p, _hint), decl_ty) =
    let (env, locl_ty) =
      Phase.localize_no_subst env ~ignore_errors:false decl_ty
    in
    match get_node (TUtils.get_base_type env locl_ty) with
    | Tclass (cls, _, tyl) ->
      (match Env.get_class env (snd cls) with
      | Some cls when not (List.is_empty (Cls.where_constraints cls)) ->
        let tc_tparams = Cls.tparams cls in
        let ety_env =
          {
            (empty_expand_env_with_on_error
               (Env.unify_error_assert_primary_pos_in_current_decl env))
            with
            substs = Subst.make_locl tc_tparams tyl;
          }
        in
        Phase.check_where_constraints
          ~in_class:true
          ~use_pos:pc
          ~definition_pos:(Pos_or_decl.of_raw_pos p)
          ~ety_env
          env
          (Cls.where_constraints cls)
      | _ -> env)
    | _ -> env
  in
  let impl = extends @ implements @ uses in
  let impl =
    if
      TypecheckerOptions.require_extends_implements_ancestors
        (Env.get_tcopt env)
    then
      impl @ req_extends @ req_implements
    else
      impl
  in
  let env = List.fold impl ~init:env ~f:check_where_constraints in
  check_parent env c tc;
  check_parents_sealed env c tc;
  ( if TypecheckerOptions.enforce_sealed_subclasses (Env.get_tcopt env) then
    match c.c_kind with
    | Ast_defs.Cenum
      when TypecheckerOptions.enable_enum_supertyping (Env.get_tcopt env) ->
      sealed_subtype ctx c ~is_enum:true
    | Ast_defs.Cenum -> ()
    | _ -> sealed_subtype ctx c ~is_enum:false );
  let _ =
    if c.c_support_dynamic_type then
      (* Any class that extends a class or implements an interface
       * that declares <<__SupportDynamicType>> must itself declare
       * <<__SupportDynamicType>>. This is checked elsewhere. But if any generic
       * parameters are not marked <<__NoRequireDynamic>> then we must check that the
       * conditional support for dynamic is sound.
       * We require that
       *    If t <: dynamic
       *    and C<T1,..,Tn> extends t
       *    then C<T1,...,Tn> <: dynamic
       *)
      let dynamic_ty =
        MakeType.dynamic (Reason.Rdynamic_coercion (Reason.Rwitness pc))
      in
      let env =
        List.fold (extends @ implements) ~init:env ~f:(fun env (_, ty) ->
            let (env, lty) =
              Phase.localize_no_subst env ~ignore_errors:true ty
            in
            match get_node lty with
            | Tclass ((_, name), _, _) ->
              begin
                match Env.get_class env name with
                | Some c when Cls.get_support_dynamic_type c ->
                  let env_with_assumptions =
                    Typing_subtype.add_constraint
                      env
                      Ast_defs.Constraint_as
                      lty
                      dynamic_ty
                      (Errors.unify_error_at pc)
                  in
                  begin
                    match Env.get_self_ty env with
                    | Some self_ty ->
                      TUtils.sub_type
                        ~coerce:(Some Typing_logic.CoerceToDynamic)
                        env_with_assumptions
                        self_ty
                        dynamic_ty
                        (fun ?code:_ reasons ->
                          let message =
                            Typing_print.full_strip_ns_decl env ty
                            ^ " is subtype of dynamic implies "
                            ^ Typing_print.full_strip_ns env self_ty
                            ^ " is subtype of dynamic"
                          in
                          Errors.bad_conditional_support_dynamic
                            pc
                            ~child:c_name
                            ~parent:name
                            message
                            reasons)
                    | _ -> env
                  end
                | _ -> env
              end
            | _ -> env)
      in
      env
    else
      env
  in

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
  if Cls.const tc then List.iter c.c_uses ~f:(check_const_trait_members pc env);
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
    List.fold ~init:env impl ~f:(fun env ->
        class_implements_type env (List.map implements ~f:snd) c)
  in
  if Cls.is_disposable tc then
    List.iter
      (c.c_extends @ c.c_uses)
      ~f:(Typing_disposable.enforce_is_disposable env);
  let (env, typed_vars_and_global_inference_envs) =
    List.map_env env vars ~f:(class_var_def ~is_static:false tc)
  in
  let (typed_vars, vars_global_inference_envs) =
    List.unzip typed_vars_and_global_inference_envs
  in
  let (typed_methods, methods_global_inference_envs) =
    List.filter_map methods ~f:(method_def env tc) |> List.unzip
  in
  let (env, typed_typeconsts) =
    List.map_env env c.c_typeconsts ~f:(typeconst_def c)
  in
  let in_enum_class = Env.is_enum_class env c_name in
  let (env, consts) =
    List.map_env env c.c_consts ~f:(class_const_def ~in_enum_class c)
  in
  let (typed_consts, const_types) = List.unzip consts in
  let env = Typing_enum.enum_class_check env tc c.c_consts const_types in
  let typed_constructor = class_constr_def env tc constructor in
  let env = Env.set_static env in
  let (env, typed_static_vars_and_global_inference_envs) =
    List.map_env env static_vars ~f:(class_var_def ~is_static:true tc)
  in
  let (typed_static_vars, static_vars_global_inference_envs) =
    List.unzip typed_static_vars_and_global_inference_envs
  in
  let (typed_static_methods, static_methods_global_inference_envs) =
    List.filter_map static_methods ~f:(method_def env tc) |> List.unzip
  in
  let (methods, constr_global_inference_env) =
    match typed_constructor with
    | None -> (typed_static_methods @ typed_methods, [])
    | Some (m, global_inference_env) ->
      ((m :: typed_static_methods) @ typed_methods, [global_inference_env])
  in
  let (env, tparams) = class_type_param env c.c_tparams in
  let (env, user_attributes) =
    List.map_env env c.c_user_attributes ~f:Typing.user_attribute
  in
  let env = Typing_solver.solve_all_unsolved_tyvars env in
  ( if TypecheckerOptions.enable_sound_dynamic (Provider_context.get_tcopt ctx)
  then
    let parent_names =
      List.filter_map
        (c.c_extends @ c.c_uses @ c.c_implements)
        ~f:(function
          | (_, Happly ((_, name), _)) -> Some name
          | _ -> None)
    in
    let error_parent_support_dynamic_type parent f =
      Errors.parent_support_dynamic_type
        (fst c.c_name)
        (snd c.c_name, c.c_kind)
        (Cls.name parent, Cls.kind parent)
        f
    in
    List.iter parent_names ~f:(fun name ->
        match Env.get_class_dep env name with
        | Some parent_type ->
          begin
            match Cls.kind parent_type with
            | Ast_defs.Cnormal
            | Ast_defs.Cabstract
            | Ast_defs.Cinterface ->
              (* ensure that we implement dynamic if we are a subclass/subinterface of a class/interface
               * that implements dynamic.  Upward well-formedness checks are performed in Typing_extends *)
              if
                Cls.get_support_dynamic_type parent_type
                && not c.c_support_dynamic_type
              then
                error_parent_support_dynamic_type
                  parent_type
                  c.c_support_dynamic_type
            | _ -> ()
          end
        | None -> ()) );

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
      Aast.c_support_dynamic_type = c.c_support_dynamic_type;
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
      Aast.c_emit_id = c.c_emit_id;
    },
    methods_global_inference_envs
    @ static_methods_global_inference_envs
    @ constr_global_inference_env
    @ static_vars_global_inference_envs
    @ vars_global_inference_envs )

let class_def ctx c =
  Counters.count Counters.Category.Typecheck @@ fun () ->
  Errors.run_with_span c.c_span @@ fun () ->
  let env = EnvFromDef.class_env ~origin:Decl_counters.TopLevel ctx c in
  let (name_pos, name) = c.c_name in
  let tc = Env.get_class env name in
  let env = Env.set_env_pessimize env in
  Typing_helpers.add_decl_errors (Option.bind tc ~f:Cls.decl_errors);
  let env =
    Env.set_module
      env
      (Naming_attributes_params.get_module_attribute c.c_user_attributes)
  in
  let env =
    Env.set_internal
      env
      (Naming_attributes.mem SN.UserAttributes.uaInternal c.c_user_attributes)
  in
  Typing_type_wellformedness.class_ env c;
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
    if not (Pos.equal name_pos (Cls.pos tc |> Pos_or_decl.unsafe_to_raw_pos))
    then
      None
    else
      let env = Typing_requirements.check_class env name_pos tc in
      if shallow_decl_enabled ctx then
        Typing_inheritance.check_class env name_pos tc;
      Some (class_def_ env c tc)

let gconst_def ctx cst =
  Counters.count Counters.Category.Typecheck @@ fun () ->
  Errors.run_with_span cst.cst_span @@ fun () ->
  let env = EnvFromDef.gconst_env ~origin:Decl_counters.TopLevel ctx cst in
  let env = Env.set_env_pessimize env in
  let (typed_cst_value, env) =
    let value = cst.cst_value in
    match cst.cst_type with
    | Some hint ->
      let ty = Decl_hint.hint env.decl_env hint in
      let ty = Typing_enforceability.compute_enforced_ty env ty in
      let (env, dty) =
        Phase.localize_possibly_enforced_no_subst env ~ignore_errors:false ty
      in
      let (env, te, value_type) =
        let expected =
          ExpectedTy.make_and_allow_coercion (fst hint) Reason.URhint dty
        in
        Typing.expr_with_pure_coeffects env ~expected value
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
      let (env, te, _value_type) = Typing.expr_with_pure_coeffects env value in
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
  let (p, _) = hint in
  let (env, cty) = Phase.localize_hint_no_subst env ~ignore_errors:false hint in
  let expected = ExpectedTy.make p Reason.URhint cty in
  match e with
  | Some e ->
    let (env, te, ty) = Typing.expr_with_pure_coeffects env ~expected e in
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
  Counters.count Counters.Category.Typecheck @@ fun () ->
  let env = EnvFromDef.record_def_env ~origin:Decl_counters.TopLevel ctx rd in
  (match rd.rd_extends with
  | Some parent -> record_def_parent env rd parent
  | None -> ());

  check_record_inheritance_cycle env rd.rd_name;

  let (env, attributes) =
    List.map_env env rd.rd_user_attributes ~f:Typing.user_attribute
  in
  let (_env, fields) = List.map_env env rd.rd_fields ~f:record_field in
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
      let env = Env.empty ctx Relative_path.default ~droot:None in
      Some (Aast.Stmt (snd (Typing.stmt env s)), [])
    | Namespace _
    | NamespaceUse _
    | SetNamespaceEnv _
    | FileAttributes _ ->
      failwith
        "Invalid nodes in NAST. These nodes should be removed during naming."
  in
  Nast_check.program ctx nast;
  let (tast, envs) = List.unzip @@ List.filter_map nast ~f:convert_def in
  let envs = List.concat envs in
  if do_tast_checks then Tast_check.program ctx tast;
  (tast, envs)

let nast_to_tast ~do_tast_checks ctx nast =
  let (tast, _gienvs) = nast_to_tast_gienv ~do_tast_checks ctx nast in
  tast

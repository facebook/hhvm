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

let enforce_param_not_disposable env param ty =
  if has_accept_disposable_attribute param then
    ()
  else
    let p = param.param_pos in
    match Typing_disposable.is_disposable_type env ty with
    | Some class_name -> Errors.invalid_disposable_hint p (strip_ns class_name)
    | None -> ()

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
      mk (r, Tarraykind (AKvarray arr_values))
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
    | ( Some { ft_arity = Fvariadic (_, { fp_type = { et_type; _ }; _ }); _ },
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

let map_funcbody_annotation an =
  match an with
  | Nast.NamedWithUnsafeBlocks -> Tast.HasUnsafeBlocks
  | Nast.Named -> Tast.NoUnsafeBlocks
  | Nast.Unnamed _ -> failwith "Should not map over unnamed body"

let fun_def ctx f :
    (Tast.fun_def * Typing_inference_env.t_global_with_pos) option =
  let env = EnvFromDef.fun_env ctx f in
  with_timeout env f.f_name ~do_:(fun env ->
      (* reset the expression dependent display ids for each function body *)
      Reason.expr_display_id_map := IMap.empty;
      let pos = fst f.f_name in
      let decl_header = get_decl_function_header env (snd f.f_name) in
      let nb = TNBody.func_body ctx f in
      Typing_helpers.add_decl_errors
        (Option.map
           (Env.get_fun env (snd f.f_name))
           ~f:(fun x -> Option.value_exn x.fe_decl_errors));
      let env = Env.open_tyvars env (fst f.f_name) in
      let env = Env.set_env_function_pos env pos in
      let env = Env.set_env_pessimize env in
      let env =
        Typing.attributes_check_def env SN.AttributeKinds.fn f.f_user_attributes
      in
      let reactive =
        fun_reactivity env.decl_env f.f_user_attributes f.f_params
      in
      let mut = TUtils.fun_mutable f.f_user_attributes in
      let env = Env.set_env_reactive env reactive in
      let env = Env.set_fun_mutable env mut in
      NastCheck.fun_ env f;
      let ety_env = Phase.env_with_self env in
      let f_tparams : decl_tparam list =
        List.map
          f.f_tparams
          ~f:(Decl_hint.aast_tparam_to_decl_tparam env.decl_env)
      in
      let (env, constraints) =
        Phase.localize_generic_parameters_with_bounds env f_tparams ~ety_env
      in
      let env = SubType.add_constraints pos env constraints in
      let env =
        Phase.localize_where_constraints ~ety_env env f.f_where_constraints
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
        List.map_env env (List.zip_exn param_tys f.f_params) Typing.bind_param
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
      let (env, tb) = Typing.fun_ ~disable env return pos nb f.f_fun_kind in
      (* restore original reactivity *)
      let env = Env.set_env_reactive env reactive in
      begin
        match hint_of_type_hint f.f_ret with
        | None ->
          if partial_callback 4030 then Errors.expecting_return_type_hint pos
        | Some hint -> Typing_return.async_suggest_return f.f_fun_kind hint pos
      end;
      let (env, file_attrs) = Typing.file_attributes env f.f_file_attributes in
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
          Aast.f_fun_kind = f.f_fun_kind;
          Aast.f_file_attributes = file_attrs;
          Aast.f_user_attributes = user_attributes;
          Aast.f_body =
            {
              Aast.fb_ast = tb;
              fb_annotation = map_funcbody_annotation nb.fb_annotation;
            };
          Aast.f_external = f.f_external;
          Aast.f_namespace = f.f_namespace;
          Aast.f_doc_comment = f.f_doc_comment;
          Aast.f_static = f.f_static;
        }
      in
      let (_env, global_inference_env) = Env.extract_global_inference_env env in
      (fundef, (pos, global_inference_env)))

let gconst_def ctx cst =
  let env = EnvFromDef.gconst_env ctx cst in
  let env = Env.set_env_pessimize env in
  add_decl_errors (Option.map (Env.get_gconst env (snd cst.cst_name)) ~f:snd);

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
  }

let nast_to_tast_gienv ~(do_tast_checks : bool) ctx nast :
    _ * Typing_inference_env.t_global_with_pos list =
  let convert_def = function
    | Fun f ->
      begin
        match fun_def ctx f with
        | Some (f, env) -> (Aast.Fun f, [env])
        | None ->
          failwith
          @@ Printf.sprintf
               "Error when typechecking function: %s"
               (snd f.f_name)
      end
    | Constant gc -> (Aast.Constant (gconst_def ctx gc), [])
    | Typedef td -> (Aast.Typedef (Typing.typedef_def ctx td), [])
    | Class c ->
      begin
        match Typing.class_def ctx c with
        | Some (c, envs) -> (Aast.Class c, envs)
        | None ->
          failwith
          @@ Printf.sprintf
               "Error in declaration of class: %s\n%s"
               (snd c.c_name)
               ( Caml.Printexc.get_callstack 99
               |> Caml.Printexc.raw_backtrace_to_string )
      end
    | RecordDef rd -> (Aast.RecordDef (record_def_def ctx rd), [])
    (* We don't typecheck top level statements:
     * https://docs.hhvm.com/hack/unsupported/top-level
     * so just create the minimal env for us to construct a Stmt.
     *)
    | Stmt s ->
      let env = Env.empty ctx Relative_path.default None in
      (Aast.Stmt (snd (Typing.stmt env s)), [])
    | Namespace _
    | NamespaceUse _
    | SetNamespaceEnv _
    | FileAttributes _ ->
      failwith
        "Invalid nodes in NAST. These nodes should be removed during naming."
  in
  Nast_check.program ctx nast;
  let (tast, envs) = List.unzip @@ List.map nast convert_def in
  let envs = List.concat envs in
  if do_tast_checks then Tast_check.program ctx tast;
  (tast, envs)

let nast_to_tast ~do_tast_checks ctx nast =
  let (tast, _gienvs) = nast_to_tast_gienv ~do_tast_checks ctx nast in
  tast

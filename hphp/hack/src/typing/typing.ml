(**
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
open Core_kernel
open Common
open Decl_defs
open Nast
open Typing_defs
open Utils

module TFTerm       = Typing_func_terminality
module TUtils       = Typing_utils
module Reason       = Typing_reason
module Inst         = Decl_instantiate
module Type         = Typing_ops
module Env          = Typing_env
module LEnv         = Typing_lenv
module Async        = Typing_async
module SubType      = Typing_subtype
module Unify        = Typing_unify
module Union        = Typing_union
module SN           = Naming_special_names
module TVis         = Typing_visibility
module TNBody       = Typing_naming_body
module T            = Tast
module Phase        = Typing_phase
module Subst        = Decl_subst
module ExprDepTy    = Typing_dependent_type.ExprDepTy
module TCO          = TypecheckerOptions
module EnvFromDef   = Typing_env_from_def.EnvFromDef(Nast.Annotations)
module TySet        = Typing_set
module C            = Typing_continuations
module CMap         = C.Map
module Try          = Typing_try
module TR           = Typing_reactivity
module FL           = FeatureLogging
module MakeType     = Typing_make_type
module Cls          = Typing_classes_heap

(* Maps a Nast to a Tast where every type is Tany.
   Used to produce a Tast for unsafe code without inferring types for it. *)
module NastTanyMapper =
  Aast_mapper.MapAnnotatedAST(Nast.Annotations)(Tast.Annotations)

let map_funcbody_annotation an =
  match an with
  | Nast.Annotations.FuncBodyAnnotation.NamedWithUnsafeBlocks ->
    Tast.Annotations.FuncBodyAnnotation.HasUnsafeBlocks
  | Nast.Annotations.FuncBodyAnnotation.Named ->
    Tast.Annotations.FuncBodyAnnotation.NoUnsafeBlocks
  | Nast.Annotations.FuncBodyAnnotation.Unnamed _ ->
    failwith "Should not map over unnamed body"

let ntm_env tcopt =
  NastTanyMapper.{
    map_env_annotation = (fun () -> Tast.empty_saved_env tcopt);
    map_expr_annotation = (fun p -> p, (Reason.Rnone, Tany));
    map_funcbody_annotation = map_funcbody_annotation;
  }

(*****************************************************************************)
(* Debugging *)
(*****************************************************************************)

(* A guess as to the last position we were typechecking, for use in debugging,
 * such as figuring out what a runaway hh_server thread is doing. Updated
 * only best-effort -- it's an approximation to point debugging in the right
 * direction, nothing more. *)
let debug_last_pos = ref Pos.none
let debug_print_last_pos _ = print_endline (Pos.string (Pos.to_absolute
  !debug_last_pos))

(****************************************************************************)
(* Hooks *)
(****************************************************************************)

let expr_hook = ref None

let with_expr_hook hook f = with_context
  ~enter: (fun () -> expr_hook := Some hook)
  ~exit: (fun () -> expr_hook := None)
  ~do_: f

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let suggest env p ty =
  let ty = Typing_expand.fully_expand env ty in
  (match Typing_print.suggest ty with
  | "..." -> Errors.expecting_type_hint p
  | ty -> Errors.expecting_type_hint_suggest p ty
  )

let err_witness env p = Reason.Rwitness p, Typing_utils.terr env

(* When typing an expression, we optionally pass in the type
 * that we *expect* the expression to have.
 *)
type _expected_ty =
  Pos.t * Reason.ureason * locl ty

let expr_error env p r =
  let ty = (r, Typing_utils.terr env) in
  env, T.make_typed_expr p ty T.Any, ty

let expr_any env p r =
  let ty = (r, Typing_utils.tany env) in
  env, T.make_typed_expr p ty T.Any, ty

let compare_field_kinds x y =
  match x, y with
  | Nast.AFvalue (p1, _), Nast.AFkvalue ((p2, _), _)
  | Nast.AFkvalue ((p2, _), _), Nast.AFvalue (p1, _) ->
      Errors.field_kinds p1 p2;
      false
  | _ ->
      true

let check_consistent_fields x l =
  List.for_all l (compare_field_kinds x)

let unbound_name env (pos, name) =
  match Env.get_mode env with
  | FileInfo.Mstrict | FileInfo.Mexperimental ->
    (Errors.unbound_name_typing pos name;
    expr_error env pos (Reason.Rwitness pos))

  | FileInfo.Mdecl | FileInfo.Mpartial | FileInfo.Mphp ->
    expr_any env pos (Reason.Rwitness pos)

(* Is this type Traversable<vty> or Container<vty> for some vty? *)
let get_value_collection_inst ty =
  match ty with
  | (_, Tclass ((_, c), _, [vty])) when
      c = SN.Collections.cTraversable ||
      c = SN.Collections.cContainer ->
    Some vty
    (* If we're expecting a mixed or a nonnull then we can just assume
     * that the element type is mixed *)
  | (_, Tnonnull) ->
    Some (MakeType.mixed Reason.Rnone)
  | (_, Tany) ->
    Some ty
  | _ ->
    None

(* Is this type KeyedTraversable<kty,vty>
 *           or KeyedContainer<kty,vty>
 * for some kty, vty?
 *)
let get_key_value_collection_inst ty =
  match ty with
  | (_, Tclass ((_, c), _, [kty; vty])) when
      c = SN.Collections.cKeyedTraversable ||
      c = SN.Collections.cKeyedContainer ->
    Some (kty, vty)
    (* If we're expecting a mixed or a nonnull then we can just assume
     * that the value and key types are mixed *)
  | (_, Tnonnull) ->
    let mixed = MakeType.mixed Reason.Rnone in
    Some (mixed, mixed)
  | (_, Tany) ->
    Some (ty, ty)
  | _ ->
    None

(* Is this type varray<vty> or a supertype for some vty? *)
let get_varray_inst ty =
  match ty with
  (* It's varray<vty> *)
  | (_, Tarraykind (AKvarray vty)) -> Some vty
  | _ -> get_value_collection_inst ty

(* Is this type one of the value collection types with element type vty? *)
let get_vc_inst vc_kind ty =
  match ty with
  | (_, Tclass ((_, c), _, [vty]))
    when c = vc_kind_to_name vc_kind -> Some vty
  | _ ->  get_value_collection_inst ty

(* Is this type array<vty> or a supertype for some vty? *)
let get_akvec_inst ty =
  match ty with
  | (_, Tarraykind (AKvec vty)) -> Some vty
  | _ -> get_value_collection_inst ty

(* Is this type array<kty,vty> or a supertype for some kty and vty? *)
let get_akmap_inst ty =
  match ty with
  | (_, Tarraykind (AKmap (kty, vty))) -> Some (kty, vty)
  | _ -> get_key_value_collection_inst ty

(* Is this type one of the three key-value collection types
 * e.g. dict<kty,vty> or a supertype for some kty and vty? *)
let get_kvc_inst kvc_kind ty =
  match ty with
  | (_, Tclass ((_, c), _, [kty; vty]))
    when c = kvc_kind_to_name kvc_kind -> Some (kty, vty)
  | _ -> get_key_value_collection_inst ty

(* Is this type darray<kty, vty> or a supertype for some kty and vty? *)
let get_darray_inst ty =
  match ty with
  (* It's darray<kty, vty> *)
  | (_, Tarraykind (AKdarray (kty, vty))) -> Some (kty, vty)
  | _ -> get_key_value_collection_inst ty

let with_timeout opts fun_name ~(do_ : unit -> 'b): 'b option =
  let timeout = opts.GlobalOptions.tco_timeout in
  if timeout = 0 then Some (do_ ())
  else
    Timeout.with_timeout ~timeout
      ~on_timeout:(fun _ -> Errors.typechecker_timeout fun_name timeout; None)
      ~do_:(fun _ -> Some (do_ ()))

(*****************************************************************************)
(* Handling function/method arguments *)
(*****************************************************************************)
let param_has_attribute param attr =
  List.exists param.param_user_attributes
    (fun { ua_name; _ } -> attr = snd ua_name)

let has_accept_disposable_attribute param =
  param_has_attribute param SN.UserAttributes.uaAcceptDisposable

let get_param_mutability param =
  if param_has_attribute param SN.UserAttributes.uaMutable
  then Some Param_borrowed_mutable
  else if param_has_attribute param SN.UserAttributes.uaMaybeMutable
  then Some Param_maybe_mutable
  else if param_has_attribute param SN.UserAttributes.uaOwnedMutable
  then Some Param_owned_mutable
  else None

(* Check whether this is a function type that (a) either returns a disposable
 * or (b) has the <<__ReturnDisposable>> attribute
 *)
let is_return_disposable_fun_type env ty =
  match Env.expand_type env ty with
  | _env, (_, Tfun ft) ->
    ft.ft_return_disposable || Option.is_some (Typing_disposable.is_disposable_type env ft.ft_ret)
  | _ -> false

let enforce_param_not_disposable env param ty =
  if has_accept_disposable_attribute param then ()
  else
  let p = param.param_pos in
  match Typing_disposable.is_disposable_type env ty with
  | Some class_name ->
    Errors.invalid_disposable_hint p (strip_ns class_name)
  | None ->
    ()

let param_has_at_most_rx_as_func p =
  let module UA = SN.UserAttributes in
  Attributes.mem UA.uaAtMostRxAsFunc p.param_user_attributes

let fun_reactivity env attrs params =
  let r = Decl_fun_utils.fun_reactivity env attrs in
  let module UA = Naming_special_names.UserAttributes in

  let r =
    (* if at least one of parameters has <<__AtMostRxAsFunc>> attribute -
      treat function reactivity as generic that is determined from the reactivity
      of arguments annotated with __AtMostRxAsFunc. Declared reactivity is used as a
      upper boundary of the reactivity function can have. *)
    if List.exists params ~f:param_has_at_most_rx_as_func
    then RxVar (Some r)
    else r in

  let r =
    (* if at least one of arguments have <<__OnlyRxIfImpl>> attribute -
      treat function reactivity as conditional that is determined at the callsite *)
    if List.exists params
      ~f:(fun { param_user_attributes = p; _ } ->
        Attributes.mem UA.uaOnlyRxIfImpl p)
    then MaybeReactive r
    else r in
  r

type array_ctx = NoArray | ElementAssignment | ElementAccess

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
let make_param_local_ty env param =
  let ety_env =
    { (Phase.env_with_self env) with from_class = Some CIstatic; } in
  let env, ty =
    match param.param_hint with
    | None ->
      let r = Reason.Rwitness param.param_pos in
      env, (r, TUtils.tany env)
    | Some x ->
      let ty = Decl_hint.hint env.Env.decl_env x in
      let condition_type =
        Decl_fun_utils.condition_type_from_attributes env.Env.decl_env param.param_user_attributes in
      begin match condition_type with
      | Some condition_type ->
        let env, ty = Phase.localize ~ety_env env ty in
        begin match TR.try_substitute_type_with_condition env condition_type ty with
        | Some r -> r
        | None -> env, ty
        end
      | _ when Attributes.mem SN.UserAttributes.uaAtMostRxAsFunc param.param_user_attributes ->
        let env, ty = Phase.localize ~ety_env env ty in
        (* expand type to track aliased function types *)
        let env, expanded_ty = Env.expand_type env ty in
        let adjusted_ty = make_function_type_rxvar expanded_ty  in
        env, if phys_equal adjusted_ty expanded_ty then ty else adjusted_ty
      | _ ->
        Phase.localize ~ety_env env ty
      end
  in
  let ty = match ty with
    | _, t when param.param_is_variadic ->
      (* when checking the body of a function with a variadic
       * argument, "f(C ...$args)", $args is a varray<C> *)
      let r = Reason.Rvar_param param.param_pos in
      let arr_values = r, t in
      r, Tarraykind (AKvarray arr_values)
    | x -> x
  in
  Typing_reactivity.disallow_atmost_rx_as_rxfunc_on_non_functions env param ty;
  env, ty

(* Given a localized parameter type and parameter information, infer
 * a type for the parameter default expression (if present) and check that
 * it is a subtype of the parameter type (if present). If no parameter type
 * is specified, then union with Tany. (So it's as though we did a conditional
 * assignment of the default expression to the parameter).
 * Set the type of the parameter in the locals environment *)
let rec bind_param env (ty1, param) =
  let env, param_te, ty1 =
    match param.param_expr with
    | None ->
        Typing_suggest.save_param (param.param_name) env ty1 (Reason.none, Tany);
        env, None, ty1
    | Some e ->
        let env, te, ty2 = expr ~expected:(param.param_pos, Reason.URparam, ty1) env e in
        Typing_sequencing.sequence_check_expr e;
        Typing_suggest.save_param (param.param_name) env ty1 ty2;
        let env, ty1 =
          if Option.is_none param.param_hint
          (* In this case ty1 must be Tany, so just union it with the type of
           * the default expression *)
          then Union.union env ty1 ty2
          (* Otherwise we have an explicit type, and the default expression type
           * must be a subtype *)
          else
            let env = Type.sub_type param.param_pos Reason.URhint env ty2 ty1 in
            env, ty1 in
        env, Some te, ty1
  in
  let tparam = {
    T.param_annotation = T.make_expr_annotation param.param_pos ty1;
    T.param_hint = param.param_hint;
    T.param_is_reference = param.param_is_reference;
    T.param_is_variadic = param.param_is_variadic;
    T.param_pos = param.param_pos;
    T.param_name = param.param_name;
    T.param_expr = param_te;
    T.param_callconv = param.param_callconv;
    T.param_user_attributes = List.map param.param_user_attributes (user_attribute env);
  } in
  let mode = get_param_mode param.param_is_reference param.param_callconv in
  let id = Local_id.make_unscoped param.param_name in
  let env = Env.set_local env id ty1 in
  let env = Env.set_param env id (ty1, mode) in
  let env = if has_accept_disposable_attribute param
            then Env.set_using_var env id else env in
  let env =
    match get_param_mutability param with
    | Some Param_borrowed_mutable ->
      Env.add_mutable_var env id (param.param_pos, Typing_mutability_env.Borrowed)
    | Some Param_owned_mutable ->
      Env.add_mutable_var env id (param.param_pos, Typing_mutability_env.Mutable)
    | Some Param_maybe_mutable ->
      Env.add_mutable_var env id (param.param_pos, Typing_mutability_env.MaybeMutable)
    | None ->
      Env.add_mutable_var env id (param.param_pos, Typing_mutability_env.Immutable)
    in
  env, tparam

(* In strict mode, we force you to give a type declaration on a parameter *)
(* But the type checker is nice: it makes a suggestion :-) *)
and check_param env param ty =
  let env = Typing_attributes.check_def env new_object
    SN.AttributeKinds.parameter param.param_user_attributes in
  match param.param_hint with
  | None -> suggest env param.param_pos ty
  | Some _ ->
    (* We do not permit hints to implement IDisposable or IAsyncDisposable *)
    enforce_param_not_disposable env param ty

and check_inout_return env =
  let params = Local_id.Map.elements (Env.get_params env) in
  List.fold params ~init:env ~f:begin fun env (id, ((r, ty), mode)) ->
    match mode with
    | FPinout ->
      (* Whenever the function exits normally, we require that each local
       * corresponding to an inout parameter be compatible with the original
       * type for the parameter (under subtyping rules). *)
      let local_ty = Env.get_local env id in
      let env, ety = Env.expand_type env local_ty in
      let pos = Reason.to_pos (fst ety) in
      let param_ty = Reason.Rinout_param (Reason.to_pos r), ty in
      Type.sub_type pos Reason.URassign_inout env ety param_ty
    | _ -> env
  end

and add_decl_errors = function
  | None -> ()
  | Some errors -> Errors.merge_into_current errors

(*****************************************************************************)
(* Now we are actually checking stuff! *)
(*****************************************************************************)
and fun_def tcopt f : Tast.fun_def option =
  with_timeout tcopt f.f_name ~do_:begin fun () ->
  (* reset the expression dependent display ids for each function body *)
  Reason.expr_display_id_map := IMap.empty;
  let pos = fst f.f_name in
  let nb = TNBody.func_body f in
  let env = EnvFromDef.fun_env tcopt f in
  add_decl_errors (Option.map
    (Env.get_fun env (snd f.f_name))
    ~f:(fun x -> Option.value_exn x.ft_decl_errors)
  );
  let env = Env.set_env_function_pos env pos in
  let env = Typing_attributes.check_def env new_object SN.AttributeKinds.fn f.f_user_attributes in
  let reactive = fun_reactivity env.Env.decl_env f.f_user_attributes f.f_params in
  let mut = TUtils.fun_mutable f.f_user_attributes in
  let env = Env.set_env_reactive env reactive in
  let env = Env.set_fun_mutable env mut in
  NastCheck.fun_ env f nb;
  (* Fresh type environment is actually unnecessary, but I prefer to
   * have a guarantee that we are using a clean typing environment. *)
  let tfun_def = Env.fresh_tenv env (
    fun env ->
      let ety_env = Phase.env_with_self env in
      let env, constraints =
        Phase.localize_generic_parameters_with_bounds env f.f_tparams
          ~ety_env in
      let env = add_constraints pos env constraints in
      let env =
        Phase.localize_where_constraints ~ety_env env f.f_where_constraints in
      let env, ty =
        match f.f_ret with
        | None ->
          env, (Reason.Rwitness pos, Typing_utils.tany env)
        | Some ret ->
          let ty = Decl_hint.hint env.Env.decl_env ret in
          Phase.localize_with_self env ty in
      let return = Typing_return.make_info f.f_fun_kind f.f_user_attributes env
        ~is_explicit:(Option.is_some f.f_ret) ty in
      let env, param_tys =
        List.map_env env f.f_params make_param_local_ty in
      if Env.is_strict env then
        List.iter2_exn ~f:(check_param env) f.f_params param_tys;
      Typing_memoize.check_function env f;
      let env, typed_params = List.map_env env (List.zip_exn param_tys f.f_params)
        bind_param in
      let env, t_variadic = match f.f_variadic with
        | FVvariadicArg vparam ->
          let env, ty = make_param_local_ty env vparam in
          if Env.is_strict env then
            check_param env vparam ty;
          let env, t_vparam = bind_param env (ty, vparam) in
          env, T.FVvariadicArg t_vparam
        | FVellipsis p ->
          if Env.is_strict env then
            Errors.ellipsis_strict_mode ~require:`Type_and_param_name pos;
          env, T.FVellipsis p
        | FVnonVariadic -> env, T.FVnonVariadic in
      let local_tpenv = env.Env.lenv.Env.tpenv in
      let env, tb = fun_ env return pos nb f.f_fun_kind in
      let env = SubType.solve_all_unsolved_tyvars env in
      Typing_subtype.log_prop env;
      let env = Env.check_todo env in
      (* restore original reactivity *)
      let env = Env.set_env_reactive env reactive in
      begin match f.f_ret with
        | None when Env.is_strict env ->
          Typing_return.suggest_return env pos return.Typing_env_return_info.return_type
        | None -> Typing_suggest.save_fun_or_method f.f_name
        | Some hint ->
          Typing_return.async_suggest_return (f.f_fun_kind) hint pos
      end;
      let filename = Pos.filename (fst f.f_name) in
      let droot = env.Env.decl_env.Decl_env.droot in
      let file_attrs = file_attributes tcopt filename f.f_mode droot f.f_file_attributes in
      let fundef = {
        T.f_annotation = Env.save local_tpenv env;
        T.f_span = f.f_span;
        T.f_mode = f.f_mode;
        T.f_ret = f.f_ret;
        T.f_name = f.f_name;
        T.f_tparams = List.map f.f_tparams (type_param env);
        T.f_where_constraints = f.f_where_constraints;
        T.f_variadic = t_variadic;
        T.f_params = typed_params;
        T.f_fun_kind = f.f_fun_kind;
        T.f_file_attributes = file_attrs;
        T.f_user_attributes = List.map f.f_user_attributes (user_attribute env);
        T.f_body = { T.fb_ast = tb; fb_annotation = map_funcbody_annotation nb.fb_annotation };
        T.f_external = f.f_external;
        T.f_namespace = f.f_namespace;
        T.f_doc_comment = f.f_doc_comment;
        T.f_static = f.f_static;
      } in
      Typing_lambda_ambiguous.suggest_fun_def env fundef
  ) in
  tfun_def
  end (* with_timeout *)

(*****************************************************************************)
(* function used to type closures, functions and methods *)
(*****************************************************************************)

and fun_ ?(abstract=false) env return pos named_body f_kind =
  Env.with_env env begin fun env ->
    debug_last_pos := pos;
    let env = Env.set_return env return in
    let env = Env.set_fn_kind env f_kind in
    let env, tb = block env named_body.fb_ast in
    Typing_sequencing.sequence_check_block named_body.fb_ast;
    let { Typing_env_return_info.return_type = ret; _} = Env.get_return env in
    let env =
      if not @@ LEnv.has_next env ||
        abstract ||
        Nast.named_body_is_unsafe named_body
      then env
      else fun_implicit_return env pos ret f_kind in
    debug_last_pos := Pos.none;
    env, tb
  end

and fun_implicit_return env pos ret = function
  | Ast.FGenerator | Ast.FAsyncGenerator -> env
  | Ast.FCoroutine
  | Ast.FSync ->
    (* A function without a terminal block has an implicit return; the
     * "void" type *)
    let env = check_inout_return env in
    let r = Reason.Rno_return pos in
    let rty = MakeType.void r in
    Typing_return.implicit_return env pos ~expected:ret ~actual:rty
  | Ast.FAsync ->
    (* An async function without a terminal block has an implicit return;
     * the Awaitable<void> type *)
    let r = Reason.Rno_return_async pos in
    let rty = MakeType.awaitable r (MakeType.void r) in
    Typing_return.implicit_return env pos ~expected:ret ~actual:rty

and block env (stl : block) = List.map_env env stl ~f:stmt

(* Set a local; must not be already assigned if it is a using variable *)
and set_local ?(is_using_clause = false) env (pos,x) ty =
  if Env.is_using_var env x
  then
    if is_using_clause
    then Errors.duplicate_using_var pos
    else Errors.illegal_disposable pos "assigned";
  let env = Env.set_local env x ty in
  if is_using_clause then Env.set_using_var env x else env

(* Check an individual component in the expression `e` in the
 * `using (e) { ... }` statement.
 * This consists of either
 *   a simple assignment `$x = e`, in which `$x` is the using variable, or
 *   an arbitrary expression `e`, in which case a temporary is the using
 *      variable, inaccessible in the source.
 * Return the typed expression and its type, and any variables that must
 * be designated as "using variables" for avoiding escapes.
 *)
and check_using_expr has_await env ((pos, content) as using_clause) =
  match content with
    (* Simple assignment to local of form `$lvar = e` *)
  | Binop (Ast.Eq None, (lvar_pos, Lvar lvar), e) ->
    let env, te, ty = expr ~is_using_clause:true env e in
    let env = Typing_disposable.enforce_is_disposable_type env has_await (fst e) ty in
    let env = set_local ~is_using_clause:true env lvar ty in
    (* We are assigning a new value to the local variable, so we need to
     * generate a new expression id
     *)
    let env = Env.set_local_expr_id env (snd lvar) (Ident.tmp()) in
    env, (T.make_typed_expr pos ty (T.Binop (Ast.Eq None,
      T.make_typed_expr lvar_pos ty (T.Lvar lvar), te)), [snd lvar])

    (* Arbitrary expression. This will be assigned to a temporary *)
  | _ ->
    let env, typed_using_clause, ty = expr ~is_using_clause:true env using_clause in
    let env = Typing_disposable.enforce_is_disposable_type env has_await pos ty in
    env, (typed_using_clause, [])

(* Check the using clause e in
 * `using (e) { ... }` statement (`has_await = false`) or
 * `await using (e) { ... }` statement (`has_await = true`).
 * The expression consists of a comma-separated list of expressions (Expr_list)
 * or a single expression.
 * Return the typed expression, and any variables that must
 * be designated as "using variables" for avoiding escapes.
 *)
and check_using_clause env has_await ((pos, content) as using_clause) =
  match content with
  | Expr_list using_clauses ->
    let env, pairs = List.map_env env using_clauses (check_using_expr has_await) in
    let typed_using_clauses, vars_list = List.unzip pairs in
    let ty_ = Ttuple (List.map typed_using_clauses T.get_type) in
    let ty = (Reason.Rnone, ty_) in
    env, T.make_typed_expr pos ty (T.Expr_list typed_using_clauses),
      List.concat vars_list
  | _ ->
    let env, (typed_using_clause, vars) = check_using_expr has_await env using_clause in
    env, typed_using_clause, vars

(* Require a new construct with disposable *)
and enforce_return_disposable _env e =
  match e with
  | _, New _ -> ()
  | _, Call _ -> ()
  | _, Await (_, Call _) -> ()
  | p, _ ->
    Errors.invalid_return_disposable p

(* Wrappers around the function with the same name in Typing_lenv, which only
 * performs the move/save and merge operation if we are in a try block or in a
 * function with return type 'noreturn'.
 * This enables significant perf improvement, because this is called at every
 * function of method call, when most calls are outside of a try block. *)
and move_and_merge_next_in_catch env =
  if env.Env.in_try || (TFTerm.is_noreturn env)
    then LEnv.move_and_merge_next_in_cont env C.Catch
    else LEnv.drop_cont env C.Next

and save_and_merge_next_in_catch env =
  if env.Env.in_try || (TFTerm.is_noreturn env)
    then LEnv.save_and_merge_next_in_cont env C.Catch
    else env

and might_throw env = save_and_merge_next_in_catch env

and gather_defined_in_block env b =
  let locals = Typing_gather_defined.block env b in
  Env.env_with_locals env locals

and gather_defined_in_expr env e =
  let locals = Typing_gather_defined.expr env e in
  Env.env_with_locals env locals

and stmt env (pos, st) =
  let env, st = stmt_ env pos st in
  env, (pos, st)

and stmt_ env pos st =
  let env = Env.open_tyvars env pos in
  (fun (env, tb) -> SubType.close_tyvars_and_solve env, tb) @@
  match st with
  | Unsafe_block b ->
    (* Do not run inference on the block, since unsafe is sometimes used to work
       around inference performance problems. *)
    let env = gather_defined_in_block env b in
    let tcopt = Env.get_tcopt env in
    let tb = NastTanyMapper.map_block (ntm_env tcopt) b in
    env, T.Unsafe_block tb
  | Fallthrough ->
      let env = if env.Env.in_case
        then LEnv.move_and_merge_next_in_cont env C.Fallthrough
        else env in
      env, T.Fallthrough
  | GotoLabel _
  | Goto _ ->
    let env = move_and_merge_next_in_catch env in
    env, T.Noop
  | Noop ->
      env, T.Noop
  | Expr e ->
      let env, te, _ = expr env e in
      let env = if TFTerm.expression_exits env e
        then LEnv.move_and_merge_next_in_cont env C.Exit
        else env in
      env, T.Expr te
  | If (e, b1, b2)  ->
      let env, te, _ = expr env e in

      (* We stash away the locals environment because condition updates it
       * locally for checking b1. For example, we might have condition
       * $x === null, or $x instanceof C, which changes the type of $x in
       * lenv *)
      let parent_lenv = env.Env.lenv in

      let env   = condition env true te in
      let env, tb1 = block env b1 in
      let lenv1 = env.Env.lenv in

      let env   = { env with Env.lenv = parent_lenv } in
      let env   = condition env false te in
      let env, tb2 = block env b2 in
      let lenv2 = env.Env.lenv in

      let env = LEnv.union_lenvs env parent_lenv lenv1 lenv2 in
      (* TODO TAST: annotate with joined types *)
      env, T.If(te, tb1, tb2)
  | Return None ->
      let env = check_inout_return env in
      let rty = Typing_return.wrap_awaitable env pos (MakeType.void (Reason.Rwitness pos)) in
      let { Typing_env_return_info.return_type = expected_return; _ } = Env.get_return env in
      let env = Typing_return.implicit_return env pos ~expected:expected_return ~actual:rty in
      let env = LEnv.move_and_merge_next_in_cont env C.Exit in
      env, T.Return None
  | Return (Some e) ->
      let env = check_inout_return env in
      let expr_pos = fst e in
      let Typing_env_return_info.{
        return_type; return_disposable; return_mutable; return_explicit;
        return_void_to_rx } = Env.get_return env in
      let expected =
        if return_explicit
        then Some (expr_pos, Reason.URreturn,
          Typing_return.strip_awaitable (Env.get_fn_kind env) env return_type)
        else None in
      if return_disposable then enforce_return_disposable env e;
      let env, te, rty = expr ~is_using_clause:return_disposable ?expected:expected env e in
      let env =
        if Env.env_reactivity env <> Nonreactive
        then begin
          Typing_mutability.handle_value_in_return
            ~function_returns_mutable:return_mutable
            ~function_returns_void_for_rx: return_void_to_rx
            env
            env.Env.function_pos
            te
        end
        else env in
      let return_type = TR.strip_condition_type_in_return env return_type in
      let env, rty = Env.unbind env rty in
      let rty = Typing_return.wrap_awaitable env pos rty in
      Typing_suggest.save_return env return_type rty;
      let env = Type.coerce_type expr_pos Reason.URreturn env rty return_type in
      let env = LEnv.move_and_merge_next_in_cont env C.Exit in
      env, T.Return (Some te)
  | Do (b, e) as st ->
    (* NOTE: leaks scope as currently implemented; this matches
       the behavior in naming (cf. `do_stmt` in naming/naming.ml).
     *)
    let env, (tb, te) = LEnv.stash_and_do env [C.Continue; C.Break; C.Do]
      (fun env ->
        let env = LEnv.save_and_merge_next_in_cont env C.Do in
        let env, _ = block env b in
        (* saving the locals in continue here even if there is no continue
         * statement because they must be merged at the end of the loop, in
         * case there is no iteration *)
        let env = LEnv.save_and_merge_next_in_cont env C.Continue in
        let alias_depth =
          if env.Env.in_loop then 1 else Typing_alias.get_depth (pos, st) in
        let env, tb = Env.in_loop env begin
          iter_n_acc alias_depth begin fun env ->
            let env = LEnv.update_next_from_conts env [C.Continue; C.Next] in
            (* The following is necessary in case there is an assignment in the
             * expression *)
            let env, te, _ = expr env e in
            let env = condition env true te in
            let env = LEnv.update_next_from_conts env [C.Do; C.Next] in
            let env, tb = block env b in
            env, tb
          end end in
        let env = LEnv.update_next_from_conts env [C.Continue; C.Next] in
        let env, te, _ = expr env e in
        let env = condition env false te in
        let env = LEnv.update_next_from_conts env [C.Break; C.Next] in
        env, (tb, te)) in
    env, T.Do(tb, te)
  | While (e, b) as st ->
    let env, (te, tb) = LEnv.stash_and_do env [C.Continue; C.Break] (fun env ->
      let env = LEnv.save_and_merge_next_in_cont env C.Continue in
      let alias_depth =
        if env.Env.in_loop then 1 else Typing_alias.get_depth (pos, st) in
      let env, tb = Env.in_loop env begin
        iter_n_acc alias_depth begin fun env ->
          let env = LEnv.update_next_from_conts env [C.Continue; C.Next] in
          (* The following is necessary in case there is an assignment in the
           * expression *)
          let env, te, _ = expr env e in
          let env = condition env true te in
          (* TODO TAST: avoid repeated generation of block *)
          let env, tb = block env b in
          env, tb
        end
      end in
      let env = LEnv.update_next_from_conts env [C.Continue; C.Next] in
      let env, te, _ = expr env e in
      let env = condition env false te in
      let env = LEnv.update_next_from_conts env [C.Break; C.Next] in
      env, (te, tb)) in
    env, T.While (te, tb)
  | Using {
    us_has_await = has_await;
    us_expr = using_clause;
    us_block = using_block;
    us_is_block_scoped
    } ->
      let env, typed_using_clause, using_vars = check_using_clause env has_await using_clause in
      let env, typed_using_block = block env using_block in
      (* Remove any using variables from the environment, as they should not
       * be in scope outside the block *)
      let env = List.fold_left using_vars ~init:env ~f:Env.unset_local in
      env, T.Using T.{
        us_has_await = has_await;
        us_expr = typed_using_clause;
        us_block = typed_using_block;
        us_is_block_scoped;
      }
  | For (e1, e2, e3, b) as st ->
    let env, (te1, te2, te3, tb) = LEnv.stash_and_do env [C.Continue; C.Break]
      (fun env ->
        (* For loops leak their initalizer, but nothing that's defined in the
           body
         *)
        let (env, te1, _) = expr env e1 in      (* initializer *)
        let env = LEnv.save_and_merge_next_in_cont env C.Continue in
        let alias_depth =
          if env.Env.in_loop then 1 else Typing_alias.get_depth (pos, st) in
        let env, (tb, te3) = Env.in_loop env begin
          iter_n_acc alias_depth begin fun env ->
            (* The following is necessary in case there is an assignment in the
             * expression *)
            let env, te2, _ = expr env e2 in
            let env = condition env true te2 in
            let env, tb = block env b in
            let env = LEnv.update_next_from_conts env [C.Continue; C.Next] in
            let (env, te3, _) = expr env e3 in
            env, (tb, te3)
          end
        end in
        let env = LEnv.update_next_from_conts env [C.Continue; C.Next] in
        let (env, te2, _) = expr env e2 in
        let env = condition env false te2 in
        let env = LEnv.update_next_from_conts env [C.Break; C.Next] in
        env, (te1, te2, te3, tb)) in
    env, T.For(te1, te2, te3, tb)
  | Switch ((pos, _) as e, cl) ->
      let env, te, ty = expr env e in
      (* Exhaustiveness etc is sensitive to unions, so normalize to avoid
       * most of the problems. TODO: make it insensitive *)
      let env, ty = Union.simplify_unions env ty in
      (* NB: A 'continue' inside a 'switch' block is equivalent to a 'break'.
       * See the note in
       * http://php.net/manual/en/control-structures.continue.php *)
      let env, (te, tcl) = LEnv.stash_and_do env [C.Continue; C.Break]
        (fun env ->
          let parent_locals = LEnv.get_all_locals env in
          let case_list env = case_list parent_locals ty env pos cl in
          let env, tcl = Env.in_case env case_list in
          let env = LEnv.update_next_from_conts env
            [C.Continue; C.Break; C.Next] in
          env, (te, tcl)) in
      env, T.Switch(te, tcl)
  | Foreach (e1, e2, b) as st ->
    (* It's safe to do foreach over a disposable, as no leaking is possible *)
    let env, te1, ty1 = expr ~accept_using_var:true env e1 in
    TR.check_foreach_collection env (fst e1) ty1;
    let env, (te1, te2, tb) = LEnv.stash_and_do env [C.Continue; C.Break]
      (fun env ->
        let env = LEnv.save_and_merge_next_in_cont env C.Continue in
        let env, tk, tv = as_expr env ty1 (fst e1) e2 in
        let alias_depth =
          if env.Env.in_loop then 1 else Typing_alias.get_depth (pos, st) in
        let env, (te2, tb) = Env.in_loop env begin
          iter_n_acc alias_depth begin fun env ->
            let env = LEnv.update_next_from_conts env [C.Continue; C.Next] in
            let env, te2 = bind_as_expr env ty1 (fst e1) tk tv e2 in
            let env, tb = block env b in
            env, (te2, tb)
          end
        end in
        let env = LEnv.update_next_from_conts env
          [C.Continue; C.Break; C.Next] in
        env, (te1, te2, tb)) in
    env, T.Foreach (te1, te2, tb)
  | Try (tb, cl, fb) ->
    let env, ttb, tcl, tfb = try_catch env tb cl fb in
    env, T.Try (ttb, tcl, tfb)
  | Def_inline _ ->
     (* Do nothing, this doesn't occur in Hack code. *)
     failwith "Should never typecheck nested definitions"
  | Awaitall (el, b) ->
      let env = might_throw env in
      let env, el = List.fold_left el ~init:(env, []) ~f:(fun (env, tel) (e1, e2) ->
      let env, te2, ty2 = expr env e2 in
      let env, ty2 = Async.overload_extract_from_awaitable env (fst e2) ty2 in
      (match e1 with
      | Some e1 ->
        let env, _, _ = assign (fst e1) env (fst e1, Lvar e1) ty2 in
        (env, (Some e1, te2) :: tel)
      | None -> (env, (None, te2) :: tel)
      )
    ) in
    let env, b = block env b in
    env, T.Awaitall (el, b)
  | Throw (is_terminal, e) ->
    let p = fst e in
    let env, te, ty = expr env e in
    let env = exception_ty p env ty in
    let env = move_and_merge_next_in_catch env in
    env, T.Throw(is_terminal, te)
  | Continue ->
    let env = LEnv.move_and_merge_next_in_cont env C.Continue in
    env, T.Continue
  | Break ->
    let env = LEnv.move_and_merge_next_in_cont env C.Break in
    env, T.Break
  | Let ((p, x) as id, h, rhs) ->
    let env, hint_ty, expected = match h with
      | Some (p, h) ->
        let ety_env =
          { (Phase.env_with_self env) with from_class = Some CIstatic; } in
        let hint_ty = Decl_hint.hint env.Env.decl_env (p, h) in
        let env, hint_ty = Phase.localize ~ety_env env hint_ty in
        env, Some hint_ty, Some (p, Reason.URhint, hint_ty)
      | None -> env, None, None
      in
    let env, t_rhs, rhs_ty = expr env rhs in
    let env, _ = match hint_ty with
      | Some ty ->
        let env = check_expected_ty "Let" env rhs_ty expected in
        set_valid_rvalue p env x ty
      | None -> set_valid_rvalue p env x rhs_ty
    in
    (* Transfer expression ID with RHS to let varible if RHS is another variable *)
    let env = match rhs with
    | _, ImmutableVar (_, x_rhs) | _, Lvar (_, x_rhs) ->
      let eid_rhs = Env.get_local_expr_id env x_rhs in
        Option.value_map
          eid_rhs ~default:env
          ~f:(Env.set_local_expr_id env x)
    | _ -> env
    in
    env, T.Let (id, h, t_rhs)
  | Block _
  | Markup _
  | Declare _ ->
    failwith "Unexpected nodes in AST. These nodes should have been removed in naming."

and finally_cont fb env ctx =
  let env = LEnv.replace_cont env C.Next (Some ctx) in
  let env, _tfb = block env fb in
  env, LEnv.get_all_locals env

and finally env fb =
  match fb with
  | [] ->
    let env = LEnv.update_next_from_conts env [C.Next; C.Finally] in
    env, []
  | _ ->
    let parent_locals = LEnv.get_all_locals env in
    (* First typecheck the finally block against all continuations merged
    * together.
    * During this phase, record errors found in the finally block, but discard
    * the resulting environment. *)
    let env' = LEnv.update_next_from_conts env C.all in
    let _, tfb = block env' fb in
    (* Second, typecheck the finally block once against each continuation. This
    * helps be more clever about what each continuation will be after the
    * finally block.
    * We don't want to record errors during this phase, because certain types
    * of errors will fire wrongly. For example, if $x is nullable in some
    * continuations but not in others, then we must use `?->` on $x, but an
    * error will fire when typechecking the finally block againts continuations
    * where $x is non-null.  *)
    let finally_cont env _key = finally_cont fb env in
    let env, locals_map = Errors.ignore_ (fun () ->
      CMap.map_env finally_cont env parent_locals) in
    let env, locals = Try.finally_merge env locals_map in
    (Env.env_with_locals env locals), tfb

and try_catch env tb cl fb =
  let parent_locals = LEnv.get_all_locals env in
  let env = LEnv.drop_conts env
    [C.Break; C.Continue; C.Exit; C.Catch; C.Finally] in
  let env, (ttb, tcb) = Env.in_try env (fun env ->
    let env, ttb = block env tb in
    let env = LEnv.move_and_merge_next_in_cont env C.Finally in
    let catchctx = LEnv.get_cont_option env C.Catch in
    let env, lenvtcblist = List.map_env env ~f:(catch catchctx) cl in
    let lenvl, tcb = List.unzip lenvtcblist in
    let env = LEnv.union_lenv_list env env.Env.lenv lenvl in
    let env = LEnv.move_and_merge_next_in_cont env C.Finally in
    env, (ttb, tcb)) in
  let env, tfb = finally env fb in
  let env = LEnv.drop_cont env C.Finally in
  let env = LEnv.restore_and_merge_conts_from
    env parent_locals [C.Break; C.Continue; C.Exit; C.Catch; C.Finally] in
  env, ttb, tcb, tfb

and case_list parent_locals ty env switch_pos cl =
  let initialize_next_cont env =
    let env = LEnv.restore_conts_from env parent_locals [C.Next] in
    let env = LEnv.update_next_from_conts env [C.Next; C.Fallthrough] in
    LEnv.drop_cont env C.Fallthrough in

  let check_fallthrough env switch_pos case_pos block rest_of_list ~is_default =
    if not @@ List.is_empty block then
      begin match rest_of_list with
      | [] | [Default []] -> ()
      | _ ->
        begin match LEnv.get_cont_option env C.Next with
        | Some _ ->
          if is_default then Errors.default_fallthrough switch_pos
          else Errors.case_fallthrough switch_pos case_pos
        | None -> ()
        end (* match *)
      end (* match *)
    else () in

  let make_exhaustive_equivalent_case_list env cl =
    let has_default = List.exists cl ~f:(function Default _ -> true | _ -> false) in
    let env, ty =
      (* If it hasn't got a default clause then we need to solve type variables
       * in order to check for an enum *)
      if has_default
      then Env.expand_type env ty
      else SubType.expand_type_and_solve env ~description_of_expected:"a value" switch_pos ty in
    let is_enum = match snd ty with
      | Tabstract (AKenum _, _) -> true
      | _ -> false in
    (* If there is no default case and this is not a switch on enum (since
     * exhaustiveness is garanteed elsewhere on enums),
     * then add a default case for control flow correctness
     *)
    if has_default || is_enum then env, cl, false else env, cl @ [Default []], true in

  let rec case_list env = function
    | [] -> env, []
    | Default b :: rl ->
      let env = initialize_next_cont env in
      let env, tb = block env b in
      check_fallthrough env switch_pos Pos.none b rl ~is_default:true;
      let env, tcl = case_list env rl in
      env, T.Default tb::tcl
    | (Case ((pos, _) as e, b)) :: rl ->
      let env = initialize_next_cont env in
      let env, te, _ = expr env e in
      let env, tb = block env b in
      check_fallthrough env switch_pos pos b rl ~is_default:false;
      let env, tcl = case_list env rl in
      env, T.Case (te, tb)::tcl in

  let env, cl, added_empty_default = make_exhaustive_equivalent_case_list env cl in
  let env, tcl = case_list env cl in
  let tcl = if added_empty_default then List.take tcl (List.length tcl - 1) else tcl in
  env, tcl

and catch catchctx env (sid, exn, b) =
  let env = LEnv.replace_cont env C.Next catchctx in
  let cid = CI sid in
  let ety_p = (fst sid) in
  let env, _, _ = instantiable_cid ety_p env cid [] in
  let env, _te, ety = static_class_id ~check_constraints:false ety_p env [] cid in
  let env = exception_ty ety_p env ety in
  let env = set_local env exn ety in
  let env, tb = block env b in
  env, (env.Env.lenv, (sid, exn, tb))

and as_expr env ty1 pe e =
  let env = Env.open_tyvars env pe in
  (fun (env, ty, tk, tv) ->
    let env =
      if TUtils.is_dynamic env ty1
      then env
      else Type.sub_type pe Reason.URforeach env ty1 ty in
    let env = Env.set_tyvar_variance env ty in
  SubType.close_tyvars_and_solve env, tk, tv) @@
  let env, tv = Env.fresh_unresolved_type env pe in
  match e with
  | As_v _ ->
      let tk = MakeType.mixed Reason.Rnone in
      env, MakeType.traversable (Reason.Rforeach pe) tv, tk, tv
  | As_kv _ ->
      let env, tk = Env.fresh_unresolved_type env pe in
      env, MakeType.keyed_traversable (Reason.Rforeach pe) tk tv, tk, tv
  | Await_as_v _ ->
      let tk = MakeType.mixed Reason.Rnone in
      env, MakeType.async_iterator (Reason.Rasyncforeach pe) tv, tk, tv
  | Await_as_kv _ ->
      let env, tk = Env.fresh_unresolved_type env pe in
      env, MakeType.async_keyed_iterator (Reason.Rasyncforeach pe) tk tv, tk, tv

and bind_as_expr env loop_ty p ty1 ty2 aexpr =
  (* Set id as dynamic if the foreach loop was dynamic *)
  let env, eloop_ty = Env.expand_type env loop_ty in
  let ty1, ty2 = if TUtils.is_dynamic env eloop_ty then
    MakeType.dynamic (fst ty1), MakeType.dynamic (fst ty2) else ty1, ty2 in
  let check_reassigned_mutable env te =
    if Env.env_local_reactive env
    then Typing_mutability.handle_assignment_mutability env te None
    else env in
  match aexpr with
    | As_v ev ->
      let env, te, _ = assign p env ev ty2 in
      let env = check_reassigned_mutable env te in
      env, T.As_v te
    | Await_as_v (p, ev) ->
      let env, te, _ = assign p env ev ty2 in
      let env = check_reassigned_mutable env te in
      env, T.Await_as_v(p, te)
    | As_kv ((p, ImmutableVar ((_, k) as id)), ev)
    | As_kv ((p, Lvar ((_, k) as id)), ev) ->
      let env, ty1' = set_valid_rvalue p env k ty1 in
      let env, te, _ = assign p env ev ty2 in
      let tk = T.make_typed_expr p ty1' (T.Lvar id) in
      let env = check_reassigned_mutable env tk in
      let env = check_reassigned_mutable env te in
      env, T.As_kv(tk, te)
    | Await_as_kv (p, (p1, ImmutableVar ((_, k) as id)), ev)
    | Await_as_kv (p, (p1, Lvar ((_, k) as id)), ev) ->
      let env, ty1' = set_valid_rvalue p env k ty1 in
      let env, te, _ = assign p env ev ty2 in
      let tk = T.make_typed_expr p1 ty1' (T.Lvar id) in
      let env = check_reassigned_mutable env tk in
      let env = check_reassigned_mutable env te in
      env, T.Await_as_kv(p, tk, te)
    | _ -> (* TODO Probably impossible, should check that *)
      assert false

and expr
    ?expected
    ?(accept_using_var = false)
    ?(is_using_clause = false)
    ?is_func_arg
    ?array_ref_ctx
    ?(valkind = `other)
    ?(check_defined = true)
    env (p, _ as e) =
  try
    begin match expected with
    | None -> ()
    | Some (_, r, ty) ->
      Typing_log.(log_with_level env "typing" 1 (fun () ->
        log_types p env
        [Log_head ("Typing.expr " ^ Typing_reason.string_of_ureason r,
         [Log_type ("expected_ty", ty)])])) end;
    raw_expr ~accept_using_var ~is_using_clause
      ~valkind ~check_defined
      ?is_func_arg ?array_ref_ctx ?expected env e
  with e ->
    let stack = Caml.Printexc.get_raw_backtrace () in
    let pos = Pos.string (Pos.to_absolute p) in
    prerr_endline (Printf.sprintf "Exception while typechecking expression at position %s" pos);
    Caml.Printexc.raise_with_backtrace e stack

and raw_expr
  ?(accept_using_var = false)
  ?(is_using_clause = false)
  ?expected
  ?lhs_of_null_coalesce
  ?is_func_arg
  ?array_ref_ctx
  ?valkind:(valkind=`other)
  ?(check_defined = true)
  env e =
  debug_last_pos := fst e;
  let env, te, ty =
    expr_ ~accept_using_var ~is_using_clause ?expected
      ?lhs_of_null_coalesce ?is_func_arg ?array_ref_ctx
      ~valkind ~check_defined env e in
  let () = match !expr_hook with
    | Some f -> f e (Typing_expand.fully_expand env ty)
    | None -> () in
  env, te, ty

and lvalue env e =
  let valkind = `lvalue in
  expr_ ~valkind ~check_defined:false env e

and lvalues env el =
  match el with
  | [] -> env, [], []
  | e::el ->
    let env, te, ty = lvalue env e in
    let env, tel, tyl = lvalues env el in
    env, te::tel, ty::tyl

and is_pseudo_function s =
  s = SN.PseudoFunctions.hh_show ||
  s = SN.PseudoFunctions.hh_show_env ||
  s = SN.PseudoFunctions.hh_log_level ||
  s = SN.PseudoFunctions.hh_loop_forever

and loop_forever env =
  (* forever = up to 10 minutes, to avoid accidentally stuck processes *)
  for i = 1 to 600 do
    (* Look up things in shared memory occasionally to have a chance to be
     * interrupted *)
    match Env.get_class env "FOR_TEST_ONLY" with
    | None -> Unix.sleep 1;
    | _ -> assert false
  done;
  Utils.assert_false_log_backtrace
    (Some "hh_loop_forever was looping for more than 10 minutes")

(* $x ?? 0 is handled similarly to $x ?: 0, except that the latter will also
 * look for sketchy null checks in the condition. *)
(* TODO TAST: type refinement should be made explicit in the typed AST *)
and eif env ~expected p c e1 e2 =
  let condition = condition ~lhs_of_null_coalesce:false in
  let env, tc, tyc = raw_expr ~lhs_of_null_coalesce:false env c in
  let parent_lenv = env.Env.lenv in

  let env = condition env true tc in
  let env, te1, ty1 = match e1 with
    | None ->
        let env, ty = TUtils.non_null env p tyc in
        env, None, ty
    | Some e1 ->
        let env, te1, ty1 = expr ?expected env e1 in
        env, Some te1, ty1
    in
  let lenv1 = env.Env.lenv in
  let env = { env with Env.lenv = parent_lenv } in
  let env = condition env false tc in
  let env, te2, ty2 = expr ?expected env e2 in
  let lenv2 = env.Env.lenv in
  let fake_members = LEnv.intersect_fake lenv1 lenv2 in
  (* we restore the locals to their parent state so as not to leak the
   * effects of the `condition` calls above *)
  let env = { env with Env.lenv =
              { parent_lenv with Env.fake_members = fake_members } } in
  (* This is a shortened form of what we do in Typing_lenv.union_lenvs. The
   * latter takes local environments as arguments, but our types here
   * aren't assigned to local variables in an environment *)
  (* TODO: Omit if expected type is present and checked in calls to expr *)
  let env, ty = Union.union env ty1 ty2 in
  make_result env p (T.Eif(tc, te1, te2)) ty

and is_parameter env x = Local_id.Map.mem x (Env.get_params env)
and check_escaping_var env (pos, x) =
  if Env.is_using_var env x
  then
    if x = this
    then Errors.escaping_this pos
    else
    if is_parameter env x
    then Errors.escaping_disposable_parameter pos
    else Errors.escaping_disposable pos
  else ()

and exprs
  ?(accept_using_var = false)
  ?is_func_arg
  ?expected
  ?(valkind = `other)
  ?(check_defined = true)
  env el =
  match el with
  | [] ->
    env, [], []

  | e::el ->
    let env, te, ty = expr ~accept_using_var
      ?is_func_arg ?expected ~valkind ~check_defined env e in
    let env, tel, tyl = exprs ~accept_using_var
      ?is_func_arg ?expected ~valkind ~check_defined env el in
    env, te::tel, ty::tyl

and exprs_expected (pos, ur, expected_tyl) env el =
  match el, expected_tyl with
  | [], _ ->
    env, [], []
  | e::el, expected_ty::expected_tyl ->
    let env, te, ty = expr ~expected:(pos, ur, expected_ty) env e in
    let env, tel, tyl = exprs_expected (pos, ur, expected_tyl) env el in
    env, te::tel, ty::tyl
  | el, [] ->
    exprs env el

and make_result env p te ty =
  (* Set the variance of any type variables that were generated according
   * to how they appear in the expression type *)
  let env = Env.set_tyvar_variance env ty in
  env, T.make_typed_expr p ty te, ty

and expr_
  ?expected
  ?(accept_using_var = false)
  ?(is_using_clause = false)
  ?lhs_of_null_coalesce
  ?(is_func_arg = false)
  ?(array_ref_ctx = NoArray)
  ~(valkind: [> `lvalue | `lvalue_subexpr | `other ])
  ~check_defined
  env (p, e) =
  let env = Env.open_tyvars env p in
  (fun (env, te, ty) -> SubType.close_tyvars_and_solve env, te, ty) @@
  let expr = expr ~check_defined in
  let exprs = exprs ~check_defined in
  let raw_expr = raw_expr ~check_defined in

  (**
   * Given a list of types, computes their supertype. If any of the types are
   * unknown (e.g., comes from PHP), the supertype will be Typing_utils.tany env.
   *)
  let compute_supertype ~expected ~reason p env tys =
    let env, supertype =
      match expected with
      | None -> Env.fresh_unresolved_type env p
      | Some (_, _, ty) -> env, ty in
    match supertype with
      (* No need to check individual subtypes if expected type is mixed or any! *)
      | (_, Tany) -> env, supertype
      | _ ->
      let subtype_value env ty = Type.sub_type p reason env ty supertype in
      let env = List.fold_left tys ~init:env ~f:subtype_value in
      if List.exists tys (fun (_, ty) -> ty = Typing_utils.tany env) then
        (* If one of the values comes from PHP land, we have to be conservative
         * and consider that we don't know what the type of the values are. *)
        env, (Reason.Rwitness p, Typing_utils.tany env)
      else
        env, supertype in

  (**
   * Given a 'a list and a method to extract an expr and its ty from a 'a, this
   * function extracts a list of exprs from the list, and computes the supertype
   * of all of the expressions' tys.
   *)
  let compute_exprs_and_supertype ~expected ?(reason = Reason.URarray_value)
    p env l extract_expr_and_ty =
    let env, exprs_and_tys = List.map_env env l (extract_expr_and_ty ~expected) in
    let exprs, tys = List.unzip exprs_and_tys in
    let env, supertype = compute_supertype ~expected ~reason p env tys in
    env, exprs, supertype in

  let forget_fake_members env p callexpr =
    (* Some functions are well known to not change the types of members, e.g.
     * `is_null`.
     * There are a lot of usages like
     *   if (!is_null($x->a) && !is_null($x->a->b))
     * where the second is_null call invalidates the first condition.
     * This function is a bit best effort. Add stuff here when you want
     * To avoid adding too many undue HH_FIXMEs. *)
    match callexpr with
    | _, Id (_, func) when (
      func = SN.StdlibFunctions.is_null ||
      func = SN.PseudoFunctions.isset) -> env
    | _ -> Env.forget_members env p in

  let check_call
    ~is_using_clause ~expected env p call_type e hl el uel ~in_suspend =
    let env, te, result =
      dispatch_call
      ~is_using_clause ~expected p env call_type e hl el uel ~in_suspend in
    let env = forget_fake_members env p e in
    env, te, result in

  try
  match e with
  | Import _
  | Collection _
  | Omitted
  | BracedExpr _
  | ParenthesizedExpr _ -> failwith "AST should not contain these nodes"
  | Any -> expr_error env p (Reason.Rwitness p)
  | Array [] ->
    (* TODO: use expected type to determine expected element type *)
    make_result env p (T.Array []) (Reason.Rwitness p, Tarraykind AKempty)
  | Array (x :: rl as l) ->
      (* True if all fields are values, or all fields are key => value *)
      let fields_consistent = check_consistent_fields x rl in
      let is_vec = match x with
        | Nast.AFvalue _ -> true
        | Nast.AFkvalue _ -> false in
      if fields_consistent && is_vec then
        (* Use expected type to determine expected element type *)
        let env, elem_expected =
          match expand_expected env expected with
          | env, Some (pos, ur, ety) ->
            begin match get_akvec_inst ety with
            | Some vty -> env, Some (pos, ur, vty)
            | None -> env, None
            end
          | _ ->
            env, None in
        let env, tel, arraykind =
          let env, tel, value_ty =
            compute_exprs_and_supertype ~expected:elem_expected p env l array_field_value in
          env, tel, AKvec value_ty in
        make_result env p
          (T.Array (List.map tel (fun e -> T.AFvalue e)))
          (Reason.Rwitness p, Tarraykind arraykind)
      else

      (* TODO TAST: produce a typed expression here *)
      if is_vec
      then
        (* Use expected type to determine expected element type *)
        let env, vexpected =
          match expand_expected env expected with
          | env, Some (pos, ur, ety) ->
            begin match get_akvec_inst ety with
            | Some vty -> env, Some (pos, ur, vty)
            | None -> env, None
            end
          | _ ->
            env, None in
        let env, _value_exprs, value_ty =
          compute_exprs_and_supertype ~expected:vexpected p env l array_field_value in
        make_result env p T.Any (Reason.Rwitness p, Tarraykind (AKvec value_ty))
      else
        (* Use expected type to determine expected element type *)
        let env, kexpected, vexpected =
          match expand_expected env expected with
          | env, Some (pos, ur, ety) ->
            begin match get_akmap_inst ety with
            | Some (kty, vty) -> env, Some (pos, ur, kty), Some (pos, ur, vty)
            | None -> env, None, None
            end
          | _ ->
            env, None, None in
        let env, key_exprs, key_ty =
          compute_exprs_and_supertype ~expected:kexpected p env l array_field_key in
        let env, value_exprs, value_ty =
          compute_exprs_and_supertype ~expected:vexpected p env l array_field_value in
        make_result env p
          (T.Array (List.map (List.zip_exn key_exprs value_exprs)
            (fun (tek, tev) -> T.AFkvalue (tek, tev))))
          (Reason.Rwitness p, Tarraykind (AKmap (key_ty, value_ty)))

  | Darray (th, l) ->
      (* Use expected type to determine expected key and value types *)
      let env, kexpected, vexpected =
        match th with
        | Some (
          ((pk, _) as tk),
          ((pv, _) as tv)
        ) when not (TCO.ignore_collection_expr_type_arguments (Env.get_tcopt env)) ->
          let env, localtk = resolve_type_argument env tk in
          let env, localtv = resolve_type_argument env tv in
          env, Some (pk, Reason.URhint, localtk), Some (pv, Reason.URhint, localtv)
        | _ -> (* no explicit typehint, fallback to supplied expect *)
          begin match expand_expected env expected with
            | env, Some (pos, ur, ety) ->
              begin match get_darray_inst ety with
              | Some (kty, vty) ->
                env, Some (pos, ur, kty), Some (pos, ur, vty)
              | None ->
                env, None, None
              end
            | _ ->
              env, None, None
          end in
      let keys, values = List.unzip l in
      let env, value_exprs, value_ty =
        compute_exprs_and_supertype ~expected:vexpected p env values array_value in
      let env, key_exprs, key_ty =
        compute_exprs_and_supertype ~expected:kexpected p env keys
          (arraykey_value p "darray") in
      let field_exprs = List.zip_exn key_exprs value_exprs in
      make_result env p
        (T.Darray (th, field_exprs))
        (Reason.Rwitness p, Tarraykind (AKdarray (key_ty, value_ty)))

  | Varray (th, values) ->
      (* Use expected type to determine expected element type *)
      let env, elem_expected =
        match th with
        | Some ((pv, _) as tv)
          when not (TCO.ignore_collection_expr_type_arguments (Env.get_tcopt env)) ->
          let env, localtv = resolve_type_argument env tv in
          env, Some (pv, Reason.URhint, localtv)
        | _ -> (* no explicit typehint, fallback to supplied expect *)
          begin match expand_expected env expected with
          | env, Some (pos, ur, ety) ->
            begin match get_varray_inst ety with
            | Some vty -> env, Some (pos, ur, vty)
            | _ -> env, None
            end
          | _ -> env, None
          end
        in
      let env, value_exprs, value_ty =
        compute_exprs_and_supertype ~expected:elem_expected p env values array_value in
      make_result env p
        (T.Varray (th, value_exprs))
        (Reason.Rwitness p, Tarraykind (AKvarray value_ty))

  | ValCollection (kind, th, el) ->
      (* Use expected type to determine expected element type *)
      let env, elem_expected =
        match th with
        | Some ((pv, _) as tv)
          when not (TCO.ignore_collection_expr_type_arguments (Env.get_tcopt env)) ->
          let env, localtv = resolve_type_argument env tv in
          env, Some (pv, Reason.URhint, localtv)
        | _ ->
          begin match expand_expected env expected with
          | env, Some (pos, ur, ety) ->
            begin match get_vc_inst kind ety with
            | Some vty -> env, Some (pos, ur, vty)
            | None -> env, None
            end
          | _ -> env, None
          end in
      let class_name = vc_kind_to_name kind in
      let subtype_val =
        match kind with
        | `Set | `ImmSet | `Keyset ->
          arraykey_value p class_name
        | `Vector | `ImmVector | `Vec | `Pair ->
          array_value in

      let env, tel, elem_ty =
        compute_exprs_and_supertype ~expected:elem_expected ~reason:Reason.URvector
          p env el subtype_val in
      let ty = MakeType.class_type (Reason.Rwitness p) class_name [elem_ty] in
      make_result env p (T.ValCollection (kind, th, tel)) ty
  | KeyValCollection (kind, th, l) ->
      (* Use expected type to determine expected key and value types *)
      let env, kexpected, vexpected =
        match th with
        | Some (
          ((pk, _) as tk),
          ((pv, _) as tv)
        ) when not (TCO.ignore_collection_expr_type_arguments (Env.get_tcopt env)) ->
          let env, localtk = resolve_type_argument env tk in
          let env, localtv = resolve_type_argument env tv in
          env, Some (pk, Reason.URhint, localtk), Some (pv, Reason.URhint, localtv)
        | _ -> (* no explicit typehint, fallback to supplied expect *)
          begin match expand_expected env expected with
          | env, Some (pos, ur, ety) ->
            begin match get_kvc_inst kind ety with
            | Some (kty, vty) ->
              env, Some (pos, ur, kty), Some (pos, ur, vty)
            | None -> env, None, None
            end
          | _ -> env, None, None
          end in
      let kl, vl = List.unzip l in
      let class_name = kvc_kind_to_name kind in
      let env, tkl, k =
        compute_exprs_and_supertype ~expected:kexpected ~reason:Reason.URkey
          p env kl (arraykey_value p class_name) in
      let env, tvl, v =
        compute_exprs_and_supertype ~expected:vexpected ~reason:Reason.URvalue
          p env vl array_value in
      let ty = MakeType.class_type (Reason.Rwitness p) class_name [k; v] in
      make_result env p (T.KeyValCollection (kind, th, List.zip_exn tkl tvl)) ty
  | Clone e ->
    let env, te, ty = expr env e in
    (* Clone only works on objects; anything else fatals at runtime *)
    let tobj = (Reason.Rwitness p, Tobject) in
    let env = Type.sub_type p Reason.URclone env ty tobj in
    make_result env p (T.Clone te) ty
  | This ->
      let r, _ = Env.get_self env in
      if r = Reason.Rnone
      then Errors.this_var_outside_class p;
      if not accept_using_var
      then check_escaping_var env (p,this);
      let (_, ty) = Env.get_local env this in
      let r = Reason.Rwitness p in
      let ty = r, TUtils.this_of (r, ty) in
      make_result env p T.This ty
  | Assert (AE_assert e) ->
      let env, te, _ = expr env e in
      let env = LEnv.save_and_merge_next_in_cont env C.Exit in
      let env = condition env true te in
      make_result env p (T.Assert (T.AE_assert te))
        (MakeType.void (Reason.Rwitness p))
  | True ->
      make_result env p T.True (MakeType.bool (Reason.Rwitness p))
  | False ->
      make_result env p T.False (MakeType.bool (Reason.Rwitness p))
    (* TODO TAST: consider checking that the integer is in range. Right now
     * it's possible for HHVM to fail on well-typed Hack code
     *)
  | Int s ->
      make_result env p (T.Int s) (MakeType.int (Reason.Rwitness p))
  | Float s ->
      make_result env p (T.Float s) (MakeType.float (Reason.Rwitness p))
    (* TODO TAST: consider introducing a "null" type, and defining ?t to
     * be null | t
     *)
  | Null ->
      make_result env p T.Null (MakeType.null (Reason.Rwitness p))
  | String s ->
      make_result env p (T.String s) (MakeType.string (Reason.Rwitness p))
  | String2 idl ->
      let env, tel = string2 env idl in
      make_result env p (T.String2 tel) (MakeType.string (Reason.Rwitness p))
  | PrefixedString (n, e) ->
      if n <> "re"
      then begin
        Errors.experimental_feature p
          "String prefixes other than `re` are not yet supported.";
        expr_error env p (Reason.Rnone)
      end else
        let env, te, ty = expr env e in
        let pe = fst e in
        let env = SubType.sub_string pe env ty in
        (match snd e with
        | String _ ->
            begin try make_result env p (T.PrefixedString (n, te))
              (Typing_regex.type_pattern e)
            with
              | Pcre.Error (Pcre.BadPattern (s, i)) ->
                let s = s ^ " [" ^ (string_of_int i) ^ "]" in
                Errors.bad_regex_pattern pe s;
                expr_error env pe (Reason.Rregex pe)
              | Typing_regex.Empty_regex_pattern ->
                Errors.bad_regex_pattern pe "This pattern is empty";
                expr_error env pe (Reason.Rregex pe)
              | Typing_regex.Missing_delimiter ->
                Errors.bad_regex_pattern pe "Missing delimiter(s)";
                expr_error env pe (Reason.Rregex pe)
              | Typing_regex.Invalid_global_option ->
                Errors.bad_regex_pattern pe "Invalid global option(s)";
                expr_error env pe (Reason.Rregex pe)
            end
        | String2 _ ->
          Errors.re_prefixed_non_string pe "Strings with embedded expressions";
          expr_error env pe (Reason.Rregex pe)
        | _ ->
          Errors.re_prefixed_non_string pe "Non-strings";
          expr_error env pe (Reason.Rregex pe))
  | Fun_id x ->
      let env, fty = fun_type_of_id env x [] in
      begin match fty with
      | _, Tfun fty -> check_deprecated (fst x) fty;
      | _ -> ()
      end;
      make_result env p (T.Fun_id x) fty
  | Id ((cst_pos, cst_name) as id) ->
      (match Env.get_gconst env cst_name with
      | None when Env.is_strict env ->
          Errors.unbound_global cst_pos;
          let ty = (Reason.Rwitness cst_pos, Typing_utils.terr env) in
          make_result env cst_pos (T.Id id) ty
      | None ->
          make_result env p (T.Id id) (Reason.Rwitness cst_pos, Typing_utils.tany env)
      | Some (ty, _) ->
        let env, ty =
          Phase.localize_with_self env ty in
        make_result env p (T.Id id) ty
      )
  | Method_id (instance, meth) ->
    (* Method_id is used when creating a "method pointer" using the magic
     * inst_meth function.
     *
     * Typing this is pretty simple, we just need to check that instance->meth
     * is public+not static and then return its type.
     *)
    let env, te, ty1 = expr env instance in
    let env, result, vis =
      obj_get_with_visibility ~obj_pos:p ~is_method:true ~nullsafe:None
        ~valkind:`other ~pos_params:None env ty1 (CIexpr instance) meth (fun x -> x) in
    let has_lost_info = Env.FakeMembers.is_invalid env instance (snd meth) in
    if has_lost_info
    then
      let name = "the method "^snd meth in
      let env, result = Env.lost_info name env result in
      make_result env p (T.Method_id (te, meth)) result
    else
      begin
        (match result with
        | _, Tfun fty -> check_deprecated p fty
        | _ -> ());
        (match vis with
        | Some (method_pos, Vprivate _) ->
            Errors.private_inst_meth method_pos p
        | Some (method_pos, Vprotected _) ->
            Errors.protected_inst_meth method_pos p
        | _ -> ()
        );
        make_result env p (T.Method_id (te, meth)) result
      end
  | Method_caller ((pos, class_name) as pos_cname, meth_name) ->
    (* meth_caller('X', 'foo') desugars to:
     * $x ==> $x->foo()
     *)
    let class_ = Env.get_class env class_name in
    (match class_ with
    | None -> unbound_name env pos_cname
    | Some class_ ->
       (* Create a class type for the given object instantiated with unresolved
        * types for its type parameters.
        *)
        let env, tvarl =
          List.map_env env (Cls.tparams class_) (fun env _ ->
            TUtils.unresolved_tparam ~reason:(Reason.Rtype_variable p) env) in
        let params = List.map (Cls.tparams class_) begin fun { tp_name = (p,n); _ } ->
          Reason.Rwitness p, Tgeneric n
        end in
        let obj_type = Reason.Rwitness p, Tapply (pos_cname, params) in
        let ety_env = {
          (Phase.env_with_self env) with
          substs = Subst.make (Cls.tparams class_) tvarl;
        } in
        let env, local_obj_ty = Phase.localize ~ety_env env obj_type in
        let env, fty =
          obj_get ~obj_pos:pos ~is_method:true ~nullsafe:None env local_obj_ty
                 (CI (pos, class_name)) meth_name (fun x -> x) in
        (match fty with
        | reason, Tfun fty ->
            check_deprecated p fty;
            (* We are creating a fake closure:
             * function(Class $x, arg_types_of(Class::meth_name))
                 : return_type_of(Class::meth_name)
             *)
            let ety_env = {
              ety_env with substs = Subst.make (Cls.tparams class_) tvarl
            } in
            let env =
              Phase.check_tparams_constraints ~use_pos:p ~ety_env env (Cls.tparams class_) in
            let env, local_obj_ty = Phase.localize ~ety_env env obj_type in
            let local_obj_fp = TUtils.default_fun_param local_obj_ty in
            let fty = { fty with
                        ft_params = local_obj_fp :: fty.ft_params } in
            let fun_arity = match fty.ft_arity with
              | Fstandard (min, max) -> Fstandard (min + 1, max + 1)
              | Fvariadic (min, x) -> Fvariadic (min + 1, x)
              | Fellipsis (min, p) -> Fellipsis (min + 1, p) in
            let caller = {
              ft_pos = pos;
              ft_deprecated = None;
              ft_abstract = false;
              (* propagate 'is_coroutine' from the method being called*)
              ft_is_coroutine = fty.ft_is_coroutine;
              ft_arity = fun_arity;
              ft_tparams = fty.ft_tparams;
              ft_where_constraints = fty.ft_where_constraints;
              ft_params = fty.ft_params;
              ft_ret = fty.ft_ret;
              ft_reactive = fty.ft_reactive;
              ft_mutability = fty.ft_mutability;
              ft_returns_mutable = fty.ft_returns_mutable;
              ft_return_disposable = fty.ft_return_disposable;
              ft_decl_errors = None;
              ft_returns_void_to_rx = fty.ft_returns_void_to_rx;
            } in
            make_result env p (T.Method_caller(pos_cname, meth_name))
              (reason, Tfun caller)
        | _ ->
            (* This can happen if the method lives in PHP *)
            make_result env p (T.Method_caller(pos_cname, meth_name))
              (Reason.Rwitness pos, Typing_utils.tany env)
        )
    )
  | Smethod_id (c, meth) ->
    (* Smethod_id is used when creating a "method pointer" using the magic
     * class_meth function.
     *
     * Typing this is pretty simple, we just need to check that c::meth is
     * public+static and then return its type.
     *)
    let class_ = Env.get_class env (snd c) in
    (match class_ with
    | None ->
      (* The class given as a static string was not found. *)
      unbound_name env c
    | Some class_ ->
      let smethod = Env.get_static_member true env class_ (snd meth) in
      (match smethod with
      | None -> (* The static method wasn't found. *)
        smember_not_found p ~is_const:false ~is_method:true class_ (snd meth);
        expr_error env p Reason.Rnone
      | Some { ce_type = lazy ty; ce_visibility; _ } ->
        let cid = CI c in
        let env, _te, cid_ty =
          static_class_id ~check_constraints:true (fst c) env [] cid in
        let tyargs =
          match cid_ty with
          | (_, Tclass(_, _, tyargs)) -> tyargs
          | _ -> [] in
        let ety_env = {
          type_expansions = [];
          substs = Subst.make (Cls.tparams class_) tyargs;
          this_ty = cid_ty;
          from_class = Some cid;
          validate_dty = None;
        } in
        match ty with
        | (r, Tfun ft) ->
          begin
            let env, ft = Phase.localize_ft ~use_pos:p ~ety_env env ft in
            let ty = r, Tfun ft in
            check_deprecated p ft;
            match ce_visibility with
            | Vpublic ->
              make_result env p (T.Smethod_id(c, meth)) ty
            | Vprivate _ ->
              Errors.private_class_meth (Reason.to_pos r) p;
              expr_error env p r
            | Vprotected _ ->
              Errors.protected_class_meth (Reason.to_pos r) p;
              expr_error env p r
          end
        | (r, _) ->
          Errors.internal_error p "We have a method which isn't callable";
          expr_error env p r
      )
    )
  | Lplaceholder p ->
      let r = Reason.Rplaceholder p in
      let ty = MakeType.void r in
      make_result env p (T.Lplaceholder p) ty
  | Dollardollar _ when valkind = `lvalue ->
      Errors.dollardollar_lvalue p;
      expr_error env p (Reason.Rwitness p)
  | Dollardollar id ->
      let ty = Env.get_local_check_defined env id in
      let env = might_throw env in
      make_result env p (T.Dollardollar id) ty
  | Lvar ((_, x) as id) ->
      if not accept_using_var
      then check_escaping_var env id;
      let ty = if check_defined
        then Env.get_local_check_defined env id
        else Env.get_local env x in
      make_result env p (T.Lvar id) ty
  | ImmutableVar ((_, x) as id) ->
    let ty = Env.get_local env x in
    make_result env p (T.ImmutableVar id) ty
  | List el ->
      let env, tel, tyl = match valkind with
        | `lvalue | `lvalue_subexpr -> lvalues env el
        | `other ->
          let env, expected = expand_expected env expected in
          match expected with
          | Some (pos, ur, (_, Ttuple expected_tyl)) ->
            exprs_expected (pos, ur, expected_tyl) env el
          | _ ->
            exprs env el
      in
      (* TODO TAST: figure out role of unbind here *)
      let env, tyl = List.map_env env tyl Typing_env.unbind in
      let env, tyl = List.map_env env tyl TUtils.unresolved in
      let ty = Reason.Rwitness p, Ttuple tyl in
      make_result env p (T.List tel) ty
  | Pair (e1, e2) ->
      (* Use expected type to determine expected element types *)
      let env, expected1, expected2 =
        match expand_expected env expected with
        | env, Some (pos, ur, (_, Tclass ((_, k), _, [ty1; ty2]))) when k = SN.Collections.cPair ->
          env, Some (pos, ur, ty1), Some (pos, ur, ty2)
        | _ -> env, None, None in
      let env, te1, ty1 = expr ?expected:expected1 env e1 in
      let env, ty1 = Typing_env.unbind env ty1 in
      let env, ty1 = TUtils.unresolved env ty1 in
      let env, te2, ty2 = expr ?expected:expected2 env e2 in
      let env, ty2 = Typing_env.unbind env ty2 in
      let env, ty2 = TUtils.unresolved env ty2 in
      let ty = MakeType.pair (Reason.Rwitness p) ty1 ty2 in
      make_result env p (T.Pair (te1, te2)) ty
  | Expr_list el ->
    (* TODO: use expected type to determine tuple component types *)
      let env, tel, tyl = exprs env el in
      let ty = Reason.Rwitness p, Ttuple tyl in
      make_result env p (T.Expr_list tel) ty
  | Array_get (e, None) ->
      let env, te, _ = update_array_type p env e None valkind in
      let env = might_throw env in
      (* NAST check reports an error if [] is used for reading in an
         lvalue context. *)
      let ty = (Reason.Rwitness p, Typing_utils.terr env) in
      make_result env p (T.Array_get (te, None)) ty
  | Array_get (e1, Some e2) ->
      let env, te1, ty1 =
        update_array_type ?lhs_of_null_coalesce p env e1 (Some e2) valkind in
      let env, ty1 = TUtils.fold_unresolved env ty1 in
      let env, te2, ty2 = expr env e2 in
      let env = might_throw env in
      let is_lvalue = phys_equal valkind `lvalue in
      let env, ty =
        Typing_array_access.array_get ~array_pos:(fst e1) ~expr_pos:p ?lhs_of_null_coalesce
          is_lvalue env ty1 e2 ty2 in
      make_result env p (T.Array_get(te1, Some te2)) ty
  | Call (Cnormal, (pos_id, Id ((_, s) as id)), hl, el, [])
      when is_pseudo_function s ->
      let env, tel, tys = exprs ~accept_using_var:true env el in
      let env =
        if s = SN.PseudoFunctions.hh_show
        then (List.iter tys (Typing_log.hh_show p env); env)
        else
        if s = SN.PseudoFunctions.hh_show_env
        then (Typing_log.hh_show_env p env; env)
        else
        if s = SN.PseudoFunctions.hh_log_level
        then match el with
          | [(_, String key_str); (_, Int level_str)] ->
            Env.set_log_level env key_str (int_of_string level_str)
          | _ -> env
        else
        if s = SN.PseudoFunctions.hh_loop_forever then (loop_forever env; env)
        else env in
      let env, ty = Env.fresh_type env p in
      make_result env p
        (T.Call(
          Cnormal,
          T.make_typed_expr pos_id (Reason.Rnone, TUtils.tany env) (T.Id id),
          hl,
          tel,
          [])) ty
  | Call (call_type, e, hl, el, uel) ->
      let env = might_throw env in
      let env, te, ty = check_call ~is_using_clause ~expected
        env p call_type e hl el uel ~in_suspend:false in
      env, te, ty
  | Binop (Ast.QuestionQuestion, e1, e2) ->
      let env, te1, ty1 = raw_expr ~lhs_of_null_coalesce:true env e1 in
      let env, te2, ty2 = expr ?expected env e2 in
      let env, ty1' = Env.fresh_unresolved_type env (fst e1) in
      let env = SubType.sub_type env ty1 (MakeType.nullable Reason.Rnone ty1') in
      let env, ty_result =
        if TypecheckerOptions.new_inference (Env.get_tcopt env)
        then
        (* Essentially mimic a call to
         *   function coalesce<Tr, Ta as Tr, Tb as Tr>(?Ta, Tb): Tr
         * That way we let the constraint solver take care of the union logic.
         *)
        let env, ty_result = Env.fresh_unresolved_type env (fst e2) in
        let env = SubType.sub_type env ty1' ty_result in
        let env = SubType.sub_type env ty2 ty_result in
        env, ty_result
      else
        Union.union env ty1' ty2 in
      make_result env p (T.Binop (Ast.QuestionQuestion, te1, te2)) ty_result
  (* For example, e1 += e2. This is typed and translated as if
   * written e1 = e1 + e2.
   * TODO TAST: is this right? e1 will get evaluated more than once
   *)
  | Binop (Ast.Eq (Some op), e1, e2) ->
      begin match op, snd e1 with
      | Ast.QuestionQuestion, Class_get _ ->
        Errors.experimental_feature p
          "null coalesce assignment operator with static properties";
        expr_error env p (Reason.Rnone)
      | _ ->
      let e_fake = (p, Binop (Ast.Eq None, e1, (p, Binop (op, e1, e2)))) in
      let env, te_fake, ty = raw_expr env e_fake in
      begin match snd te_fake with
        | T.Binop (_, te1, (_, T.Binop (_, _, te2))) ->
          let te = T.Binop (Ast.Eq (Some op), te1, te2) in
          make_result env p te ty
        | _ -> assert false
      end
      end
  | Binop (Ast.Eq None, e1, e2) ->
     let array_ref_ctx = match e1, e2 with
        | (_, Array_get _), (_, Unop (Ast.Uref, _)) -> ElementAssignment
        | _, (_, Unop (Ast.Uref, (_, Array_get _))) -> ElementAccess
        | _ -> NoArray in
      begin match e1 with
        | _, ImmutableVar (p, x) ->
          Errors.let_var_immutability_violation p (Local_id.get_name x)
        | _ -> ()
      end;
      let env, te2, ty2 = raw_expr ~array_ref_ctx env e2 in
      let env, te1, ty = assign p env e1 ty2 in
      let env =
        if Env.env_local_reactive env then
        Typing_mutability.handle_assignment_mutability env te1 (Some (snd te2))
        else env
      in
      (* If we are assigning a local variable to another local variable then
       * the expression ID associated with e2 is transferred to e1
       *)
      (match e1, e2 with
      | (_, Lvar (_, x1)), (_, ImmutableVar (_, x2))
      | (_, Lvar (_, x1)), (_, Lvar (_, x2)) ->
          let eid2 = Env.get_local_expr_id env x2 in
          let env =
            Option.value_map
              eid2 ~default:env
              ~f:(Env.set_local_expr_id env x1) in
          make_result env p (T.Binop(Ast.Eq None, te1, te2)) ty
      | _ ->
          make_result env p (T.Binop(Ast.Eq None, te1, te2)) ty
      )
  | Binop ((Ast.Ampamp | Ast.Barbar as bop), e1, e2) ->
      let c = bop = Ast.Ampamp in
      let env, te1, _ = expr env e1 in
      let lenv = env.Env.lenv in
      let env = condition env c te1 in
      let env, te2, _ = expr env e2 in
      let env = { env with Env.lenv = lenv } in
      make_result env p (T.Binop(bop, te1, te2)) (MakeType.bool (Reason.Rlogic_ret p))
  | Binop (bop, e1, e2) when Env.is_strict env
                        && (snd e1 = Nast.Null || snd e2 = Nast.Null)
                        && (bop = Ast.Eqeqeq || bop = Ast.Diff2) ->
      let e, ne = if snd e2 = Nast.Null then e1, e2 else e2, e1 in
      let env, te, ty = raw_expr env e in
      let tne = T.make_typed_expr (fst ne) ty T.Null in
      let te1, te2 = if snd e2 = Nast.Null then te, tne else tne, te in
      make_result env p (T.Binop(bop, te1, te2)) (MakeType.bool (Reason.Rcomp p))
  | Binop (bop, e1, e2) ->
      let env, te1, ty1 = raw_expr env e1 in
      let env, te2, ty2 = raw_expr env e2 in
      let env = might_throw env in
      let env, te3, ty =
        binop p env bop (fst e1) te1 ty1 (fst e2) te2 ty2 in
      env, te3, ty
  | Pipe (e0, e1, e2) ->
      let env, te1, ty = expr env e1 in
      (** id is the ID of the $$ that is implicitly declared by the pipe.
       * Set the local type for the $$ in the RHS. *)
      let env = set_local env e0 ty in
      let env, te2, ty2 = expr env e2 in
      (**
       * Return ty2 since the type of the pipe expression is the type of the
       * RHS.
       *
       * Note: env does have the type of this Pipe's $$, but it doesn't
       * override the outer one since they have different ID's.
       *
       * For example:
       *   a() |> ( inner1($$) |> inner2($$) ) + $$
       *
       *   The rightmost $$ refers to the result of a()
       *)
      make_result env p (T.Pipe(e0, te1, te2)) ty2
  | Unop (uop, e) ->
      let env, te, ty = raw_expr env e in
      let env = might_throw env in
      unop ~is_func_arg ~array_ref_ctx p env uop te ty
  | Eif (c, e1, e2) -> eif env ~expected p c e1 e2
  | Typename sid ->
      begin match Env.get_typedef env (snd sid) with
        | Some {td_tparams = tparaml; _} ->
            (* Typedef type parameters cannot have constraints *)
            let params = List.map ~f:begin fun { tp_name = (p,x); _ } ->
              Reason.Rwitness p, Tgeneric x
            end tparaml in
            let tdef = Reason.Rwitness (fst sid), Tapply (sid, params) in
            let typename =
              Reason.Rwitness p, Tapply((p, SN.Classes.cTypename), [tdef]) in
            let env, tparams = List.map_env env tparaml begin fun env tp ->
              Env.fresh_unresolved_type env (fst tp.tp_name)
            end in
            let ety_env = { (Phase.env_with_self env) with
                            substs = Subst.make tparaml tparams } in
            let env = Phase.check_tparams_constraints ~use_pos:p ~ety_env env tparaml in
            let env, ty = Phase.localize ~ety_env env typename in
            make_result env p (T.Typename sid) ty
        | None ->
            (* Should never hit this case since we only construct this AST node
             * if in the expression Foo::class, Foo is a type def.
             *)
            expr_error env p (Reason.Rwitness p)
      end
  | Class_const (cid, mid) -> class_const env p (cid, mid)
  | Class_get ((px, x), CGstring (py, y))
      when Env.FakeMembers.get_static env x y <> None ->
        let env, local = Env.FakeMembers.make_static p env x y in
        let local = p, Lvar (p, local) in
        let env, _, ty = expr env local in
        let env, te, _ = static_class_id ~check_constraints:false px env [] x in
        make_result env p (T.Class_get (te, T.CGstring (py, y))) ty
  | Class_get ((cpos, cid), CGstring mid) ->
      let env, te, cty = static_class_id ~check_constraints:false cpos env [] cid in
      let env = might_throw env in
      let env, ty, _ =
        class_get ~is_method:false ~is_const:false env cty mid cid in
      if Env.FakeMembers.is_static_invalid env cid (snd mid)
      then
        let fake_name = Env.FakeMembers.make_static_id cid (snd mid) in
        let env, ty = Env.lost_info fake_name env ty in
        make_result env p (T.Class_get (te, T.CGstring mid)) ty
      else
        make_result env p (T.Class_get (te, T.CGstring mid)) ty
    (* Fake member property access. For example:
     *   if ($x->f !== null) { ...$x->f... }
     *)
  | Class_get (_, CGexpr _) -> failwith "AST should not have any CGexprs after naming"
  | Obj_get (e, (pid, Id (py, y)), nf)
    when Env.FakeMembers.get env e y <> None ->
      let env = might_throw env in
      let env, local = Env.FakeMembers.make p env e y in
      let local = p, Lvar (p, local) in
      let env, _, ty = expr env local in
      let env, t_lhs, _ = expr ~accept_using_var:true env e in
      let t_rhs = T.make_typed_expr pid ty (T.Id (py, y)) in
      make_result env p (T.Obj_get (t_lhs, t_rhs, nf)) ty
    (* Statically-known instance property access e.g. $x->f *)
  | Obj_get (e1, (pm, Id m), nullflavor) ->
      let nullsafe =
        (match nullflavor with
          | OG_nullthrows -> None
          | OG_nullsafe -> Some p
        ) in
      let env, te1, ty1 = expr ~accept_using_var:true env e1 in
      let env = might_throw env in
      let env, result =
        obj_get ~obj_pos:(fst e1) ~is_method:false ~nullsafe ~valkind
          env ty1 (CIexpr e1) m (fun x -> x) in
      let has_lost_info = Env.FakeMembers.is_invalid env e1 (snd m) in
      let env, result =
        if has_lost_info
        then
          let name = "the member " ^ snd m in
          Env.lost_info name env result
        else
          env, result
      in
      make_result env p (T.Obj_get(te1,
        T.make_typed_expr pm result (T.Id m), nullflavor)) result
    (* Dynamic instance property access e.g. $x->$f *)
  | Obj_get (e1, e2, nullflavor) ->
    let env, te1, ty1 = expr ~accept_using_var:true env e1 in
    let env, te2, _ = expr env e2 in
    let ty = if TUtils.is_dynamic env ty1 then
      MakeType.dynamic (Reason.Rwitness p) else
      (Reason.Rwitness p, Typing_utils.tany env)
    in
    let (pos, _), te2 = te2 in
    let env = might_throw env in
    let te2 = T.make_typed_expr pos ty te2 in
    make_result env p (T.Obj_get(te1, te2, nullflavor)) ty
  | Yield_break ->
      make_result env p T.Yield_break (Reason.Rwitness p, Typing_utils.tany env)
  | Yield af ->
      let env, (taf, opt_key, value) = array_field env af in
      let env, send = Env.fresh_type env p in
      let env, key = match af, opt_key with
        | Nast.AFvalue (p, _), None ->
          begin match Env.get_fn_kind env with
          | Ast.FCoroutine
          | Ast.FSync
          | Ast.FAsync ->
            Errors.internal_error p "yield found in non-generator";
            env, (Reason.Rwitness p, Typing_utils.tany env)
          | Ast.FGenerator ->
            env, MakeType.int (Reason.Rwitness p)
          | Ast.FAsyncGenerator ->
            let env, ty = Env.fresh_type env p in
            env, MakeType.nullable (Reason.Ryield_asyncnull p) ty
          end
        | _, Some x ->
            env, x
        | _, _ -> assert false in
      let rty = match Env.get_fn_kind env with
        | Ast.FCoroutine ->
            (* yield in coroutine is already reported as error in NastCheck *)
            let _, _, ty = expr_error env p (Reason.Rwitness p) in
            ty
        | Ast.FGenerator ->
            MakeType.generator (Reason.Ryield_gen p) key value send
        | Ast.FAsyncGenerator ->
            MakeType.async_generator (Reason.Ryield_asyncgen p) key value send
        | Ast.FSync | Ast.FAsync ->
            failwith "Parsing should never allow this" in
      let Typing_env_return_info.{ return_type = expected_return; _ } = Env.get_return env in
      let env =
        Type.coerce_type p (Reason.URyield) env rty expected_return in
      let env = Env.forget_members env p in
      let env = LEnv.save_and_merge_next_in_cont env C.Exit in
      make_result env p (T.Yield taf) (MakeType.nullable (Reason.Ryield_send p) send)
  | Yield_from e ->
    let env, key = Env.fresh_type env p in
    let env, value = Env.fresh_type env p in
    let env, te, yield_from_ty =
      expr ~is_using_clause env e in
      (* Expected type of `e` in `yield from e` is KeyedTraversable<Tk,Tv> (but might be dynamic)*)
    let expected_yield_from_ty = MakeType.keyed_traversable (Reason.Ryield_gen p) key value in
    let from_dynamic = SubType.is_sub_type env yield_from_ty (MakeType.dynamic (fst yield_from_ty)) in
    let env =
      if from_dynamic
      then env (* all set if dynamic, otherwise need to check against KeyedTraversable *)
      else Type.coerce_type p Reason.URyield_from env yield_from_ty expected_yield_from_ty in
    let rty = match Env.get_fn_kind env with
      | Ast.FCoroutine ->
        (* yield in coroutine is already reported as error in NastCheck *)
        let _, _, ty = expr_error env p (Reason.Rwitness p) in
        ty
      | Ast.FGenerator ->
        if from_dynamic
        then MakeType.dynamic (Reason.Ryield_gen p) (*TODO: give better reason*)
        else MakeType.generator (Reason.Ryield_gen p) key value (MakeType.void (Reason.Rwitness p))
      | Ast.FSync | Ast.FAsync | Ast.FAsyncGenerator ->
        failwith "Parsing should never allow this" in
    let Typing_env_return_info.{ return_type = expected_return; _ } = Env.get_return env in
    let env =
      Type.coerce_type p (Reason.URyield_from) env rty expected_return in
    let env = Env.forget_members env p in
    make_result env p (T.Yield_from te) (MakeType.void (Reason.Rwitness p))
  | Await e ->
      let env = might_throw env in
      (* Await is permitted in a using clause e.g. using (await make_handle()) *)
      let env, te, rty =
        expr ~is_using_clause env e in
      let env, ty = Async.overload_extract_from_awaitable env p rty in
      make_result env p (T.Await te) ty
  | Suspend (e) ->
      let env, te, ty =
        match e with
        | _, Call (call_type, e, hl, el, uel) ->
          let env = Env.open_tyvars env p in
          (fun (env, te, ty) -> SubType.close_tyvars_and_solve env, te, ty) @@
          check_call ~is_using_clause ~expected
            env p call_type e hl el uel ~in_suspend:true
        | (epos, _)  ->
          let env, te, ty = expr env e in
          (* not a call - report an error *)
          Errors.non_call_argument_in_suspend
            epos
            (Reason.to_string ("This is " ^ Typing_print.error env ty) (fst ty));
          env, te, ty in
      make_result env p (T.Suspend te) ty

  | Special_func func -> special_func env p func
  | New ((pos, c), tal, el, uel, p1) ->
      let env = might_throw env in
      let env, tc, tel, tuel, ty, ctor_fty =
        new_object ~expected ~is_using_clause ~check_parent:false ~check_not_abstract:true
          pos env c tal el uel in
      let env = Env.forget_members env p in
      make_result env p (T.New(tc, tal, tel, tuel, (p1, ctor_fty))) ty
  | Record _ ->
    failwith "Record - not implemented"
  | Cast ((_, Harray (None, None)), _)
    when Env.is_strict env
    || TCO.migration_flag_enabled (Env.get_tcopt env) "array_cast" ->
      Errors.array_cast p;
      expr_error env p (Reason.Rwitness p)
  | Cast (hint, e) ->
      let env, te, ty2 = expr env e in
      let env = might_throw env in
      if (TypecheckerOptions.experimental_feature_enabled
        (Env.get_tcopt env)
        TypecheckerOptions.experimental_forbid_nullable_cast)
        && TUtils.is_option_non_mixed env ty2
      then Errors.nullable_cast p (Typing_print.error env ty2) (Reason.to_pos (fst ty2));
      let env, ty = Phase.localize_hint_with_self env hint in
      make_result env p (T.Cast (hint, te)) ty
  | InstanceOf (e, (pos, cid)) ->
      let env, te, _ = expr env e in
      let env, te2, _class = instantiable_cid pos env cid [] in
      make_result env p (T.InstanceOf (te, te2)) (MakeType.bool (Reason.Rwitness p))
  | Is (e, hint) ->
    let env, te, _ = expr env e in
    make_result env p (T.Is (te, hint)) (MakeType.bool (Reason.Rwitness p))
  | As (e, hint, is_nullable) ->
    let refine_type env lpos lty rty =
      let reason = Reason.Ras lpos in
      let env, rty = Env.expand_type env rty in
      if snd rty <> Tdynamic && SubType.is_sub_type env lty rty
      then env, lty
      else safely_refine_type env p reason lpos lty rty
    in
    let env, te, expr_ty = expr env e in
    let env = might_throw env in
    let ety_env = { (Phase.env_with_self env) with from_class = Some CIstatic; } in
    let env, hint_ty = Phase.localize_hint ~ety_env env hint in
    let env, hint_ty =
      if is_nullable then
        let env, hint_ty = refine_type env (fst e) expr_ty hint_ty in
        env, MakeType.nullable (Reason.Rwitness p) hint_ty
      else if is_instance_var e then
        let env, _, ivar_ty = raw_expr env e in
        let env, ((ivar_pos, _) as ivar) = get_instance_var env e in
        let env, hint_ty = refine_type env ivar_pos ivar_ty hint_ty in
        let env = set_local env ivar hint_ty in
        env, hint_ty
      else
        refine_type env (fst e) expr_ty hint_ty
    in
    make_result env p (T.As (te, hint, is_nullable)) hint_ty
  | Efun (f, idl)
  | Lfun (f, idl) ->
      let is_anon = match e with Efun _ -> true | Lfun _ -> false | _ -> assert false in
      (* This is the function type as declared on the lambda itself.
       * If type hints are absent then use Tany instead. *)
      let declared_ft = Decl.fun_decl_in_env env.Env.decl_env f in
      (* When creating a closure, the 'this' type will mean the late bound type
       * of the current enclosing class
       *)
      let ety_env =
        { (Phase.env_with_self env) with from_class = Some CIstatic } in
      let env, declared_ft = Phase.localize_ft ~use_pos:p ~ety_env env declared_ft in
      List.iter idl (check_escaping_var env);
      (* Ensure lambda arity is not Fellipsis in strict mode *)
      begin match declared_ft.ft_arity with
      | Fellipsis _ when Env.is_strict env ->
        Errors.ellipsis_strict_mode ~require:`Param_name p
      | _ -> ()
      end;
      (* Is the return type declared? *)
      let is_explicit_ret = Option.is_some f.f_ret in
      let reactivity =
          Decl_fun_utils.fun_reactivity_opt env.Env.decl_env f.f_user_attributes
          |> Option.value ~default:(TR.strip_conditional_reactivity (Env.env_reactivity env)) in
      let check_body_under_known_params env ?ret_ty ft =
        let old_reactivity = Env.env_reactivity env in
        let env = Env.set_env_reactive env reactivity in
        let old_inside_ppl_class = env.Typing_env.inside_ppl_class in
        let env = { env with Typing_env.inside_ppl_class = false } in
        let ft = { ft with ft_reactive = reactivity } in
        let (is_coroutine, _counter, _, anon) = anon_make env p f ft idl is_anon in
        let ft = { ft with ft_is_coroutine = is_coroutine } in
        let env, tefun, ty = anon ?ret_ty env ft.ft_params ft.ft_arity in
        let env = Env.set_env_reactive env old_reactivity in
        let env = { env with
          Typing_env.inside_ppl_class = old_inside_ppl_class; } in
        let inferred_ty =
          if is_explicit_ret
          then (Reason.Rwitness p, Tfun { ft with ft_ret = declared_ft.ft_ret })
          else (Reason.Rwitness p, Tfun { ft with ft_ret = ty }) in
        env, tefun, inferred_ty in
      let env, eexpected = expand_expected env expected in
      begin match eexpected with
      | Some (_pos, _ur, (_, Tfun expected_ft)) ->
        (* First check that arities match up *)
        check_lambda_arity p expected_ft.ft_pos declared_ft.ft_arity expected_ft.ft_arity;
        (* Use declared types for parameters in preference to those determined
         * by the context: they might be more general. *)
        let rec replace_non_declared_types params declared_ft_params expected_ft_params =
          match params, declared_ft_params, expected_ft_params with
          | param::params, declared_ft_param::declared_ft_params,
            expected_ft_param::expected_ft_params ->
            let rest = replace_non_declared_types params declared_ft_params expected_ft_params in
            let resolved_ft_param = if Option.is_some param.param_hint
              then declared_ft_param
              else { declared_ft_param with fp_type = expected_ft_param.fp_type } in
            resolved_ft_param :: rest
          | _, _, _ ->
            (* This means the expected_ft params list can have more parameters
             * than declared parameters in the lambda. For variadics, this is OK,
             * for non-variadics, this will be caught elsewhere in arity checks.
             *)
            expected_ft_params
        in
        let replace_non_declared_arity variadic declared_arity expected_arity =
          match variadic with
          | FVvariadicArg {param_hint = Some(_); _} -> declared_arity
          | FVvariadicArg _ ->
              begin
                match declared_arity, expected_arity with
                | Fvariadic (min_arity, declared), Fvariadic (_, expected) ->
                  Fvariadic (min_arity, { declared with fp_type = expected.fp_type})
                | _, _ -> declared_arity
              end
          | _ -> declared_arity
        in
        let expected_ft = { expected_ft with ft_arity =
          replace_non_declared_arity
            f.f_variadic declared_ft.ft_arity expected_ft.ft_arity } in
        let expected_ft = { expected_ft with ft_params =
          replace_non_declared_types f.f_params declared_ft.ft_params expected_ft.ft_params } in
        (* Don't bother passing in `void` if there is no explicit return *)
        let ret_ty =
          match expected_ft.ft_ret with
          | _, Tprim Tvoid when not is_explicit_ret -> None
          | _ -> Some expected_ft.ft_ret in
        Typing_log.increment_feature_count env FL.Lambda.contextual_params;
        check_body_under_known_params env ?ret_ty expected_ft
      | _ ->
        let explicit_variadic_param_or_non_variadic =
          begin match f.f_variadic with
          | FVvariadicArg {param_hint; _} -> Option.is_some param_hint
          | FVellipsis _ -> false
          | _ -> true
          end
        in
        (* If all parameters are annotated with explicit types, then type-check
         * the body under those assumptions and pick up the result type *)
        let all_explicit_params =
          List.for_all f.f_params (fun param -> Option.is_some param.param_hint) in
        if all_explicit_params && explicit_variadic_param_or_non_variadic
        then begin
          Typing_log.increment_feature_count env
            (if List.is_empty f.f_params then FL.Lambda.no_params else FL.Lambda.explicit_params);
          check_body_under_known_params env declared_ft
        end
        else begin
          match expected with
          | Some (_, _, (_, Tany)) ->
            (* If the expected type is Tany env then we're passing a lambda to an untyped
             * function and we just assume every parameter has type Tany env *)
            Typing_log.increment_feature_count env FL.Lambda.untyped_context;
            check_body_under_known_params env declared_ft
          | Some _ ->
            (* If the expected type is something concrete but not a function
             * then we should reject in strict mode. Check body anyway *)
            if Env.is_strict env
            then Errors.untyped_lambda_strict_mode p;
            Typing_log.increment_feature_count env FL.Lambda.non_function_typed_context;
            check_body_under_known_params env declared_ft
          | _ ->
            (* If we're in partial mode then type-check definition anyway,
             * so treating parameters without type hints as "untyped"
            *)
            if TypecheckerOptions.new_inference (Env.get_tcopt env) && not (Env.is_strict env)
            then begin
              Typing_log.increment_feature_count env FL.Lambda.non_strict_unknown_params;
              check_body_under_known_params env declared_ft
            end
            else
            (* If new_inference and new_inference_lambda are enabled,
             * check lambda using constraints *)
            if TypecheckerOptions.new_inference (Env.get_tcopt env)
            && TypecheckerOptions.new_inference_lambda (Env.get_tcopt env)
            then begin
              Typing_log.increment_feature_count env FL.Lambda.fresh_tyvar_params;
              let freshen_untyped_param env ft_param =
                match snd ft_param.fp_type with
                | Tany ->
                  let env, ty = Env.fresh_invariant_type_var env ft_param.fp_pos in
                  env, { ft_param with fp_type = ty }
                | _ ->
                  env, ft_param in
              let env, ft_params = List.map_env env declared_ft.ft_params freshen_untyped_param in
              let declared_ft = { declared_ft with ft_params } in
              check_body_under_known_params env declared_ft
            end
            (* Legacy lambda inference *)
            else begin
              Typing_log.increment_feature_count env FL.Lambda.unknown_params;
              (* check for recursive function calls *)
              let reactivity = fun_reactivity env.Env.decl_env f.f_user_attributes f.f_params in
              let old_reactivity = Env.env_reactivity env in
              let env = Env.set_env_reactive env reactivity in
              let is_coroutine, counter, pos, anon = anon_make env p f declared_ft idl is_anon in
              let env, tefun, _, anon_id = Errors.try_with_error
                (fun () ->
                  let (_, tefun, ty) = anon env declared_ft.ft_params declared_ft.ft_arity in
                  let anon_fun = reactivity, is_coroutine, counter, pos, anon in
                  let env, anon_id = Env.add_anonymous env anon_fun in
                  env, tefun, ty, anon_id)
                (fun () ->
                  (* If the anonymous function declaration has errors itself, silence
                     them in any subsequent usages. *)
                  let anon_ign ?el:_ ?ret_ty:_ env fun_params =
                    Errors.ignore_ (fun () -> (anon env fun_params)) in
                  let (_, tefun, ty) = anon_ign env declared_ft.ft_params declared_ft.ft_arity in
                  let anon_fun = reactivity, is_coroutine, counter, pos, anon in
                  let env, anon_id = Env.add_anonymous env anon_fun in
                  env, tefun, ty, anon_id) in
              let env = Env.set_env_reactive env old_reactivity in
              let anon_ty = (Reason.Rwitness p, Tanon (declared_ft.ft_arity, anon_id)) in
              let ((ep,_efun_ty),efun) = tefun in
              let tefun = ((ep, anon_ty), efun) in
              env, tefun, anon_ty
            end
        end
      end
  | Xml (sid, attrl, el) ->
      let cid = CI sid in
      let env, _te, classes = class_id_for_new ~exact:Nonexact p env cid [] in
      let class_info = match classes with
        | [] -> None
         (* OK to ignore rest of list; class_info only used for errors, and
          * cid = CI sid cannot produce a union of classes anyhow *)
        | (_, class_info, _)::_ -> Some class_info
      in
      let env, _te, obj = expr env (fst sid, New ((fst sid, cid), [], [], [], (fst sid))) in
      let env, typed_attrs, attr_types = xhp_attribute_exprs env class_info attrl in
      let env, tel = List.map_env env el ~f:(fun env e -> let env, te, _ = expr env e in env, te) in
      let txml = T.Xml (sid, typed_attrs, List.rev tel) in
      (match class_info with
       | None -> make_result env p txml (Reason.Runknown_class p, Tobject)
       | Some class_info ->
        let env = List.fold_left attr_types ~f:begin fun env attr ->
          let namepstr, valpty = attr in
          let valp, valty = valpty in
          let env, declty =
            obj_get ~obj_pos:(fst sid) ~is_method:false ~nullsafe:None env obj cid
              namepstr (fun x -> x) in
          let ureason = Reason.URxhp ((Cls.name class_info), snd namepstr) in
          Type.coerce_type valp ureason env valty declty
        end ~init:env in
        make_result env p txml obj
      )
  | Unsafe_expr e ->
    (* Do not run inference on the expression, since unsafe is sometimes used to
       work around inference performance problems. *)
    let env = gather_defined_in_expr env e in
    let tcopt = Env.get_tcopt env in
    let te = NastTanyMapper.map_expr (ntm_env tcopt) e in
    make_result env p (T.Unsafe_expr te) (Reason.Rnone, Tany)
  | Callconv (kind, e) ->
      let env, te, ty = expr env e in
      make_result env p (T.Callconv (kind, te)) ty
  | Shape fdm ->
      let env, fdm_with_expected =
        match expand_expected env expected with
        | env, Some (pos, ur, (_, Tshape (_, expected_fdm))) ->
          let fdme =
            List.map
              ~f:(fun (k, v) ->
                match ShapeMap.get k expected_fdm with
                | None -> (k, (v, None))
                | Some sft -> (k, (v, Some (pos, ur, sft.sft_ty)))) fdm in
          env, fdme
        | _ ->
          env, List.map ~f:(fun (k, v) -> (k, (v, None))) fdm in

      (* allow_inter adds a type-variable *)
      let env, tfdm =
        List.map_env
          ~f:(fun env (key, (e, expected)) ->
            let env, te, ty = expr ?expected env e in env, (key, (te,ty)))
          env fdm_with_expected in
      let env, fdm =
        let convert_expr_and_type_to_shape_field_type env (key, (_, ty)) =
          let env, sft_ty = TUtils.unresolved env ty in
          (* An expression evaluation always corresponds to a shape_field_type
             with sft_optional = false. *)
          env, (key, { sft_optional = false; sft_ty }) in
        List.map_env ~f:convert_expr_and_type_to_shape_field_type env tfdm in
      let fdm = List.fold_left
        ~f:(fun acc (k, v) -> ShapeMap.add k v acc)
        ~init:ShapeMap.empty
        fdm in
      let env = check_shape_keys_validity env p (ShapeMap.keys fdm) in
      (* Fields are fully known, because this shape is constructed
       * using shape keyword and we know exactly what fields are set. *)
      make_result env p (T.Shape (List.map ~f:(fun (k,(te,_)) -> (k, te)) tfdm))
        (Reason.Rwitness p, Tshape (FieldsFullyKnown, fdm))

  | PU_atom _ -> failwith "TODO(T36532263): Pocket Universes"
  | PU_identifier _ -> failwith "TODO(T36532263): Pocket Universes"

  with Typing_lenv_cont.Continuation_not_found _ ->
    expr_any env p (Reason.Rwitness p)

and class_const ?(incl_tc=false) env p ((cpos, cid), mid) =
  let env, ce, cty = static_class_id ~check_constraints:false cpos env [] cid in
  let env, const_ty, cc_abstract_info =
    class_get ~is_method:false ~is_const:true ~incl_tc env cty mid cid in
  match cc_abstract_info with
    | Some (cc_pos, cc_name) ->
      let () = match cid with
        | CIstatic | CIexpr _ -> ();
        | _ -> Errors.abstract_const_usage p cc_pos cc_name; ()
      in make_result env p (T.Class_const (ce, mid)) const_ty
    | None ->
      make_result env p (T.Class_const (ce, mid)) const_ty

and anon_sub_type pos ur env ty_sub ty_super =
  Errors.try_add_err pos (Reason.string_of_ureason ur)
    (fun () -> SubType.sub_type env ty_sub ty_super)
    (fun () -> env)

and anon_coerce_type pos ur env ty_have ty_expect =
  Typing_ops.coerce_type ~sub_fn:anon_sub_type pos ur env ty_have ty_expect

(*****************************************************************************)
(* XHP attribute/body helpers. *)
(*****************************************************************************)
(**
 * Process a spread operator by computing the intersection of XHP attributes
 * between the spread expression and the XHP constructor onto which we're
 * spreading.
 *)
and xhp_spread_attribute env c_onto valexpr =
  let (p, _) = valexpr in
  let env, te, valty = expr env valexpr in
  (* Build the typed attribute node *)
  let typed_attr = T.Xhp_spread te in
  let env, attr_ptys = match c_onto with
    | None -> env, []
    | Some class_info -> Typing_xhp.get_spread_attributes env p class_info valty
  in env, typed_attr, attr_ptys

(**
 * Simple XHP attributes (attr={expr} form) are simply interpreted as a member
 * variable prefixed with a colon, the types of which will be validated later
 *)
and xhp_simple_attribute env id valexpr =
  let (p, _) = valexpr in
  let env, te, valty = expr env valexpr in
  (* This converts the attribute name to a member name. *)
  let name = ":"^(snd id) in
  let attr_pty = ((fst id, name), (p, valty)) in
  let typed_attr = T.Xhp_simple (id, te) in
  env, typed_attr, [attr_pty]


(**
 * Typecheck the attribute expressions - this just checks that the expressions are
 * valid, not that they match the declared type for the attribute and,
 * in case of spreads, makes sure they are XHP.
 *)
and xhp_attribute_exprs env cid attrl =
  let handle_attr (env, typed_attrl, attr_ptyl) attr =
    let env, typed_attr, attr_ptys = match attr with
      | Xhp_simple (id, valexpr) -> xhp_simple_attribute env id valexpr
      | Xhp_spread valexpr -> xhp_spread_attribute env cid valexpr
    in
    env, typed_attr::typed_attrl, attr_ptys @ attr_ptyl
  in
  let env, typed_attrl, attr_ptyl = List.fold_left ~f:handle_attr ~init:(env, [], []) attrl in
  env, List.rev typed_attrl, List.rev attr_ptyl

(*****************************************************************************)
(* Anonymous functions. *)
(*****************************************************************************)
and anon_bind_param params (env, t_params) ty : Env.env * Tast.fun_param list =
  match !params with
  | [] ->
      (* This code cannot be executed normally, because the arity is wrong
       * and it will error later. Bind as many parameters as we can and carry
       * on. *)
      env, t_params
  | param :: paraml ->
      params := paraml;
      match param.param_hint with
      | Some h ->

        let h = Decl_hint.hint env.Env.decl_env h in
        (* When creating a closure, the 'this' type will mean the
         * late bound type of the current enclosing class
         *)
        let ety_env =
          { (Phase.env_with_self env) with from_class = Some CIstatic } in
        let env, h = Phase.localize ~ety_env env h in
        let pos = Reason.to_pos (fst ty) in
        (* Don't use Type.coerce_type as it resets env.Env.pos unnecessarily *)
        let env = anon_coerce_type pos Reason.URparam env ty h in
        (* Closures are allowed to have explicit type-hints. When
         * that is the case we should check that the argument passed
         * is compatible with the type-hint.
         * The body of the function should be type-checked with the
         * hint and not the type of the argument passed.
         * Otherwise it leads to strange results where
         * foo(?string $x = null) is called with a string and fails to
         * type-check. If $x is a string instead of ?string, null is not
         * subtype of string ...
        *)
        let env, t_param = bind_param env (h, param) in
        env, t_params @ [t_param]
      | None ->
        let env, t_param = bind_param env (ty, param) in
        env, t_params @ [t_param]

and anon_bind_variadic env vparam variadic_ty =
  let env, ty, pos =
    match vparam.param_hint with
    | None ->
      (* if the hint is missing, use the type we expect *)
      env, variadic_ty, Reason.to_pos (fst variadic_ty)
    | Some hint ->
      let h = Decl_hint.hint env.Env.decl_env hint in
      let ety_env =
        { (Phase.env_with_self env) with from_class = Some CIstatic; } in
      let env, h = Phase.localize ~ety_env env h in
      let pos = Reason.to_pos (fst variadic_ty) in
      let env = anon_coerce_type pos Reason.URparam env variadic_ty h in
      env, h, vparam.param_pos
  in
  let r = Reason.Rvar_param pos in
  let arr_values = r, (snd ty) in
  let ty = r, Tarraykind (AKvarray arr_values) in
  let env, t_variadic = bind_param env (ty, vparam) in
  env, t_variadic


and anon_bind_opt_param env param : Env.env =
  match param.param_expr with
  | None ->
      let ty = Reason.Rwitness param.param_pos, Typing_utils.tany env in
      let env, _ = bind_param env (ty, param) in
      env
  | Some default ->
      let env, _te, ty = expr env default in
      Typing_sequencing.sequence_check_expr default;
      let env, _ = bind_param env (ty, param) in
      env

and anon_check_param env param =
  match param.param_hint with
  | None -> env
  | Some hty ->
      let env, hty = Phase.localize_hint_with_self env hty in
      let paramty = Env.get_local env (Local_id.make_unscoped param.param_name) in
      let hint_pos = Reason.to_pos (fst hty) in
      let env = Type.coerce_type hint_pos Reason.URhint env paramty hty in
      env

and stash_conts_for_anon env is_anon captured f =
  let captured = if Env.is_local_defined env this then (Pos.none, this) :: captured else captured in
  let initial_locals = if is_anon
    then Env.get_locals env captured
    else Env.next_cont_exn env in
  let env, (tfun, result) = Typing_lenv.stash_and_do env C.all (
    fun env ->
      let env = Env.reinitialize_locals env in
      let env = Env.set_locals env initial_locals in
      let env, tfun, result = f env in
      env, (tfun, result)) in
  env, tfun, result

(* Make a type-checking function for an anonymous function. *)
and anon_make tenv p f ft idl is_anon =
  let anon_lenv = tenv.Env.lenv in
  let is_typing_self = ref false in
  let nb = Nast.assert_named_body f.f_body in
  let is_coroutine = f.f_fun_kind = Ast.FCoroutine in
  is_coroutine,
  ref ([], []),
  p,
  (* Here ret_ty should include Awaitable wrapper *)
  fun ?el ?ret_ty env supplied_params supplied_arity ->
    if !is_typing_self
    then begin
      Errors.anonymous_recursive p;
      expr_error env p (Reason.Rwitness p)
    end
    else begin
      is_typing_self := true;
      Env.anon anon_lenv env begin fun env ->
      stash_conts_for_anon env is_anon idl begin fun env ->
        let env = Env.clear_params env in
        let make_variadic_arg env varg tyl =
          let remaining_types =
            (* It's possible the variadic arg will capture the variadic
             * parameter of the supplied arity (if arity is Fvariadic)
             * and additional supplied params.
             *
             * For example in cases such as:
             *  lambda1 = (int $a, string...$c) ==> {};
             *  lambda1(1, "hello", ...$y); (where $y is a variadic string)
             *  lambda1(1, "hello", "world");
             * then ...$c will contain "hello" and everything in $y in the first
             * example, and "hello" and "world" in the second example.
             *
             * To account for a mismatch in arity, we take the remaining supplied
             * parameters and return a list of all their types. We'll use this
             * to create a union type when creating the typed variadic arg.
             *)
            let remaining_params = List.drop supplied_params (List.length f.f_params) in
            List.map ~f:(fun param -> param.fp_type) remaining_params
          in
          let r = Reason.Rvar_param (varg.param_pos) in
          let union = Tunresolved (tyl @ remaining_types) in
          let env, t_param = anon_bind_variadic env varg (r, union) in
          env, T.FVvariadicArg t_param
        in
        let env, t_variadic =
          begin match f.f_variadic, supplied_arity with
          | FVvariadicArg arg, Fvariadic (_, variadic) ->
            make_variadic_arg env arg [variadic.fp_type]
          | FVvariadicArg arg, Fstandard _ ->
            make_variadic_arg env arg []
          | FVellipsis pos, _ -> env, T.FVellipsis pos
          | _, _ -> env, T.FVnonVariadic
          end in
        let params = ref f.f_params in
        let env, t_params = List.fold_left ~f:(anon_bind_param params) ~init:(env, [])
          (List.map supplied_params (fun x -> x.fp_type)) in
        let env = List.fold_left ~f:anon_bind_opt_param ~init:env !params in
        let env = List.fold_left ~f:anon_check_param ~init:env f.f_params in
        let env = match el with
          | None ->
            iter2_shortest Unify.unify_param_modes ft.ft_params supplied_params;
            env
          | Some x ->
            let var_param = match f.f_variadic with
              | FVellipsis pos ->
                let param = TUtils.default_fun_param ~pos
                  (Reason.Rvar_param pos, Tany) in
                Some param
              | _ -> None in
            let rec iter l1 l2 =
              match l1, l2, var_param with
              | _, [], _ -> ()
              | [], _, None -> ()
              | [], x2::rl2, Some def1 ->
                param_modes ~is_variadic:true def1 x2;
                iter [] rl2
              | x1::rl1, x2::rl2, _ -> param_modes x1 x2; iter rl1 rl2
            in
            iter ft.ft_params x;
            wfold_left2 inout_write_back env ft.ft_params x in
        let env = Env.set_fn_kind env f.f_fun_kind in
        let env, hret =
          match f.f_ret with
          | None ->
            (* Do we have a contextual return type? *)
            begin match ret_ty with
            | None ->
              let env, ret_ty = Env.fresh_unresolved_type env p in
              env, Typing_return.wrap_awaitable env p ret_ty
            | Some ret_ty ->
              (* We might need to force it to be Awaitable if it is a type variable *)
              Typing_return.force_awaitable env p ret_ty
            end
          | Some x ->
            let ret = Decl_hint.hint env.Env.decl_env x in
            (* If a 'this' type appears it needs to be compatible with the
             * late static type
             *)
            let ety_env =
              { (Phase.env_with_self env) with
                from_class = Some CIstatic } in
            Phase.localize ~ety_env env ret in
        let env = Env.set_return env
          (Typing_return.make_info f.f_fun_kind [] env
            ~is_explicit:(Option.is_some ret_ty)
            hret) in
        let local_tpenv = env.Env.lenv.Env.tpenv in
        let env, tb = block env nb.fb_ast in
        let implicit_return = LEnv.has_next env in
        let env =
          if not implicit_return || Nast.named_body_is_unsafe nb
          then env
          else fun_implicit_return env p hret f.f_fun_kind
        in
        (* We don't want the *uses* of the function to affect its return type *)
        let env, hret = Env.unbind env hret in
        is_typing_self := false;
        let annotation =
          if Nast.named_body_is_unsafe nb
          then Tast.Annotations.FuncBodyAnnotation.HasUnsafeBlocks
          else Tast.Annotations.FuncBodyAnnotation.NoUnsafeBlocks in
        let tfun_ = {
          T.f_annotation = Env.save local_tpenv env;
          T.f_span = f.f_span;
          T.f_mode = f.f_mode;
          T.f_ret = f.f_ret;
          T.f_name = f.f_name;
          T.f_tparams = List.map f.f_tparams (type_param env);
          T.f_where_constraints = f.f_where_constraints;
          T.f_fun_kind = f.f_fun_kind;
          T.f_file_attributes = [];
          T.f_user_attributes = List.map f.f_user_attributes (user_attribute env);
          T.f_body = { T.fb_ast = tb; fb_annotation = annotation };
          T.f_params = t_params;
          T.f_variadic = t_variadic; (* TODO TAST: Variadic efuns *)
          T.f_external = f.f_external;
          T.f_namespace = f.f_namespace;
          T.f_doc_comment = f.f_doc_comment;
          T.f_static = f.f_static;
        } in
        let ty = (Reason.Rwitness p, Tfun ft) in
        let te = if is_anon
          then T.make_typed_expr p ty (T.Efun (tfun_, idl))
          else T.make_typed_expr p ty (T.Lfun (tfun_, idl)) in
        let env = Env.set_tyvar_variance env ty in
        env, te, hret
      end (* stash_conts_for_anon *)
      end (* Env.anon *)
    end

(*****************************************************************************)
(* End of anonymous functions. *)
(*****************************************************************************)

and special_func env p func =
  let env, tfunc, ty = (match func with
  | Gena e ->
      let env, te, ety = expr env e in
      let env, ty = Async.gena env p ety in
      env, T.Gena te, ty
  | Genva el ->
      let env, tel, etyl = exprs env el in
      let env, ty = Async.genva env p etyl in
      env, T.Genva tel, ty
  | Gen_array_rec e ->
      let env, te, ety = expr env e in
      let env, ty = Async.gen_array_rec env p ety in
      env, T.Gen_array_rec te, ty
  ) in
  let result_ty = MakeType.awaitable (Reason.Rwitness p) ty in
  make_result env p (T.Special_func tfunc) result_ty

and requires_consistent_construct = function
  | CIstatic -> true
  | CIexpr _ -> true
  | CIparent -> false
  | CIself -> false
  | CI _ -> false

(* Caller will be looking for a particular form of expected type
 * e.g. a function type (when checking lambdas) or tuple type (when checking
 * tuples). First expand the expected type and elide single union; also
 * strip nullables, so ?t becomes t, as context will always accept a t if a ?t
 * is expected.
 *)
and expand_expected env expected =
  match expected with
  | None ->
    env, None
  | Some (p, ur, ty) ->
    let env, ty = Env.expand_type env ty in
    match ty with
    | _, Tunresolved [ty] -> env, Some (p, ur, ty)
    | _, Toption ty -> env, Some (p, ur, ty)
    | _ -> env, Some (p, ur, ty)

(* Do a subtype check of inferred type against expected type *)
and check_expected_ty message env inferred_ty expected =
  match expected with
  | None ->
    env
  | Some (p, ur, expected_ty) ->
    Typing_log.(log_with_level env "typing" 1 (fun () ->
      log_types p env
      [Log_head (Printf.sprintf "Typing.check_expected_ty %s" message,
       [Log_type ("inferred_ty", inferred_ty);
        Log_type ("expected_ty", expected_ty)])]));
    Type.coerce_type p ur env inferred_ty expected_ty

and new_object ~expected ~check_parent ~check_not_abstract ~is_using_clause p env cid tal el uel =
  (* Obtain class info from the cid expression. We get multiple
   * results with a CIexpr that has a union type *)
  let env, tcid, classes = instantiable_cid ~exact:Exact p env cid tal in
  let allow_abstract_bound_generic = match tcid with
  | (_, (_, Tabstract (AKgeneric tt, _))), T.CI (_, tn) -> tt = tn
  | _ -> false in
  let finish env tcid tel tuel ty ctor_fty =
    let env, new_ty =
      let (_, cid_ty), _ = tcid in
      match cid_ty with
      | _, Tabstract (AKgeneric _, _) ->
        env, cid_ty
      | _ ->
        if check_parent then env, ty
        else ExprDepTy.make env cid ty in
    env, tcid, tel, tuel, new_ty, ctor_fty in
  let rec gather env tel tuel res classes =
    match classes with
    | [] ->
      begin
        match res with
        | [] ->
          let env, tel, _ = exprs env el in
          let env, tuel, _ = exprs env uel in
          let r = Reason.Runknown_class p in
          finish env tcid tel tuel (r, Tobject) (r, TUtils.terr env)
        | [ty,ctor_fty] -> finish env tcid tel tuel ty ctor_fty
        | l ->
          let tyl, ctyl = List.unzip l in
          let r = Reason.Rwitness p in
          finish env tcid tel tuel (r, Tunresolved tyl) (r, Tunresolved ctyl)
      end

    | (cname, class_info, c_ty)::classes ->
      if check_not_abstract && (Cls.abstract class_info)
        && not (requires_consistent_construct cid)
        && not allow_abstract_bound_generic then
        uninstantiable_error env p cid (Cls.pos class_info) (Cls.name class_info) p c_ty;
      let env, obj_ty_, params =
        match cid, tal, snd c_ty with
        (* Explicit type arguments *)
        | CI _, (_::_), Tclass(_, _, tyl) -> env, (snd c_ty), tyl
        | _ ->
          let env, params = List.map_env env (Cls.tparams class_info)
            (fun env tparam ->
              let env, tvar = Env.fresh_unresolved_type env p in
              Typing_log.log_new_tvar_for_new_object env p tvar cname tparam;
              env, tvar) in
          begin match snd c_ty with
          | Tclass(_, Exact, _) ->
            env, (Tclass (cname, Exact, params)), params
          | _ ->
            env, (Tclass (cname, Nonexact, params)), params
          end in
      if not check_parent && not is_using_clause && (Cls.is_disposable class_info)
      then Errors.invalid_new_disposable p;
      let r_witness = Reason.Rwitness p in
      let obj_ty = (r_witness, obj_ty_) in
      let c_ty =
        match cid with
        | CIstatic -> (r_witness, TUtils.this_of obj_ty)
        | CIexpr _ -> (r_witness, snd c_ty)
        | _ -> obj_ty in
      let env, new_ty =
        let (_, cid_ty), _ = tcid in
        match cid_ty with
        | _, Tabstract (AKgeneric _, _) ->
          env, cid_ty
        | _ ->
          if check_parent
          then env, c_ty
          else ExprDepTy.make env cid c_ty in
      (* Set variance according to type of `new` expression now. Lambda arguments
       * to the constructor might depend on it, and `call_construct` only uses
       * `ctor_fty` to set the variance which has void return type *)
      let env = Env.set_tyvar_variance env new_ty in
      let env, _tcid, tel, tuel, ctor_fty =
        let env = check_expected_ty "New" env new_ty expected in
        call_construct p env class_info params el uel cid in
      if (snd (Cls.construct class_info)) = Inconsistent then
        (match cid with
          | CIstatic -> Errors.new_inconsistent_construct p cname `static
          | CIexpr _ -> Errors.new_inconsistent_construct p cname `classname
          | _ -> ());
      match cid with
        | CIparent ->
          let ctor_fty =
            match (fst (Cls.construct class_info)) with
            | Some {ce_type = lazy ty; _ } ->
              let ety_env = {
                type_expansions = [];
                substs = SMap.empty;
                this_ty = obj_ty;
                from_class = None;
                validate_dty = None;
              } in
              let _, ctor_fty = Phase.localize ~ety_env env ty in
              check_abstract_parent_meth SN.Members.__construct p ctor_fty
            | None -> ctor_fty
          in
          gather env tel tuel ((obj_ty,ctor_fty)::res) classes
        | CIstatic | CI _ | CIself -> gather env tel tuel ((c_ty,ctor_fty)::res) classes
        | CIexpr _ ->
          (* When constructing from a (classname) variable, the variable
           * dictates what the constructed object is going to be. This allows
           * for generic and dependent types to be correctly carried
           * through the 'new $foo()' iff the constructed obj_ty is a
           * supertype of the variable-dictated c_ty *)
          let env = SubType.sub_type env c_ty obj_ty in
          gather env tel tuel ((c_ty,ctor_fty)::res) classes
  in
  gather env [] [] [] classes

(* FIXME: we need to separate our instantiability into two parts. Currently,
 * all this function is doing is checking if a given type is inhabited --
 * that is, whether there are runtime values of type T. However,
 * instantiability should be the stricter notion that T has a runtime
 * constructor; that is, `new T()` should be valid. In particular, interfaces
 * are inhabited, but not instantiable.
 * To make this work with classname, we likely need to add something like
 * concrete_classname<T>, where T cannot be an interface.
 * *)
and instantiable_cid ?(exact = Nonexact) p env cid tal =
  let env, te, classes = class_id_for_new ~exact p env cid tal in
  begin
    List.iter classes begin fun ((pos, name), class_info, c_ty) ->
      if (Cls.kind class_info) = Ast.Ctrait || (Cls.kind class_info) = Ast.Cenum
      then
         match cid with
          | CIexpr _ | CI _ ->
            uninstantiable_error env p cid (Cls.pos class_info) name pos c_ty
          | CIstatic | CIparent | CIself -> ()
      else if (Cls.kind class_info) = Ast.Cabstract && (Cls.final class_info)
      then
        uninstantiable_error env p cid (Cls.pos class_info) name pos c_ty
      else () end;
    env, te, classes
  end

and uninstantiable_error env reason_pos cid c_tc_pos c_name c_usage_pos c_ty =
  let reason_msgl = match cid with
    | CIexpr _ ->
      let ty_str = "This would be "^Typing_print.error env c_ty in
      [(reason_pos, ty_str)]
    | _ -> [] in
  Errors.uninstantiable_class c_usage_pos c_tc_pos c_name reason_msgl

and exception_ty pos env ty =
  let exn_ty = MakeType.throwable (Reason.Rthrow pos) in
  Type.sub_type pos (Reason.URthrow) env ty exn_ty

and shape_field_pos = function
  | Ast.SFlit_int (p, _) | Ast.SFlit_str (p, _) -> p
  | Ast.SFclass_const ((cls_pos, _), (mem_pos, _)) -> Pos.btw cls_pos mem_pos

and check_shape_keys_validity env pos keys =
    (* If the key is a class constant, get its class name and type. *)
    let get_field_info env key =
      let key_pos = shape_field_pos key in
      (* Empty strings or literals that start with numbers are not
         permitted as shape field names. *)
      (match key with
        | Ast.SFlit_int _ ->
          env, key_pos, None
        | Ast.SFlit_str (_, key_name) ->
           if (String.length key_name = 0) then
             (Errors.invalid_shape_field_name_empty key_pos);
           env, key_pos, None
        | Ast.SFclass_const (p, cls as x, y) ->
          let env, _te, ty = class_const env pos ((p, CI x), y) in
          let env = Typing_enum.check_valid_array_key_type
            Errors.invalid_shape_field_type ~allow_any:false
            env key_pos ty in
          env, key_pos, Some (cls, ty))
    in

    let check_field witness_pos witness_info env key =
      let env, key_pos, key_info = get_field_info env key in
      match witness_info, key_info with
        | Some _, None ->
          Errors.invalid_shape_field_literal key_pos witness_pos; env
        | None, Some _ ->
          Errors.invalid_shape_field_const key_pos witness_pos; env
        | None, None -> env
        | Some (cls1, ty1), Some (cls2, ty2) ->
          if cls1 <> cls2 then
            Errors.shape_field_class_mismatch
              key_pos witness_pos (strip_ns cls2) (strip_ns cls1);
          if not (SubType.is_sub_type env ty1 ty2 && SubType.is_sub_type env ty2 ty1)
          then
            Errors.shape_field_type_mismatch
              key_pos witness_pos
              (Typing_print.error env ty2) (Typing_print.error env ty1);
          env
    in

    (* Sort the keys by their positions since the error messages will make
     * more sense if we take the one that appears first as canonical and if
     * they are processed in source order. *)
    let cmp_keys x y = Pos.compare (shape_field_pos x) (shape_field_pos y) in
    let keys = List.sort cmp_keys keys in

    match keys with
      | [] -> env
      | witness :: rest_keys ->
        let env, pos, info = get_field_info env witness in
        List.fold_left ~f:(check_field pos info) ~init:env rest_keys

and set_valid_rvalue p env x ty =
  let env = set_local env (p, x) ty in
  (* We are assigning a new value to the local variable, so we need to
   * generate a new expression id
   *)
  let env = Env.set_local_expr_id env x (Ident.tmp()) in
  env, ty

(* Deal with assignment of a value of type ty2 to lvalue e1 *)
and assign p env e1 ty2 : _ * T.expr * T.ty =
  assign_ p Reason.URassign env e1 ty2

and assign_ p ur env e1 ty2 =
  match e1 with
  | (_, Lvar ((_, x) as id)) ->
    let env, ty1 = set_valid_rvalue p env x ty2 in
    make_result env (fst e1) (T.Lvar id) ty1
  | (_, Lplaceholder id) ->
    let placeholder_ty = MakeType.void (Reason.Rplaceholder p) in
    make_result env (fst e1) (T.Lplaceholder id) placeholder_ty
  | (_, List el) ->
    let env, folded_ty2 = TUtils.fold_unresolved env ty2 in
    let resl =
      TUtils.try_over_concrete_supertypes env folded_ty2
        begin fun env ty2 ->
          let env, ty2 = SubType.expand_type_and_solve
            ~description_of_expected:"assignable value" env p ty2 in
          match ty2 with
          (* Vector<t> or ImmVector<t> or ConstVector<t> or vec<T> *)
          | (_, Tclass ((_, x), _, [elt_type]))
          when x = SN.Collections.cVector
            || x = SN.Collections.cImmVector
            || x = SN.Collections.cVec
            || x = SN.Collections.cConstVector ->
            let env, tel = List.map_env env el begin fun env e ->
              let env, te, _ = assign (fst e) env e elt_type in
              env, te
            end in
            make_result env (fst e1) (T.List tel) ty2
          (* array<t> or varray<t> *)
          | (_, Tarraykind (AKvec elt_type))
          | (_, Tarraykind (AKvarray elt_type)) ->
            let env, tel = List.map_env env el begin fun env e ->
              let env, te, _ = assign (fst e) env e elt_type in
              env, te
            end in
            make_result env (fst e1) (T.List tel) ty2
          (* array or empty array or Tany *)
          | (r, (Tarraykind (AKany | AKempty) | Tany)) ->
            let env, tel = List.map_env env el begin fun env e ->
              let env, te, _ = assign (fst e) env e (r, Typing_utils.tany env) in
              env, te
            end in
            make_result env (fst e1) (T.List tel) ty2
          | (r, (Tdynamic)) ->
            let env, tel = List.map_env env el begin fun env e ->
              let env, te, _ = assign (fst e) env e (MakeType.dynamic r) in
              env, te
            end in
            make_result env (fst e1) (T.List tel) ty2
          (* Pair<t1,t2> *)
          | ((r, Tclass ((_, coll), _, [ty1; ty2])) as folded_ety2)
            when coll = SN.Collections.cPair ->
              (match el with
            | [x1; x2] ->
                let env, te1, _ = assign p env x1 ty1 in
                let env, te2, _ = assign p env x2 ty2 in
                make_result env (fst e1) (T.List [te1; te2]) folded_ety2
            | _ ->
                Errors.pair_arity p;
                make_result env (fst e1) T.Any (r, Typing_utils.terr env))
        (* Other, including tuples. Create a tuple type for the left hand
         * side and attempt subtype against it. In particular this deals with
         * types such as (string,int) | (int,bool) *)
        | (r, _) ->
          let env = Env.open_tyvars env p in
          let env, tyl = List.map_env env el
             ~f:(fun env _ -> Env.fresh_unresolved_type env (Reason.to_pos r)) in
          let tuple_ty = (Reason.Rwitness (fst e1), Ttuple tyl) in
          let env = Type.sub_type p ur env folded_ty2 tuple_ty in
          let env = Env.set_tyvar_variance env tuple_ty in
          let env = SubType.close_tyvars_and_solve env in
          let env, reversed_tel =
            List.fold2_exn el tyl ~init:(env,[]) ~f:(fun (env,tel) lvalue ty2 ->
            let env, te, _ = assign p env lvalue ty2 in
            env, te::tel) in
          make_result env (fst e1) (T.List (List.rev reversed_tel)) ty2
        end in
    begin match resl with
      | [res] -> res
      | _ -> assign_simple p ur env e1 ty2
    end

  | pobj, Obj_get (obj, (pm, Id (_, member_name as m)), nullflavor) ->
      let lenv = env.Env.lenv in
      let no_fakes = LEnv.env_with_empty_fakes env in
      (* In this section, we check that the assignment is compatible with
       * the real type of a member. Remember that members can change
       * type (cf fake_members). But when we assign a value to $this->x,
       * we want to make sure that the type assign to $this->x is compatible
       * with the actual type hint. In this portion of the code, type-check
       * the assignment in an environment without fakes, and therefore
       * check that the assignment is compatible with the type of
       * the member.
       *)
      let nullsafe = match nullflavor with
        | OG_nullthrows -> None
        | OG_nullsafe -> Some pobj in
      let env, tobj, obj_ty = expr ~accept_using_var:true no_fakes obj in
      let env = might_throw env in
      let env, ty2' = Env.unbind env ty2 in
      let k (env, member_ty, vis) =
        let env = Type.coerce_type p ur env ty2' member_ty in
        env, member_ty, vis in
      let env, result =
        obj_get ~obj_pos:(fst obj) ~is_method:false ~nullsafe ~valkind:`lvalue
          env obj_ty (CIexpr e1) m k in
      let te1 =
        T.make_typed_expr pobj result
          (T.Obj_get
             (tobj, T.make_typed_expr pm result (T.Id m), nullflavor)) in
      let env = { env with Env.lenv = lenv } in
      begin match obj with
      | _, This ->
         let env, local = Env.FakeMembers.make p env obj member_name in
         let env, exp_real_type = Env.expand_type env result in
         Typing_suggest.save_member member_name env exp_real_type ty2;
         let env, ty = set_valid_rvalue p env local ty2 in
         env, te1, ty
      | _, Lvar _ ->
          let env, local = Env.FakeMembers.make p env obj member_name in
          let env, ty = set_valid_rvalue p env local ty2 in
          env, te1, ty
      | _ -> env, te1, ty2
      end
  | _, Obj_get _ ->
      let lenv = env.Env.lenv in
      let no_fakes = LEnv.env_with_empty_fakes env in
      let env, te1, real_type = lvalue no_fakes e1 in
      let env, exp_real_type = Env.expand_type env real_type in
      let env = { env with Env.lenv = lenv } in
      let env, ty2' = Env.unbind env ty2 in
      let env = Type.coerce_type p ur env ty2' exp_real_type in
      env, te1, ty2
  | _, Class_get (_, CGexpr _) -> failwith "AST should not have any CGexprs after naming"
  | _, Class_get ((_, x), CGstring (_, y)) ->
      let lenv = env.Env.lenv in
      let no_fakes = LEnv.env_with_empty_fakes env in
      let env, te1, real_type = lvalue no_fakes e1 in
      let env, exp_real_type = Env.expand_type env real_type in
      let env = { env with Env.lenv = lenv } in
      let env, ety2 = Env.expand_type env ty2 in
      let real_type_list =
        match exp_real_type with
        | _, Tunresolved tyl -> tyl
        | ty -> [ty]
      in
      let env = List.fold_left real_type_list ~f:begin fun env real_type ->
        Type.coerce_type p ur env ety2 real_type
      end ~init:env in
      let env, local = Env.FakeMembers.make_static p env x y in
      let env, ty3 = set_valid_rvalue p env local ty2 in
      (match x with
       | CIself
       | CIstatic ->
         Typing_suggest.save_member y env exp_real_type ty2;
       | _ -> ());
      env, te1, ty3
  | pos, Array_get (e1, None) ->
    let env, te1, ty1 = update_array_type pos env e1 None `lvalue in
    let env, (ty1', _ty2') =
      Typing_array_access.assign_array_append ~array_pos:(fst e1) ~expr_pos:p ur env ty1 ty2 in
    let env, te1 =
      if TUtils.is_hack_collection env ty1
      then env, te1
      else let env, te1, _ = assign_ p ur env e1 ty1' in env, te1 in
    make_result env pos (T.Array_get (te1, None)) ty2
  | pos, Array_get (e1, Some e) ->
    let env, te1, ty1 = update_array_type pos env e1 (Some e) `lvalue in
    let env, te, ty = expr env e in
    let env, (ty1', ty2') =
      Typing_array_access.assign_array_get ~array_pos:(fst e1) ~expr_pos:p ur env ty1 e ty ty2 in
    let env, te1 =
      if TUtils.is_hack_collection env ty1
      then env, te1
      else let env, te1, _ = assign_ p ur env e1 ty1' in env, te1 in
    env, ((pos, ty2'), T.Array_get (te1, Some te)), ty2
  | pref, Unop (Ast.Uref, e1') ->
    (* references can be "lvalues" in foreach bindings *)
    Errors.binding_ref_to_array pref;
    let env, texpr, ty = assign p env e1' ty2 in
    make_result env (fst e1) (T.Unop (Ast.Uref, texpr)) ty
  | _ ->
      assign_simple p ur env e1 ty2

and assign_simple pos ur env e1 ty2 =
  let env, te1, ty1 = lvalue env e1 in
  let env, ty2 = TUtils.unresolved env ty2 in
  let env = Type.coerce_type pos ur env ty2 ty1 in
  env, te1, ty2

and array_field env = function
  | Nast.AFvalue ve ->
    let env, tve, tv = expr env ve in
    let env, tv = Typing_env.unbind env tv in
    env, (T.AFvalue tve, None, tv)
  | Nast.AFkvalue (ke, ve) ->
      let env, tke, tk = expr env ke in
      let env, tve, tv = expr env ve in
      let env, tv = Typing_env.unbind env tv in
      env, (T.AFkvalue (tke, tve), Some tk, tv)

and array_value ~expected env x =
  let env, te, ty = expr ?expected ~array_ref_ctx:ElementAssignment env x in
  let env, ty = Typing_env.unbind env ty in
  env, (te, ty)

and array_field_value ~expected env = function
  | Nast.AFvalue x | Nast.AFkvalue (_, x) ->
      array_value ~expected env x

and arraykey_value p class_name ~expected env ((pos, _) as x) =
  let env, (te, ty) = array_value ~expected env x in
  let ty_arraykey = MakeType.arraykey (Reason.Ridx_dict pos) in
  let env = Type.sub_type p (Reason.index_class class_name) env ty ty_arraykey in
  env, (te, ty)

and array_field_key ~expected env = function
  (* This shouldn't happen *)
  | Nast.AFvalue (p, _) ->
      let ty = MakeType.int (Reason.Rwitness p) in
      env, (T.make_typed_expr p ty T.Any, ty)
  | Nast.AFkvalue (x, _) ->
      array_value ~expected env x

and check_parent_construct pos env el uel env_parent =
  let check_not_abstract = false in
  let env, env_parent = Phase.localize_with_self env env_parent in
  let env, _tcid, tel, tuel, parent, fty =
    new_object ~expected:None ~check_parent:true ~check_not_abstract
      ~is_using_clause:false
      pos env CIparent [] el uel in
  (* Not sure why we need to equate these types *)
  let env = Type.sub_type pos (Reason.URnone) env env_parent parent in
  let env = Type.sub_type pos (Reason.URnone) env parent env_parent in
  env, tel, tuel, MakeType.void (Reason.Rwitness pos), parent, fty

and call_parent_construct pos env el uel =
  let parent = Env.get_parent env in
  match parent with
    | _, Tapply _ ->
      check_parent_construct pos env el uel parent
    | _,
      (
          Tany
        | Tdynamic
        | Tmixed
        | Tnonnull
        | Tnothing
        | Tarray (_, _)
        | Tdarray (_, _)
        | Tvarray _
        | Tvarray_or_darray _
        | Tgeneric _
        | Toption _
        | Tlike _
        | Tprim _
        | Terr
        | Tfun _
        | Ttuple _
        | Tshape _
        | Taccess (_, _)
        | Tthis
      ) -> (* continue here *)
      let ty = (Reason.Rwitness pos, Typing_utils.tany env) in
      let default = env, [], [], ty, ty, ty in
      match Env.get_self env with
        | _, Tclass ((_, self), _, _) ->
          (match Env.get_class env self with
            | Some trait when Cls.kind trait = Ast.Ctrait ->
              (match trait_most_concrete_req_class trait env with
                | None -> Errors.parent_in_trait pos; default
                | Some (_, parent_ty) ->
                  check_parent_construct pos env el uel parent_ty
              )
            | Some self_tc ->
              if not (Cls.members_fully_known self_tc)
              then () (* Don't know the hierarchy, assume it's correct *)
              else Errors.undefined_parent pos;
              default
            | None -> assert false)
        | _, (Terr | Tany | Tnonnull | Tarraykind _ | Toption _
              | Tprim _ | Tfun _ | Ttuple _ | Tshape _ | Tvar _ | Tdynamic
              | Tabstract (_, _) | Tanon (_, _) | Tunresolved _ | Tobject
             ) ->
           Errors.parent_outside_class pos;
           let ty = (Reason.Rwitness pos, Typing_utils.terr env) in
           env, [], [], ty, ty, ty

(* parent::method() in a class definition invokes the specific parent
 * version of the method ... it better be callable *)
and check_abstract_parent_meth mname pos fty =
  if is_abstract_ft fty
  then Errors.parent_abstract_call mname pos (Reason.to_pos (fst fty));
  fty

and is_abstract_ft fty = match fty with
  | _r, Tfun { ft_abstract = true; _ } -> true
  | _r, (Terr | Tany | Tnonnull | Tarraykind _ | Toption _ | Tprim _
            | Tvar _ | Tfun _ | Tclass _ | Tabstract _ | Ttuple _
            | Tanon _ | Tunresolved _ | Tobject | Tshape _ | Tdynamic
        )
    -> false

(* Depending on the kind of expression we are dealing with
 * The typing of call is different.
 *)

 and dispatch_call ~expected ~is_using_clause p env call_type
    (fpos, fun_expr as e) tal el uel ~in_suspend =
  let make_call env te thl tel tuel ty =
    make_result env p (T.Call (call_type, te, thl, tel, tuel)) ty in
  (* TODO: Avoid Tany annotations in TAST by eliminating `make_call_special` *)
  let make_call_special env id tel ty =
    make_call env
      (T.make_typed_expr fpos (Reason.Rnone, TUtils.tany env) (T.Id id)) [] tel [] ty in
  (* For special functions and pseudofunctions with a definition in hhi. *)
  let make_call_special_from_def env id tel ty_ =
    let env, fty = fun_type_of_id env id tal in
    let ty = match fty with
      | _, Tfun ft -> ft.ft_ret
      | _ -> (Reason.Rwitness p, ty_) in
    make_call env (T.make_typed_expr fpos fty (T.Id id)) [] tel [] ty in
  let overload_function = overload_function make_call fpos in

  let check_coroutine_call env fty =
    let () = if is_return_disposable_fun_type env fty && not is_using_clause
             then Errors.invalid_new_disposable p else () in
    (* returns
       - Some true if type is definitely a coroutine
       - Some false if type is definitely not a coroutine
       - None if type is Tunresolved that contains
         both coroutine and non-coroutine constituents *)
    (* TODO: replace the case analysis here with a subtyping check;
     * see T37483866 and the linked diff for discussion.
     *)
    let rec is_coroutine env ty =
      let env, ety = SubType.expand_type_and_solve env (Reason.to_pos (fst ty)) ty
        ~description_of_expected:"a function value" in
      match snd ety with
      | Tfun { ft_is_coroutine = true; _ } ->
        env, Some true
      | Tanon (_, id) ->
        env, Some (Option.value_map (Env.get_anonymous env id) ~default:false ~f:(fun (_,b,_,_,_) -> b))
      | Tunresolved ts -> are_coroutines env ts
      | _ ->
        env, Some false
    and are_coroutines env ts =
      let env, ts_are_coroutines = List.map_env env ts ~f:is_coroutine in
      let ts_are_coroutines = match ts_are_coroutines with
        | None :: _ -> None
        | Some x :: xs ->
          (*if rest of the list has the same value as the first element
            return value of the first element or None otherwise*)
          if List.for_all xs ~f:(Option.value_map ~default:false ~f:((=)x))
          then Some x
          else None
        | _ -> Some false in
      env, ts_are_coroutines in
    let env, fty_is_coroutine = is_coroutine env fty in
    let () = match in_suspend, fty_is_coroutine with
      | true, Some true
      | false, Some false -> ()
      | true, _ ->
        (* non-coroutine call in suspend *)
        Errors.non_coroutine_call_in_suspend
          fpos
          (Reason.to_string ("This is " ^ Typing_print.error env fty) (fst fty));
      | false, _ ->
        (*coroutine call outside of suspend *)
        Errors.coroutine_call_outside_of_suspend p in
    env in

  let check_function_in_suspend name =
    if in_suspend
    then Errors.function_is_not_coroutine fpos name in

  let check_class_function_in_suspend class_name function_name =
    check_function_in_suspend (class_name ^ "::" ^ function_name) in

  match fun_expr with
  (* Special function `echo` *)
  | Id ((p, pseudo_func) as id) when pseudo_func = SN.SpecialFunctions.echo ->
      check_function_in_suspend SN.SpecialFunctions.echo;
      let env, tel, _ = exprs ~accept_using_var:true env el in
      make_call_special env id tel (MakeType.void (Reason.Rwitness p))
  (* Special function `empty` *)
  | Id ((_, pseudo_func) as id) when pseudo_func = SN.PseudoFunctions.empty ->
    check_function_in_suspend SN.PseudoFunctions.empty;
    let env, tel, _ = exprs ~accept_using_var:true env el in
    if uel <> [] then
      Errors.unpacking_disallowed_builtin_function p pseudo_func;
    make_call_special_from_def env id tel (Tprim Tbool)
  (* Special function `isset` *)
  | Id ((_, pseudo_func) as id) when pseudo_func = SN.PseudoFunctions.isset ->
    check_function_in_suspend SN.PseudoFunctions.isset;
    let env, tel, _ =
      exprs ~accept_using_var:true ~check_defined:false env el in
    if uel <> [] then
      Errors.unpacking_disallowed_builtin_function p pseudo_func;
    make_call_special_from_def env id tel (Tprim Tbool)
  (* Special function `unset` *)
  | Id ((_, pseudo_func) as id) when pseudo_func = SN.PseudoFunctions.unset ->
    check_function_in_suspend SN.PseudoFunctions.unset;
     let env, tel, _ = exprs env el in
     if uel <> [] then
       Errors.unpacking_disallowed_builtin_function p pseudo_func;
     let disallow_varray =
       TypecheckerOptions.disallow_unset_on_varray (Env.get_tcopt env) in
     let unset_error = if disallow_varray then
        Errors.unset_nonidx_in_strict_no_varray
      else
        Errors.unset_nonidx_in_strict in
     let env = if Env.is_strict env then
       (match el, uel with
         | [(_, Array_get ((_, Class_const _), Some _))], [] ->
           Errors.const_mutation p Pos.none "";
           env
         | [(_, Array_get (ea, Some _))], [] ->
           let env, _te, ty = expr env ea in
           let tany = (Reason.Rnone, Typing_utils.tany env) in
           if List.exists ~f:(fun super -> SubType.is_sub_type env ty super) [
             MakeType.dict Reason.Rnone tany tany;
             MakeType.keyset Reason.Rnone tany;
             if disallow_varray then
               (Reason.Rnone, Tarraykind (AKmap (tany, tany)))
             else (Reason.Rnone, Tarraykind AKany);
           ] then env
           else begin
               unset_error
               p
               (Reason.to_string ("This is " ^ Typing_print.error env ty) (fst ty));
             env
           end
         | _ -> unset_error p []; env)
       else env in
      (match el with
        | [(p, Obj_get (_, _, OG_nullsafe))] ->
          begin
            Errors.nullsafe_property_write_context p;
            make_call_special_from_def env id tel (TUtils.terr env)
          end;
        | _ ->
          make_call_special_from_def env id tel (Tprim Tvoid))
  (* Special function `array_filter` *)
  | Id ((_, array_filter) as id)
      when array_filter = SN.StdlibFunctions.array_filter && el <> [] && uel = [] ->
      check_function_in_suspend SN.StdlibFunctions.array_filter;
      (* dispatch the call to typecheck the arguments *)
      let env, fty = fun_type_of_id env id tal in
      let env, tel, tuel, res = call ~expected p env fty el uel in
      (* but ignore the result and overwrite it with custom return type *)
      let x = List.hd_exn el in
      let env, _tx, ty = expr env x in
      let explain_array_filter (r, t) =
        (Reason.Rarray_filter (p, r), t) in
      let get_value_type env tv =
        let env, tv =
          if List.length el > 1
          then env, tv
          else TUtils.non_null env p tv in
        env, explain_array_filter tv in
      let rec get_array_filter_return_type env ty =
        let env, ety = Env.expand_type env ty in
        (match ety with
        | (_, Tarraykind (AKany | AKempty)) as array_type ->
            env, array_type
        | (r, Tarraykind (AKvec tv | AKvarray tv)) ->
            let env, tv = get_value_type env tv in
            env, (r, Tarraykind (AKvec tv))
        | (r, Tunresolved x) ->
            let acc, x = List.map_env env x get_array_filter_return_type in
            acc, (r, Tunresolved x)
        | (r, Tany) ->
            env, (r, Typing_utils.tany env)
        | (r, Terr) ->
            env, (r, Typing_utils.terr env)
        | (r, _) ->
            let env, tk = Env.fresh_unresolved_type env p in
            let env, tv = Env.fresh_unresolved_type env p in
            Errors.try_
              (fun () ->
                let keyed_container_type = MakeType.keyed_container Reason.Rnone tk tv in
                let env = SubType.sub_type env ety keyed_container_type in
                let env, tv = get_value_type env tv in
                env, (r, Tarraykind (AKmap (
                  (explain_array_filter tk),
                  tv)
                )))
              (fun _ -> Errors.try_
                (fun () ->
                  let container_type = MakeType.container Reason.Rnone tv in
                  let env = SubType.sub_type env ety container_type in
                  let env, tv = get_value_type env tv in
                  env, (r, Tarraykind (AKmap (
                    (explain_array_filter (MakeType.arraykey r)),
                    tv))))
                (fun _ -> env, res)))
      in let env, rty = get_array_filter_return_type env ty in
      let fty =
        match fty with
        | r, Tfun ft -> r, Tfun {ft with ft_ret = rty}
        | _ -> fty in
      make_call env (T.make_typed_expr fpos fty (T.Id id)) tal tel tuel rty
  (* Special function `type_structure` *)
  | Id (p, type_structure)
      when type_structure = SN.StdlibFunctions.type_structure
           && (List.length el = 2) && uel = [] ->
    check_function_in_suspend SN.StdlibFunctions.type_structure;
    (match el with
     | [e1; e2] ->
       (match e2 with
        | p, Nast.String cst ->
          (* find the class constant implicitly defined by the typeconst *)
          let cid = (match e1 with
            | _, Class_const (cid, (_, x))
            | _, Class_get (cid, CGstring (_, x)) when x = SN.Members.mClass -> cid
            | _ -> (fst e1, Nast.CIexpr e1)) in
          class_const ~incl_tc:true env p (cid, (p, cst))
        | _ ->
          Errors.illegal_type_structure p "second argument is not a string";
          expr_error env p (Reason.Rwitness p))
     | _ -> assert false)
  (* Special function `array_map` *)
  | Id ((_, array_map) as x)
      when array_map = SN.StdlibFunctions.array_map && el <> [] && uel = [] ->
      check_function_in_suspend SN.StdlibFunctions.array_map;
      let env, fty = fun_type_of_id env x [] in
      let env, fty = Env.expand_type env fty in
      let env, fty = match fty, el with
        | ((r_fty, Tfun fty), _::args) when args <> [] ->
          let arity = List.length args in
          (*
            Builds a function with signature:

            function<T1, ..., Tn, Tr>(
              (function(T1, ..., Tn):Tr),
              Container<T1>,
              ...,
              Container<Tn>,
            ): R

            where R is constructed by build_output_container applied to Tr
          *)
          let build_function env build_output_container =
            let env, vars, tr =
              (* If T1, ... Tn, Tr are provided explicitly, instantiate the function parameters with
               * those directly. *)
              if List.is_empty tal
              then
                let env, tr = Env.fresh_unresolved_type env p in
                let env, vars = List.map_env env args
                  ~f:(fun env _ -> Env.fresh_unresolved_type env p) in
                env, vars, tr
              else if List.length tal <> List.length args + 1 then begin
                let env, tr = Env.fresh_unresolved_type env p in
                Errors.expected_tparam ~use_pos:fpos ~definition_pos:fty.ft_pos
                  (1 + (List.length args));
                let env, vars = List.map_env env args
                  ~f:(fun env _ -> Env.fresh_unresolved_type env p) in
                env, vars, tr end
              else
              let env, vars_and_tr = List.map_env env tal Phase.localize_hint_with_self in
              let vars, trl = List.split_n vars_and_tr (List.length vars_and_tr - 1) in
              (* Since we split the arguments and return type at the last index and the length is
                 non-zero this is safe. *)
              let tr = List.hd_exn trl in
              env, vars, tr
            in
            let f = TUtils.default_fun_param (
              r_fty,
              Tfun {
                ft_pos = fty.ft_pos;
                ft_deprecated = None;
                ft_abstract = false;
                ft_is_coroutine = false;
                ft_arity = Fstandard (arity, arity);
                ft_tparams = ([], FTKtparams);
                ft_where_constraints = [];
                ft_params = List.map vars TUtils.default_fun_param;
                ft_ret = tr;
                ft_reactive = fty.ft_reactive;
                ft_mutability = fty.ft_mutability;
                ft_returns_mutable = fty.ft_returns_mutable;
                ft_return_disposable = fty.ft_return_disposable;
                ft_decl_errors = None;
                ft_returns_void_to_rx = fty.ft_returns_void_to_rx;
              }
            ) in
            let containers = List.map vars (fun var ->
              let tc = Tclass ((fty.ft_pos, SN.Collections.cContainer), Nonexact, [var]) in
              TUtils.default_fun_param (r_fty, tc)
            ) in
            env, (r_fty, Tfun {fty with
                               ft_arity = Fstandard (arity+1, arity+1);
                               ft_params = f::containers;
                               ft_ret =  build_output_container tr;
                              })
          in

          (*
            Takes a Container type and returns a function that can "pack" a type
            into an array of appropriate shape, preserving the key type, i.e.:
            array                 -> f, where f R = array
            array<X>              -> f, where f R = array<R>
            array<X, Y>           -> f, where f R = array<X, R>
            Vector<X>             -> f  where f R = array<R>
            KeyedContainer<X, Y>  -> f, where f R = array<X, R>
            Container<X>          -> f, where f R = array<arraykey, R>
            X                     -> f, where f R = Y
          *)
          let rec build_output_container
            env (x:locl ty) : (Env.env * (locl ty -> locl ty)) =
            let env, x = Env.expand_type env x in
              match x with
              | (_, Tarraykind (AKany | AKempty)) as array_type ->
                env, (fun _ -> array_type)
              | (r, Tarraykind (AKvec _ | AKvarray _)) ->
                env, (fun tr -> (r, Tarraykind (AKvec(tr))) )
              | (r, Tany) ->
                env, (fun _ -> (r, Typing_utils.tany env))
              | (r, Terr) ->
                env, (fun _ -> (r, Typing_utils.terr env))
              | (r, Tunresolved x) ->
                let env, x = List.map_env env x build_output_container in
                env, (fun tr -> (r, Tunresolved (List.map x (fun f -> f tr))))
              | (r, _) ->
                let env, tk = Env.fresh_unresolved_type env p in
                let env, tv = Env.fresh_unresolved_type env p in
                let try_vector env =
                  let vector_type = MakeType.const_vector r_fty tv in
                  let env = SubType.sub_type env x vector_type in
                  env, (fun tr -> (r, Tarraykind (
                    AKvec(tr)
                  ))) in
                let try_keyed_container env =
                  let keyed_container_type = MakeType.keyed_container r_fty tk tv in
                  let env = SubType.sub_type env x keyed_container_type in
                  env, (fun tr -> (r, Tarraykind (AKmap (
                    tk,
                    tr
                  )))) in
                let try_container env =
                  let container_type = MakeType.container r_fty tv in
                  let env = SubType.sub_type env x container_type in
                  env, (fun tr -> (r, Tarraykind (AKmap (
                    (MakeType.arraykey r),
                    tr)))) in
                let env, tr =
                  Errors.try_
                  (fun () ->
                    try_vector  env)
                  (fun _ -> Errors.try_
                    (fun () ->
                      try_keyed_container env)
                    (fun _ -> Errors.try_
                      (fun () ->
                        try_container env)
                      (fun _ -> env, (fun _ -> (Reason.Rwitness p, Typing_utils.tany env))))) in
                env, tr in
          (*
            Single argument calls preserve the key type, multi argument
            calls always return an array<Tr>
          *)
          (match args with
            | [x] ->
              let env, _tx, x = expr env x in
              let env, output_container = build_output_container env x in
              build_function env output_container
            | _ ->
              build_function env (fun tr ->
                (r_fty, Tarraykind (AKvec(tr)))))
        | _ -> env, fty in
      let env, tel, tuel, ty = call ~expected p env fty el [] in
      make_call env (T.make_typed_expr fpos fty (T.Id x)) tal tel tuel ty
  (* Special function `idx` *)
  | Id ((_, idx) as id) when idx = SN.FB.idx ->
      check_function_in_suspend SN.FB.idx;
      (* Directly call get_fun so that we can muck with the type before
       * instantiation -- much easier to work in terms of Tgeneric Tk/Tv than
       * trying to figure out which Tvar is which. *)
      (match Env.get_fun env (snd id) with
      | Some fty ->
        let param1, param2, param3 =
          match fty.ft_params with
            | [param1; param2; param3] -> param1, param2, param3
            | _ -> assert false in
        let { fp_type = (r2, _); _ } = param2 in
        let { fp_type = (r3, _); _ } = param3 in
        let params, ret = match List.length el with
          | 2 ->
            let ty1 = match param1.fp_type with
              | (r11, Toption (r12, Tapply (coll, [tk; (r13, _) as tv]))) ->
                (r11, Toption (r12, Tapply (coll, [tk; (r13, Toption tv)])))
              | _ -> assert false in
            let param1 = { param1 with fp_type = ty1 } in
            let ty2 = MakeType.nullable r2 (r2, Tgeneric "Tk") in
            let param2 = { param2 with fp_type = ty2 } in
            let rret = fst fty.ft_ret in
            let ret = MakeType.nullable rret (rret, Tgeneric "Tv") in
            [param1; param2], ret
          | 3 ->
            let param2 = { param2 with fp_type = MakeType.nullable r2 (r2, Tgeneric "Tk") } in
            let param3 = { param3 with fp_type = (r3, Tgeneric "Tv") } in
            let ret = (fst fty.ft_ret, Tgeneric "Tv") in
            [param1; param2; param3], ret
          | _ -> fty.ft_params, fty.ft_ret in
        let fty = { fty with ft_params = params; ft_ret = ret } in
        let ety_env = Phase.env_with_self env in
        let env, fty = Phase.localize_ft ~use_pos:p ~ety_env env fty in
        let tfun = Reason.Rwitness fty.ft_pos, Tfun fty in
        let env, tel, _tuel, ty = call ~expected p env tfun el [] in
        (* Remove double nullables. This shouldn't be necessary, and currently
         * interferes with new_inference because it "solves" before we set variance
         *)
        let env, ty = match ty with
          | r, Toption ty when not (TypecheckerOptions.new_inference (Env.get_tcopt env)) ->
            let env, ty = TUtils.non_null env p ty in
            env, (r, Toption ty)
          | _ -> env, ty in
        make_call env (T.make_typed_expr fpos tfun (T.Id id)) [] tel [] ty
      | None -> unbound_name env id)

  (* Special function `Shapes::idx` *)
  | Class_const ((_, CI (_, shapes)) as class_id, ((_, idx) as method_id))
      when shapes = SN.Shapes.cShapes && idx = SN.Shapes.idx ->
      check_class_function_in_suspend SN.Shapes.cShapes SN.Shapes.idx;
      overload_function p env class_id method_id el uel
      begin fun env fty res el -> match el with
        | [shape; field] ->
          let env, _ts, shape_ty = expr env shape in
          Typing_shapes.idx env p (fst fty) shape_ty field None
        | [shape; field; default] ->
            let env, _ts, shape_ty = expr env shape in
            let env, _td, default_ty = expr env default in
            Typing_shapes.idx env p (fst fty) shape_ty field
              (Some ((fst default), default_ty))
        | _ -> env, res
      end
   (* Special function `Shapes::keyExists` *)
   | Class_const ((_, CI (_, shapes)) as class_id, ((_, key_exists) as method_id))
      when shapes = SN.Shapes.cShapes && key_exists = SN.Shapes.keyExists ->
      check_class_function_in_suspend SN.Shapes.cShapes SN.Shapes.keyExists;
      overload_function p env class_id method_id el uel
      begin fun env fty res el -> match el with
        | [shape; field] ->
          let env, _te, shape_ty = expr env shape in
          (* try accessing the field, to verify existence, but ignore
           * the returned type and keep the one coming from function
           * return type hint *)
          let env, _ = Typing_shapes.idx env p (fst fty) shape_ty field None in
          env, res
        | _  -> env, res
      end
   (* Special function `Shapes::removeKey` *)
   | Class_const ((_, CI (_, shapes)) as class_id, ((_, remove_key) as method_id))
      when shapes = SN.Shapes.cShapes && remove_key = SN.Shapes.removeKey ->
      check_class_function_in_suspend SN.Shapes.cShapes SN.Shapes.removeKey;
      overload_function p env class_id method_id el uel
      begin fun env _ res el -> match el with
        | [shape; field] -> begin match shape with
            | (_, Lvar (_, lvar))
            | (_, Callconv (Ast.Pinout, (_, Lvar (_, lvar))))
            | (_, Unop (Ast.Uref, (_, Lvar (_, lvar)))) ->
              let env, _te, shape_ty = expr ~is_func_arg:true env shape in
              let env, shape_ty =
                Typing_shapes.remove_key p env shape_ty field in
              let env, _ = set_valid_rvalue p env lvar shape_ty in
              env, res
            | _ ->
              Errors.invalid_shape_remove_key (fst shape);
              env, res
          end
        | _  -> env, res
      end
  (* Special function `Shapes::toArray` *)
  | Class_const ((_, CI (_, shapes)) as class_id, ((_, to_array) as method_id))
    when shapes = SN.Shapes.cShapes && to_array = SN.Shapes.toArray ->
    check_class_function_in_suspend SN.Shapes.cShapes SN.Shapes.toArray;
    overload_function p env class_id method_id el uel
    begin fun env _ res el -> match el with
      | [shape] ->
         let env, _te, shape_ty = expr env shape in
         Typing_shapes.to_array env p shape_ty res
      | _  -> env, res
    end

  (* Special function `Shapes::toDict` *)
  | Class_const ((_, CI (_, shapes)) as class_id, ((_, to_array) as method_id))
    when shapes = SN.Shapes.cShapes && to_array = SN.Shapes.toDict ->
    check_class_function_in_suspend SN.Shapes.cShapes SN.Shapes.toDict;
    overload_function p env class_id method_id el uel
    begin fun env _ res el -> match el with
      | [shape] ->
         let env, _te, shape_ty = expr env shape in
         Typing_shapes.to_dict env p shape_ty res
      | _  -> env, res
    end

  (* Special function `parent::__construct` *)
  | Class_const ((pos, CIparent), ((_, construct) as id))
    when construct = SN.Members.__construct ->
      check_class_function_in_suspend "parent" SN.Members.__construct;
      let env, tel, tuel, ty, pty, ctor_fty =
        call_parent_construct p env el uel in
      make_call env (T.make_typed_expr fpos ctor_fty
        (T.Class_const (((pos, pty), T.CIparent), id))) tal tel tuel ty

  (* Calling parent method *)
  | Class_const ((pos, CIparent), m) ->
      let env, tcid, ty1 = static_class_id ~check_constraints:false pos env [] CIparent in
      let this_ty = (Reason.Rwitness fpos, TUtils.this_of (Env.get_self env)) in
      if Env.is_static env
      then begin
        (* in static context, you can only call parent::foo() on static
         * methods *)
        let env, fty, _ =
          class_get ~is_method:true ~is_const:false ~explicit_tparams:tal env ty1 m CIparent in
        let fty = check_abstract_parent_meth (snd m) p fty in
        let env = check_coroutine_call env fty in
        let env, tel, tuel, ty =
          call ~expected
          ~method_call_info:(TR.make_call_info ~receiver_is_self:false
            ~is_static:true this_ty (snd m))
          p env fty el uel in
        make_call env (T.make_typed_expr fpos fty
          (T.Class_const (tcid, m))) tal tel tuel ty
      end
      else begin
        (* in instance context, you can call parent:foo() on static
         * methods as well as instance methods *)
        if not(class_contains_smethod env ty1 m)
        then
            (* parent::nonStaticFunc() is really weird. It's calling a method
             * defined on the parent class, but $this is still the child class.
             * We can deal with this by hijacking the continuation that
             * calculates the SN.Typehints.this type *)
            let k_lhs _ = this_ty in
            let ftys = ref [] in
            let env, method_, _ =
              obj_get_ ~is_method:true ~nullsafe:None ~obj_pos:pos
                ~pos_params:(Some el) ~valkind:`other env ty1 CIparent m
              begin fun (env, fty, _) ->
                let fty = check_abstract_parent_meth (snd m) p fty in
                let env = check_coroutine_call env fty in
                let env, _tel, _tuel, method_ = call ~expected
                  ~method_call_info:(TR.make_call_info ~receiver_is_self:false
                    ~is_static:false this_ty (snd m))
                  p env fty el uel in
                ftys := fty :: !ftys;
                env, method_, None
              end
              k_lhs
            in
            let fty =
              match !ftys with
              | [fty] -> fty
              | l -> (Reason.none, Tunresolved l) in
            make_call env (T.make_typed_expr fpos fty (T.Class_const (tcid, m)))
              tal [] [] method_
        else
            let env, fty, _ =
              class_get ~is_method:true ~is_const:false ~explicit_tparams:tal env ty1 m CIparent in
            let fty = check_abstract_parent_meth (snd m) p fty in
            let env = check_coroutine_call env fty in
            let env, tel, tuel, ty =
              call ~expected ~method_call_info:(TR.make_call_info ~receiver_is_self:false
                  ~is_static:true this_ty (snd m))
                p env fty el uel in
            make_call  env (T.make_typed_expr fpos fty
              (T.Class_const (tcid, m))) tal tel tuel ty
      end
  (* Call class method *)
  | Class_const ((pid, e1), m) ->
      let env, te1, ty1 = static_class_id ~check_constraints:true pid env [] e1 in
      let env, fty, _ =
        class_get ~is_method:true ~is_const:false ~explicit_tparams:tal
        ~pos_params:el env ty1 m e1 in
      let () = match e1 with
        | CIself when is_abstract_ft fty ->
          begin match Env.get_self env with
            | _, Tclass ((_, self), _, _) ->
              (* at runtime, self:: in a trait is a call to whatever
               * self:: is in the context of the non-trait "use"-ing
               * the trait's code *)
              begin match Env.get_class env self with
                | Some cls when Cls.kind cls = Ast.Ctrait -> ()
                | _ -> Errors.self_abstract_call (snd m) p (Reason.to_pos (fst fty))
              end
            | _ -> ()
          end
        | CI c when is_abstract_ft fty ->
          Errors.classname_abstract_call (snd c) (snd m) p (Reason.to_pos (fst fty))
        | CI (_, classname) ->
          begin match Typing_heap.Classes.get classname with
          | Some class_def ->
            let (_, method_name) = m in
            begin match Cls.get_smethod class_def method_name with
            | None -> ()
            | Some elt ->
              if elt.ce_synthesized then
                Errors.static_synthetic_method classname (snd m) p (Reason.to_pos (fst fty))
            end
          | None ->
            (* This technically should be an error, but if we throw here we'll break a ton of our
            tests since they reference classes that only exist in www, and any missing classes will
            get caught elsewhere in the pipeline. *)
            ()
          end
        | _ -> () in
      let env = check_coroutine_call env fty in
      let env, tel, tuel, ty =
        call ~expected
        ~method_call_info:(TR.make_call_info ~receiver_is_self:(e1 = CIself)
          ~is_static:true ty1 (snd m))
        p env fty el uel in
      make_call env (T.make_typed_expr fpos fty
        (T.Class_const(te1, m))) tal tel tuel ty
  (* <<__PPL>>: sample, factor, observe, condition *)
  | Id (pos, id) when env.Env.inside_ppl_class && SN.PPLFunctions.is_reserved id ->
      let m = (pos, String_utils.lstrip id "\\") in
      (* Mock these as type equivalent to \Infer -> sample... *)
      let infer_e = CI (p, "\\Infer") in
      let env, _, ty1 = static_class_id ~check_constraints:true p env [] infer_e in
      let nullsafe = None in
      let tel = ref [] and tuel = ref [] and tftyl = ref [] in
      let fn = (fun (env, fty, _) ->
        let env, tel_, tuel_, method_ =
          call
            ~expected
            ~method_call_info:(TR.make_call_info ~receiver_is_self:false
              ~is_static:false ty1 (snd m))
            p env fty el uel in
        tel := tel_; tuel := tuel_;
        tftyl := fty :: !tftyl;
        env, method_, None) in
      let env, ty = obj_get ~obj_pos:p ~is_method:true ~nullsafe ~pos_params:el
                      ~explicit_tparams:tal env ty1 infer_e m fn in
      let tfty =
        match !tftyl with
        | [fty] -> fty
        | tftyl -> (Reason.none, Tunresolved tftyl)
      in
      make_call env (T.make_typed_expr fpos tfty (T.Fun_id m)) tal !tel !tuel ty

  (* Call instance method *)
  | Obj_get(e1, (pos_id, Id m), nullflavor) ->
      let is_method = call_type = Cnormal in
      let env, te1, ty1 = expr ~accept_using_var:true env e1 in
      let nullsafe =
        (match nullflavor with
          | OG_nullthrows -> None
          | OG_nullsafe -> Some p
        ) in
      let tel = ref [] and tuel = ref [] and tftyl = ref [] in
      let k = (fun (env, fty, _) ->
        let env = check_coroutine_call env fty in
        let env, tel_, tuel_, method_ =
          call ~expected
            ~method_call_info:(TR.make_call_info ~receiver_is_self:false
              ~is_static:false ty1 (snd m))
            p env fty el uel in
        tel := tel_; tuel := tuel_;
        tftyl := fty :: !tftyl;
        env, method_, None) in
      let env, ty = obj_get ~obj_pos:(fst e1) ~is_method ~nullsafe ~pos_params:el
                      ~explicit_tparams:tal env ty1 (CIexpr e1) m k in
      let tfty =
        match !tftyl with
        | [fty] -> fty
        | tftyl -> (Reason.none, Tunresolved tftyl)
      in
      make_call env (T.make_typed_expr fpos tfty (T.Obj_get(te1,
        T.make_typed_expr pos_id tfty (T.Id m), nullflavor))) tal !tel !tuel ty

  (* Function invocation *)
  | Fun_id x ->
      let env, fty = fun_type_of_id env x tal in
      let env = check_coroutine_call env fty in
      let env, tel, tuel, ty =
        call ~expected p env fty el uel in
      make_call  env (T.make_typed_expr fpos fty (T.Fun_id x)) tal tel tuel ty
  | Id (_, id as x) ->
      let env, fty = fun_type_of_id env x tal in
      let env = check_coroutine_call env fty in
      let env, tel, tuel, ty =
        call ~expected p env fty el uel in
      let is_mutable = id = SN.Rx.mutable_ in
      let is_move = id = SN.Rx.move in
      let is_freeze = id = SN.Rx.freeze in
      (* error when rx builtins are used in non-reactive context *)
      if not (Env.env_local_reactive env) then begin
        if is_mutable then Errors.mutable_in_nonreactive_context p
        else if is_move then Errors.move_in_nonreactive_context p
        else if is_freeze then Errors.freeze_in_nonreactive_context p
      end;
      (* ban unpacking when calling builtings *)
      if (is_mutable || is_move || is_freeze) && uel <> [] then begin
        Errors.unpacking_disallowed_builtin_function p id
      end;
      (* adjust env for Rx\freeze or Rx\move calls *)
      let env =
        if is_freeze
        then Typing_mutability.freeze_local p env tel
        else if is_move
        then Typing_mutability.move_local p env tel
        else env
      in
      make_call env (T.make_typed_expr fpos fty (T.Id x)) tal tel tuel ty
  | _ ->
      let env, te, fty = expr env e in
      let env, fty = SubType.expand_type_and_solve
        ~description_of_expected:"a function value" env fpos fty in
      let env = check_coroutine_call env fty in
      let env, tel, tuel, ty = call ~expected p env fty el uel in
      make_call env te tal tel tuel ty

and fun_type_of_id env x tal =
  match Env.get_fun env (snd x) with
  | None -> let env, _, ty = unbound_name env x in env, ty
  | Some fty ->
      let ety_env = Phase.env_with_self env in
      let env, fty =
        Phase.localize_ft ~use_pos:(fst x) ~explicit_tparams:tal ~ety_env env fty in
      env, (Reason.Rwitness fty.ft_pos, Tfun fty)

(**
 * Checks if a class (given by cty) contains a given static method.
 *
 * We could refactor this + class_get
 *)
and class_contains_smethod env cty (_pos, mid) =
  let lookup_member ty =
    match ty with
    | _, Tclass ((_, c), _, _) ->
      (match Env.get_class env c with
      | None -> false
      | Some class_ ->
        Option.is_some @@ Env.get_static_member true env class_ mid
      )
    | _ -> false in
  let _env, tyl = TUtils.get_concrete_supertypes env cty in
  List.exists tyl ~f:lookup_member

and class_get ~is_method ~is_const ?(explicit_tparams=[]) ?(incl_tc=false)
              ?(pos_params : expr list option) env cty (p, mid) cid =
  let env, this_ty =
    if is_method then
      this_for_method env cid cty
    else
      env, cty in
  let ety_env = {
    type_expansions = [];
    this_ty = this_ty;
    substs = SMap.empty;
    from_class = Some cid;
    validate_dty = None;
  } in
  class_get_ ~is_method ~is_const ~ety_env ~explicit_tparams ~incl_tc
             ~pos_params env cid cty (p, mid)

and class_get_ ~is_method ~is_const ~ety_env ?(explicit_tparams=[])
               ?(incl_tc=false) ~pos_params env cid cty
(p, mid) =
  let env, cty = Env.expand_type env cty in
  match cty with
  | r, Tany -> env, (r, Typing_utils.tany env), None
  | r, Terr -> env, err_witness env (Reason.to_pos r), None
  | _, Tdynamic -> env, cty, None
  | _, Tunresolved tyl ->
      let env, tyl =
        List.map_env env tyl begin fun env ty ->
        let env, ty, _ =
          class_get_ ~is_method ~is_const ~ety_env ~explicit_tparams ~incl_tc
                     ~pos_params env cid ty (p, mid)
            in env, ty
        end in
      let env, ty = Union.union_list env (fst cty) tyl in
      let env, method_ = TUtils.in_var env ty in
      env, method_, None
  | _, Tabstract (_, Some ty) ->
      class_get_ ~is_method ~is_const ~ety_env ~explicit_tparams ~incl_tc
        ~pos_params env cid ty (p, mid)
  | _, Tabstract (_, None) ->
      let resl = TUtils.try_over_concrete_supertypes env cty (fun env ty ->
        class_get_ ~is_method ~is_const ~ety_env ~explicit_tparams ~incl_tc
          ~pos_params env cid ty (p, mid)) in
      begin match resl with
      | [] ->
        Errors.non_class_member
          mid p (Typing_print.error env cty)
          (Reason.to_pos (fst cty));
        (env, err_witness env p, None)
      | ((_, (_, ty), _) as res)::rest ->
        if List.exists rest (fun (_, (_, ty'), _) -> ty' <> ty)
        then
          begin
            Errors.ambiguous_member
              mid p (Typing_print.error env cty)
              (Reason.to_pos (fst cty));
            (env, err_witness env p, None)
          end
        else res
      end
  | _, Tclass ((_, c), _, paraml) ->
      let class_ = Env.get_class env c in
      (match class_ with
      | None -> env, (Reason.Rwitness p, Typing_utils.tany env), None
      | Some class_ ->
        (* We need to instantiate generic parameters in the method signature *)
        let ety_env =
          { ety_env with
            substs = Subst.make (Cls.tparams class_) paraml } in
        if is_const then begin
          let const =
            if incl_tc then Env.get_const env class_ mid else
            match Env.get_typeconst env class_ mid with
            | Some _ ->
              Errors.illegal_typeconst_direct_access p;
              None
            | None ->
              Env.get_const env class_ mid
          in
          match const with
          | None ->
            smember_not_found p ~is_const ~is_method class_ mid;
            env, (Reason.Rnone, Typing_utils.terr env), None
          | Some { cc_type; cc_abstract; cc_pos; _ } ->
            let env, cc_type = Phase.localize ~ety_env env cc_type in
            env, cc_type,
            (if cc_abstract
             then Some (cc_pos, (Cls.name class_) ^ "::" ^ mid)
             else None)
        end else begin
          let smethod = Env.get_static_member is_method env class_ mid in
          match smethod with
          | None ->
            (match Env.get_static_member is_method env class_
              SN.Members.__callStatic with
              | None ->
                smember_not_found p ~is_const ~is_method class_ mid;
                env, (Reason.Rnone, Typing_utils.terr env), None
              | Some {ce_visibility = vis; ce_lsb = lsb; ce_type = lazy (r, Tfun ft); _} ->
                let p_vis = Reason.to_pos r in
                TVis.check_class_access p env (p_vis, vis, lsb) cid class_;
                let env, ft =
                  Phase.localize_ft ~use_pos:p ~ety_env ~explicit_tparams:explicit_tparams env ft in
                let arity_pos = match ft.ft_params with
                  | [_; { fp_pos; fp_kind = FPnormal; _ }] -> fp_pos
                  (* we should really assert here but this is not yet validated *)
                  | _ -> p_vis in
                let ft = { ft with
                  ft_arity = Fellipsis (0, arity_pos);
                  ft_tparams = ([], FTKtparams); ft_params = [];
                } in
                let res_ty = (r, Tfun ft) in
                env, res_ty, None
              | _ -> assert false)
          | Some { ce_visibility = vis; ce_lsb = lsb; ce_type = lazy method_; _ } ->
            let p_vis = Reason.to_pos (fst method_) in
            TVis.check_class_access p env (p_vis, vis, lsb) cid class_;
            let env, method_ =
              begin match method_ with
                (* We special case Tfun here to allow passing in explicit tparams to localize_ft. *)
                | r, Tfun ft ->
                  let env, ft =
                    Phase.localize_ft ~use_pos:p ~ety_env ~explicit_tparams:explicit_tparams env ft
                  in env, (r, Tfun ft)
                | _ ->
                  Phase.localize ~ety_env env method_
              end in
            env, method_, None
        end
      )
  | _, (Tvar _ | Tnonnull | Tarraykind _ | Toption _
        | Tprim _ | Tfun _ | Ttuple _ | Tanon (_, _) | Tobject
       | Tshape _) ->
      (* should never happen; static_class_id takes care of these *)
      env, (Reason.Rnone, Typing_utils.tany env), None

and smember_not_found pos ~is_const ~is_method class_ member_name =
  let kind =
    if is_const then `class_constant
    else if is_method then `static_method
    else `class_variable in
  let error hint =
    let cid = ((Cls.pos class_), (Cls.name class_)) in
    Errors.smember_not_found kind pos cid member_name hint
  in
  match Env.suggest_static_member is_method class_ member_name with
  | None ->
      (match Env.suggest_member is_method class_ member_name with
      | None when not (Cls.members_fully_known class_) ->
          (* no error in this case ... the member might be present
           * in one of the parents of class_ that the typing cannot see *)
          ()
      | None ->
          error `no_hint
      | Some (pos2, v) ->
          error (`closest (pos2, v))
      );
  | Some (pos2, v) ->
      error (`did_you_mean (pos2, v))

and member_not_found pos ~is_method class_ member_name r =
  let kind = if is_method then `method_ else `member in
  let cid = (Cls.pos class_), (Cls.name class_) in
  let reason = Reason.to_string
    ("This is why I think it is an object of type "^strip_ns (Cls.name class_)) r
  in
  let error hint =
    Errors.member_not_found kind pos cid member_name hint reason in
  match Env.suggest_member is_method class_ member_name with
    | None ->
      (match Env.suggest_static_member is_method class_ member_name with
        | None when not (Cls.members_fully_known class_) ->
          (* no error in this case ... the member might be present
           * in one of the parents of class_ that the typing cannot see *)
          ()
        | None ->
          error `no_hint
        | Some (def_pos, v) ->
          error (`closest (def_pos, v))
      )
    | Some (def_pos, v) ->
        error (`did_you_mean (def_pos, v))

(* Look up the type of the property or method id in the type ty1 of the
 *receiver and use the function k to postprocess the result.
 * Return any fresh type variables that were substituted for generic type
 * parameters in the type of the property or method.
 *
 * Essentially, if ty1 is a concrete type, e.g., class C, then k is applied
 * to the type of the property id in C; and if ty1 is an unresolved type,
 * e.g., a union of classes (C1 | ... | Cn), then k is applied to the type
 * of the property id in each Ci and the results are collected into an
 * unresolved type.
 *
 * The extra flexibility offered by the functional argument k is used in two
 * places:
 *
 *   (1) when type-checking method calls: if the receiver has an unresolved
 *   type, then we need to type-check the method call with each possible
 *   receiver type and collect the results into an unresolved type;
 *
 *   (2) when type-checking assignments to properties: if the receiver has
 *   an unresolved type, then we need to check that the right hand side
 *   value can be assigned to the property id for each of the possible types
 *   of the receiver.
 *)
and obj_get ~obj_pos ~is_method ~nullsafe ?(valkind = `other) ?(explicit_tparams=[])
            ?(pos_params: expr list option) env ty1 cid id k =
  let env, method_, _ =
    obj_get_with_visibility ~is_method ~nullsafe ~valkind ~obj_pos ~pos_params
      ~explicit_tparams env ty1 cid id k in
  env, method_

and obj_get_with_visibility ~is_method ~nullsafe ~valkind ~obj_pos ~pos_params
    ?(explicit_tparams=[]) env ty1 cid id k =
  obj_get_ ~is_method ~nullsafe ~valkind ~obj_pos ~pos_params ~explicit_tparams env ty1
    cid id k (fun ty -> ty)

(* We know that the receiver is a concrete class: not a generic with
 * bounds, or a Tunresolved. *)
and obj_get_concrete_ty ~is_method ~valkind ?(explicit_tparams=[])
    env concrete_ty class_id (id_pos, id_str) k_lhs =
  let default () = env, (Reason.Rwitness id_pos, Typing_utils.tany env), None in
  let mk_ety_env r class_info x e paraml =
    let this_ty = k_lhs (r, (Tclass(x, e, paraml))) in
    {
      type_expansions = [];
      this_ty = this_ty;
      substs = Subst.make (Cls.tparams class_info) paraml;
      from_class = Some class_id;
      validate_dty = None;
    }
  in
  match concrete_ty with
  | (r, Tclass(x, exact, paraml)) ->
    begin match Env.get_class env (snd x) with
    | None ->
      default ()

    | Some class_info when not is_method
        && not (Env.is_strict env)
        && (Cls.name class_info) = SN.Classes.cStdClass ->
      default ()

    | Some class_info ->
      let paraml =
        if List.length paraml = 0
        then List.map (Cls.tparams class_info)
            (fun _ -> Reason.Rwitness id_pos, Typing_utils.tany env)
        else paraml in
      let old_member_info = Env.get_member is_method env class_info id_str in
      let self = Env.get_self_id env in
      let member_info, shadowed = if Cls.has_ancestor class_info self
      then
        (* We look up the current context to see if there is a field/method with
        * private visibility. If there is one, that one takes precedence *)
        begin match Env.get_class env self with
        | None -> old_member_info, false
        | Some self_class ->
          match Env.get_member is_method env self_class id_str with
          | Some { ce_visibility = Vprivate _; _ } as member_info ->
            member_info, true
          | _ -> old_member_info, false
        end
      else old_member_info, false
      in

      begin match member_info with
      | None when not is_method ->
        if not (SN.Members.is_special_xhp_attribute id_str)
        then member_not_found id_pos ~is_method class_info id_str r;
        default ()

      | None ->
        begin match Env.get_member is_method env class_info SN.Members.__call with
        | None ->
          member_not_found id_pos ~is_method class_info id_str r;
          default ()

        | Some {ce_visibility = vis; ce_type = lazy (r, Tfun ft); _}  ->
          let mem_pos = Reason.to_pos r in
          TVis.check_obj_access id_pos env (mem_pos, vis);

          (* the return type of __call can depend on the class params or be this *)
          let ety_env = mk_ety_env r class_info x exact paraml in
          let env, ft = Phase.localize_ft ~use_pos:id_pos ~ety_env env ft in

          let arity_pos = match ft.ft_params with
          | [_; { fp_pos; fp_kind = FPnormal; _ }] -> fp_pos
          (* we should really assert here but this is not yet validated *)
          | _ -> mem_pos in

          (* we change the params of the underlying declaration to act as a
           * variadic function ... this transform cannot be done when processing
           * the declaration of call because direct calls to $inst->__call are also
           * valid.
          *)
          let ft = {ft with
            ft_arity = Fellipsis (0, arity_pos); ft_tparams = ([], FTKtparams); ft_params = []; } in

          let member_ty = (r, Tfun ft) in
          env, member_ty, Some (mem_pos, vis)

        | _ -> assert false

        end (* match Env.get_member is_method env class_info SN.Members.__call *)

      | Some ({ce_visibility = vis; ce_type = lazy member_; _ } as member_ce) ->
        let mem_pos = Reason.to_pos (fst member_) in
        if shadowed then begin match old_member_info with
          | Some ({ce_visibility = old_vis; ce_type = lazy old_member; _ }) ->
            let old_mem_pos = Reason.to_pos (fst old_member) in
            begin match class_id with
            | CIexpr (_, This) when snd x = self -> ()
            | _ -> Errors.ambiguous_object_access
              id_pos id_str mem_pos (TUtils.string_of_visibility old_vis) old_mem_pos self (snd x)
            end;
          | _ -> ()
        end;
        TVis.check_obj_access id_pos env (mem_pos, vis);
        let member_ty = Typing_enum.member_type env member_ce in
        let ety_env = mk_ety_env r class_info x exact paraml in
        let env, member_ty =
          begin match member_ty with
            | (r, Tfun ft) ->
              (* We special case function types here to be able to pass explicit type
               * parameters. *)
              let env, ft =
                Phase.localize_ft ~use_pos:id_pos ~explicit_tparams ~ety_env env ft in
              env, (r, Tfun ft)
            | _ ->
              Phase.localize ~ety_env env member_ty
          end in

        if member_ce.ce_const && valkind = `lvalue then
          if not (env.Env.inside_constructor &&
            (* expensive call behind short circuiting && *)
            SubType.is_sub_type env (Env.get_self env) concrete_ty) then
            Errors.assigning_to_const id_pos;

        env, member_ty, Some (mem_pos, vis)
      end (* match member_info *)

    end (* match Env.get_class env (snd x) *)
  | _, Tdynamic ->
    let ty = MakeType.dynamic (Reason.Rdynamic_prop id_pos) in
    env, ty, None
  | _, Tobject
  | _, Tany
  | _, Terr ->
    default ()
  | _ ->
    Errors.non_object_member
      id_str id_pos (Typing_print.error env concrete_ty)
      (Reason.to_pos (fst concrete_ty));
    default ()

and widen_class_for_obj_get ~is_method ~nullsafe member_name env ty =
  match ty with
  | _, Tprim Tnull ->
    if Option.is_some nullsafe then env, Some ty else env, None

  | (r2, Tclass ((_, class_name) as class_id, _, tyl)) ->
    let default () =
      let ty = (r2, Tclass (class_id, Nonexact, tyl)) in
      env, Some ty in
    begin match Env.get_class env class_name with
    | None -> default ()
    | Some class_info ->
      match Env.get_member is_method env class_info member_name with
      | Some { ce_origin; _ } ->
        (* If this member was inherited then we obtain the type from which
         * it is inherited as our wider type *)
        if ce_origin = class_name
        then default ()
        else
          begin match Cls.get_ancestor class_info ce_origin with
          | None -> default ()
          | Some basety ->
            let ety_env =
            {
              type_expansions = [];
              substs = Subst.make (Cls.tparams class_info) tyl;
              this_ty = ty;
              from_class = None;
              validate_dty = None;
            } in
            let env, basety = Phase.localize ~ety_env env basety in
            env, Some basety
          end
      | None ->
        env, None
    end
  | _ ->
    env, None

(* k_lhs takes the type of the object receiver *)
and obj_get_ ~is_method ~nullsafe ~valkind ~obj_pos
  ~(pos_params : expr list option) ?(explicit_tparams=[])
  env ty1 cid (id_pos, id_str as id) k k_lhs =
  let env, ety1 =
    if is_method
    then SubType.expand_type_and_solve env ~description_of_expected:"an object" obj_pos ty1
    else SubType.expand_type_and_narrow env ~description_of_expected:"an object"
      (widen_class_for_obj_get ~is_method ~nullsafe id_str) obj_pos ty1 in
  let nullable_obj_get ty = match nullsafe with
    | Some p1 ->
        let env, method_, x = obj_get_ ~obj_pos ~is_method ~nullsafe ~valkind
          ~pos_params ~explicit_tparams env ty cid id k k_lhs in
        let env, method_ = TUtils.non_null env id_pos method_ in
        env, MakeType.nullable (Reason.Rnullsafe_op p1) method_, x
    | None ->
        Errors.null_member id_str id_pos
          (Reason.to_string
             "This is what makes me believe it can be null"
             (fst ety1)
          );
        k (env, (fst ety1, Typing_utils.terr env), None) in
  match ety1 with
  | _, Tunresolved tyl ->
      let (env, vis), tyl = List.map_env (env, None) tyl
        begin fun (env, vis) ty ->
          let env, ty, vis' =
            obj_get_ ~obj_pos ~is_method ~nullsafe ~valkind ~pos_params
              ~explicit_tparams env ty cid id k k_lhs in
          (* There is one special case where we need to expose the
           * visibility outside of obj_get (checkout inst_meth special
           * function).
           * We keep a witness of the "most restrictive" visibility
           * we encountered (position + visibility), to be able to
           * special case inst_meth.
           *)
          let vis = TVis.min_vis_opt vis vis' in
          (env, vis), ty
        end in
      let env, ty = Union.union_list env (fst ety1) tyl in
      let env, method_ = TUtils.in_var env ty in
      env, method_, vis

  | p', (Tabstract(ak, Some ty)) ->
    let k_lhs' ty = match ak with
    | AKnewtype (_, _) -> k_lhs ty
    | _ -> k_lhs (p', Tabstract (ak, Some ty)) in
      obj_get_ ~obj_pos ~is_method ~nullsafe ~valkind ~pos_params
        ~explicit_tparams env ty cid id k k_lhs'

  | p', (Tabstract(ak,_)) ->
    let resl =
    TUtils.try_over_concrete_supertypes env ety1
      (fun env ty ->
      (* We probably don't want to rewrap new types for the 'this' closure *)
      (* TODO AKENN: we shouldn't refine constraints by changing
       * the type like this *)
         let k_lhs' ty = match ak with
         | AKnewtype (_, _) -> k_lhs ty
         | _ -> k_lhs (p', Tabstract (ak, Some ty)) in
         obj_get_concrete_ty ~is_method ~valkind ~explicit_tparams env ty cid id k_lhs'
      ) in
    begin match resl with
      | [] -> begin
          Errors.non_object_member
            id_str id_pos (Typing_print.error env ety1)
            (Reason.to_pos (fst ety1));
          k (env, err_witness env id_pos, None)
        end
      | ((_env, (_, ty), _vis) as res)::rest ->
        if List.exists rest (fun (_, (_,ty'), _) -> ty' <> ty)
        then
        begin
          Errors.ambiguous_member
            id_str id_pos (Typing_print.error env ety1)
            (Reason.to_pos (fst ety1));
          k (env, err_witness env id_pos, None)
        end
        else k res
    end

  | _, Toption ty -> nullable_obj_get ty
  | r, Tprim Nast.Tnull ->
    let ty =
      if TypecheckerOptions.new_inference (Env.get_tcopt env)
      then (r, Tunresolved [])
      else (r, Tany) in
    nullable_obj_get ty
  (* We are trying to access a member through a value of unknown type *)
  | r, Tvar _ ->
    Errors.unknown_object_member id_str id_pos (Reason.to_string "It is unknown" r);
    k (env, (r, Typing_utils.terr env), None)

  | _, _ ->
    k (obj_get_concrete_ty ~is_method ~valkind ~explicit_tparams env ety1 cid id k_lhs)

and class_id_for_new ~exact p env cid tal =
  let env, te, cid_ty = static_class_id ~exact ~check_constraints:false p env tal cid in
  (* Need to deal with union case *)
  let rec get_info res tyl =
    match tyl with
    | [] -> env, te, res
    | ty::tyl ->
      match snd ty with
      | Tunresolved tyl' ->
        get_info res (tyl' @ tyl)
      | _ ->
        (* Instantiation on an abstract class (e.g. from classname<T>) is
         * via the base type (to check constructor args), but the actual
         * type `ty` must be preserved. *)
        match TUtils.get_base_type env ty with
        | _, Tclass (sid, _, _) ->
          begin
            let class_ = Env.get_class env (snd sid) in
            match class_ with
            | None -> get_info res tyl
            | Some class_info ->
              match te, cid_ty with
              (* When computing the classes for a new T() where T is a generic,
               * the class must be consistent (final, final constructor, or
               * <<__ConsistentConstruct>>) for its constructor to be considered *)
              | (_, T.CI (_, c)), (_, Tabstract (AKgeneric cg, _)) when c = cg ->
                (* Only have this choosing behavior for new T(), not all generic types
                 * i.e. new classname<T>, TODO: T41190512 *)
                if Tast_utils.valid_newable_class class_info
                then get_info ((sid, class_info, ty)::res) tyl
                else get_info res tyl
              | _ ->
                get_info ((sid, class_info, ty)::res) tyl
          end
        | _, (Tany | Terr | Tnonnull | Tarraykind _ | Toption _
              | Tprim _ | Tvar _ | Tfun _ | Tabstract (_, _) | Ttuple _
              | Tanon (_, _) | Tunresolved _ | Tobject | Tshape _ | Tdynamic ) ->
          get_info res tyl in
  get_info [] [cid_ty]

(* To be a valid trait declaration, all of its 'require extends' must
 * match; since there's no multiple inheritance, it follows that all of
 * the 'require extends' must belong to the same inheritance hierarchy
 * and one of them should be the child of all the others *)
and trait_most_concrete_req_class trait env =
  Sequence.fold (Cls.all_ancestor_reqs trait) ~f:begin fun acc (_p, ty) ->
    let _r, (_p, name), _paraml = TUtils.unwrap_class_type ty in
    let keep = match acc with
      | Some (c, _ty) -> Cls.has_ancestor c name
      | None -> false
    in
    if keep then acc
    else
      let class_ = Env.get_class env name in
      (match class_ with
        | None -> acc
        | Some c when Cls.kind c = Ast.Cinterface -> acc
        | Some c when Cls.kind c = Ast.Ctrait ->
          (* this is an error case for which the nastCheck spits out
           * an error, but does *not* currently remove the offending
           * 'require extends' or 'require implements' *)
          acc
        | Some c -> Some (c, ty)
      )
  end ~init:None

(* If there are no explicit type arguments then generate fresh type variables
 * for all of them. Otherwise, check the arity, and use the explicit types. *)
and resolve_type_argument env hint =
  (* For explicit type arguments we support a wildcard syntax `_` for which
  * Hack will generate a fresh type variable *)
   match hint with
   | (p, Happly((_, id), [])) when id = SN.Typehints.wildcard  ->
     Env.fresh_unresolved_type env p
   | _ ->
     Phase.localize_hint_with_self env hint
and resolve_type_arguments env p _class_id tparaml hintl =
  let length_hintl = List.length hintl in
  let length_tparaml = List.length tparaml in
  if length_hintl <> length_tparaml
  then begin
    List.map_env env tparaml begin fun env tparam ->
      let env, tvar = Env.fresh_unresolved_type env p in
      Typing_log.log_tparam_instantiation env p tparam tvar;
      env, tvar end
  end
  else List.map_env env hintl resolve_type_argument

(* Do all of the above, and also check any constraints associated with the type parameters.
 *)
and resolve_type_arguments_and_check_constraints ~exact ~check_constraints
  env p class_id from_class tparaml hintl =
  let env, type_argl = resolve_type_arguments env p class_id tparaml hintl in
  let this_ty = (Reason.Rwitness (fst class_id), Tclass (class_id, exact, type_argl)) in
  let env =
    if check_constraints
    then let ety_env = {
      type_expansions = [];
      this_ty = this_ty;
      substs = Subst.make tparaml type_argl;
      from_class = Some from_class;
      validate_dty = None;
    } in
      Phase.check_tparams_constraints ~use_pos:p ~ety_env env tparaml
    else env in
  env, this_ty

(* When invoking a method the class_id is used to determine what class we
 * lookup the method in, but the type of 'this' will be the late bound type.
 * For example:
 *
 *  class C {
 *    public static function get(): this { return new static(); }
 *
 *    public static function alias(): this { return self::get(); }
 *  }
 *
 *  In C::alias, when we invoke self::get(), 'self' is resolved to the class
 *  in the lexical scope (C), so call C::get. However the method is executed in
 *  the current context, so static inside C::get will be resolved to the late
 *  bound type (get_called_class() within C::alias).
 *
 *  This means when determining the type of this, CIparent and CIself should be
 *  changed to CIstatic. For the other cases of C::get() or $c::get(), we only
 *  look at the left hand side of the '::' and use the type type associated
 *  with it.
 *
 *  Thus C::get() will return a type C, while $c::get() will return the same
 *  type as $c.
 *)
and this_for_method env cid default_ty = match cid with
  | CIparent | CIself | CIstatic ->
      let p = Reason.to_pos (fst default_ty) in
      let env, _te, ty = static_class_id ~check_constraints:false p env [] CIstatic in
      ExprDepTy.make env CIstatic ty
  | _ ->
      env, default_ty

and static_class_id ?(exact = Nonexact) ~check_constraints p env tal =
  let make_result env te ty = env, ((p, ty), te), ty in
  function
  | CIparent ->
    (match Env.get_self env with
      | _, Tclass ((_, self), _, _) ->
        (match Env.get_class env self with
          | Some trait when Cls.kind trait = Ast.Ctrait ->
            (match trait_most_concrete_req_class trait env with
              | None ->
                Errors.parent_in_trait p;
                make_result env T.CIparent (Reason.Rwitness p, Typing_utils.terr env)
              | Some (_, parent_ty) ->
                (* inside a trait, parent is SN.Typehints.this, but with the
                 * type of the most concrete class that the trait has
                 * "require extend"-ed *)
                let r = Reason.Rwitness p in
                let env, parent_ty = Phase.localize_with_self env parent_ty in
                make_result env T.CIparent (r, TUtils.this_of parent_ty)
            )
          | _ ->
            let parent = Env.get_parent env in
            let parent_defined = snd parent <> Typing_utils.tany env in
            if not parent_defined
            then Errors.parent_undefined p;
            let r = Reason.Rwitness p in
            let env, parent = Phase.localize_with_self env parent in
            (* parent is still technically the same object. *)
            make_result env T.CIparent (r, TUtils.this_of (r, snd parent))
          )
      | _, (Terr | Tany | Tnonnull | Tarraykind _ | Toption _ | Tprim _
            | Tfun _ | Ttuple _ | Tshape _ | Tvar _ | Tdynamic
            | Tanon (_, _) | Tunresolved _ | Tabstract (_, _) | Tobject
           ) ->
        let parent = Env.get_parent env in
        let parent_defined = snd parent <> Typing_utils.tany env in
        if not parent_defined
        then Errors.parent_undefined p;
        let r = Reason.Rwitness p in
        let env, parent = Phase.localize_with_self env parent in
        (* parent is still technically the same object. *)
        make_result env T.CIparent (r, TUtils.this_of (r, snd parent))
    )
  | CIstatic ->
    let this = (Reason.Rwitness p, TUtils.this_of (Env.get_self env)) in
    make_result env T.CIstatic this
  | CIself ->
    let self =
      match snd (Env.get_self env) with
      | Tclass(c, _, tyl) -> Tclass(c, exact, tyl)
      | self -> self in
    make_result env T.CIself (Reason.Rwitness p, self)
  | CI ((p, id) as c) as e1 ->
    if Env.is_generic_parameter env id
    then
      let r = Reason.Rhint p in
      let tgeneric = (r, Tabstract (AKgeneric id, None)) in
      make_result env (T.CI c) tgeneric
    else
      let class_ = Env.get_class env (snd c) in
      (match class_ with
        | None ->
          make_result env (T.CI c) (Reason.Rwitness p, Typing_utils.tany env)
        | Some class_ ->
          let env, ty =
            resolve_type_arguments_and_check_constraints ~exact ~check_constraints
              env p c e1 (Cls.tparams class_) tal in
          make_result env (T.CI c) ty
      )
  | CIexpr (p, _ as e) ->
      let env, te, ty = expr env e in
      let rec resolve_ety env ty =
        let env, ty = SubType.expand_type_and_solve ~description_of_expected:"an object" env p ty in
        let env, ty = TUtils.fold_unresolved env ty in
        match TUtils.get_base_type env ty with
        | _, Tabstract (AKnewtype (classname, [the_cls]), _) when
            classname = SN.Classes.cClassname -> resolve_ety env the_cls
        | _, Tabstract (AKgeneric _, _)
        | _, Tclass _ -> env, ty
        | r, Tunresolved tyl ->
          let env, tyl = List.map_env env tyl resolve_ety in
          env, (r, Tunresolved tyl)
        | _, Tdynamic as ty -> env, ty
        | _, (Tany | Tprim Tstring | Tabstract (_, None) | Tobject)
              when not (Env.is_strict env) ->
          env, (Reason.Rwitness p, Typing_utils.tany env)
        | r, Tvar _ ->
          Errors.unknown_type "an object" p (Reason.to_string "It is unknown" r);
          env, (Reason.Rwitness p, Typing_utils.terr env)

        | (_, (Terr | Tany | Tnonnull | Tarraykind _ | Toption _
                 | Tprim _ | Tfun _ | Ttuple _
                 | Tabstract ((AKenum _ | AKdependent _ | AKnewtype _), _)
                 | Tanon (_, _) | Tobject | Tshape _)) as ty
          ->
          Errors.expected_class ~suffix:(", but got "^Typing_print.error env ty) p;
          env, (Reason.Rwitness p, Typing_utils.terr env) in
      let env, result_ty = resolve_ety env ty in
      make_result env (T.CIexpr te) result_ty

and call_construct p env class_ params el uel cid =
  let cid = if cid = CIparent then CIstatic else cid in
  let env, tcid, cid_ty = static_class_id ~check_constraints:false p env [] cid in
  let ety_env = {
    type_expansions = [];
    this_ty = cid_ty;
    substs = Subst.make (Cls.tparams class_) params;
    from_class = Some cid;
    validate_dty = None;
  } in
  let env = Phase.check_tparams_constraints ~use_pos:p ~ety_env env (Cls.tparams class_) in
  if (Cls.is_xhp class_) then env, tcid, [], [], (Reason.Rnone, TUtils.tany env) else
  let cstr = Env.get_construct env class_ in
  let mode = Env.get_mode env in
  match (fst cstr) with
    | None ->
      if el <> [] &&
        (FileInfo.is_strict mode || mode = FileInfo.Mpartial) &&
        (Cls.members_fully_known class_)
      then Errors.constructor_no_args p;
      let env, tel, _tyl = exprs env el in
      env, tcid, tel, [], (Reason.Rnone, TUtils.terr env)
    | Some { ce_visibility = vis; ce_type = lazy m; _ } ->
      TVis.check_obj_access p env (Reason.to_pos (fst m), vis);
      let env, m = Phase.localize ~ety_env env m in
      let env, tel, tuel, _ty = call ~expected:None p env m el uel in
      env, tcid, tel, tuel, m

and check_arity ?(did_unpack=false) pos pos_def (arity:int) exp_arity =
  let exp_min = (Typing_defs.arity_min exp_arity) in
  if arity < exp_min
  then Errors.typing_too_few_args pos pos_def;
  match exp_arity with
    | Fstandard (_, exp_max) ->
      let arity = if did_unpack then arity + 1 else arity in
      if arity > exp_max
      then Errors.typing_too_many_args pos pos_def;
    | Fvariadic _ | Fellipsis _ -> ()

and check_lambda_arity lambda_pos def_pos lambda_arity expected_arity =
  let expected_min = Typing_defs.arity_min expected_arity in
  match lambda_arity, expected_arity with
  | Fstandard (lambda_min, _), Fstandard _ ->
    if lambda_min < expected_min
    then Errors.typing_too_few_args lambda_pos def_pos;
    if lambda_min > expected_min
    then Errors.typing_too_many_args lambda_pos def_pos
  | _, _ -> ()

and check_deprecated p { ft_pos; ft_deprecated; _ } =
  match ft_deprecated with
  | Some s -> Errors.deprecated_use p ft_pos s
  | None -> ()

(* The variadic capture argument is an array listing the passed
 * variable arguments for the purposes of the function body; callsites
 * should not unify with it *)
and variadic_param env ft =
  match ft.ft_arity with
    | Fvariadic (_, param) -> env, Some param
    | Fellipsis (_, pos) ->
      env, Some (TUtils.default_fun_param ~pos (Reason.Rvar_param pos, Tany))
    | Fstandard _ -> env, None

and param_modes ?(is_variadic=false) { fp_pos; fp_kind; _ } (pos, e) =
  match fp_kind, e with
  | FPnormal, Unop (Ast.Uref, _) ->
    Errors.pass_by_ref_annotation_unexpected pos fp_pos is_variadic
  | FPnormal, Callconv _ ->
    Errors.inout_annotation_unexpected pos fp_pos is_variadic
  | FPnormal, _
  | FPref, Unop (Ast.Uref, _) -> ()
  | FPref, Callconv (kind, _) ->
    (match kind with
    (* HHVM supports pass-by-ref for arguments annotated as 'inout'. *)
    | Ast.Pinout -> ()
    )
  | FPref, _ ->
    Errors.pass_by_ref_annotation_missing pos fp_pos
  (* HHVM also allows '&' on arguments to inout parameters via interop layer. *)
  | FPinout, Unop (Ast.Uref, _)
  | FPinout, Callconv (Ast.Pinout, _) -> ()
  | FPinout, _ ->
    Errors.inout_annotation_missing pos fp_pos

and inout_write_back env { fp_type; _ } (_, e) =
    match e with
    | Callconv (Ast.Pinout, e1) ->
      (* Translate the write-back semantics of inout parameters.
       *
       * This matters because we want to:
       * (1) make sure we can write to the original argument
       *     (modifiable lvalue check)
       * (2) allow for growing of locals / Tunresolveds (type side effect)
       *     but otherwise unify the argument type with the parameter hint
       *)
      let env, _te, _ty = assign_ (fst e1) Reason.URparam_inout env e1 fp_type in
      env
    | _ -> env

and call ~expected ?method_call_info pos env fty el uel =
  let env, tel, tuel, ty =
    call_ ~expected ~method_call_info pos env fty el uel in
  let new_inference = TypecheckerOptions.new_inference (Env.get_tcopt env) in
  let env = if not new_inference then Env.check_todo env else env in
  env, tel, tuel, ty

and call_ ~expected ~method_call_info pos env fty el uel =
  let make_unpacked_traversable_ty pos ty = MakeType.traversable (Reason.Runpack_param pos) ty in
  let resl = TUtils.try_over_concrete_supertypes env fty begin fun env fty ->
    let env, efty = SubType.expand_type_and_solve
      ~description_of_expected:"a function value" env pos fty in
    match efty with
    | _, (Terr | Tany | Tunresolved [] | Tdynamic) ->
      let el = el @ uel in
      let env, tel = List.map_env env el begin fun env elt ->
        let env, te, _ =
          expr ~expected:(pos, Reason.URparam, (Reason.Rnone, Typing_utils.tany env))
          ~is_func_arg:true env elt
        in
        let env =
          match elt with
          | _, Callconv (Ast.Pinout, e1) ->
            let env, _te, _ty = assign_ (fst e1) Reason.URparam_inout env e1 efty in
            env
          | _, Unop (Ast.Uref, e1) ->
            let env, _te, _ty = assign_ (fst e1) Reason.URparam env e1 efty in
            env
          | _ -> env in
        env, te
      end in
      let env = call_untyped_unpack env uel in
      let ty =
        if snd efty = Tdynamic
        then MakeType.dynamic (Reason.Rdynamic_call pos)
        else (Reason.Rnone, Typing_utils.tany env)
      in
      env, tel, [], ty
    | _, Tunresolved [ty] ->
      call ~expected pos env ty el uel
    | r, Tunresolved tyl ->
      let env, retl = List.map_env env tyl begin fun env ty ->
        let env, _, _, ty = call ~expected pos env ty el uel in env, ty
      end in
      let env, ty = TUtils.in_var env (r, Tunresolved retl) in
      env, [], [], ty
    | r2, Tfun ft ->
      (* Typing of format string functions. It is dependent on the arguments (el)
       * so it cannot be done earlier.
       *)
      let pos_def = Reason.to_pos r2 in
      let env, ft = Typing_exts.retype_magic_func env ft el in
      check_deprecated pos ft;
      let env, var_param = variadic_param env ft in

      (* Force subtype with expected result *)
      let env = check_expected_ty "Call result" env ft.ft_ret expected in
      let env = Env.set_tyvar_variance env ft.ft_ret in
      let is_lambda e = match snd e with Efun _ | Lfun _ -> true | _ -> false in

      let get_next_param_info paraml =
        match paraml with
        | param::paraml ->
          false, Some param, paraml
        | [] ->
          true, var_param, paraml in

      let check_arg env (pos, _ as e) opt_param ~is_variadic =
        match opt_param with
        | Some param ->
          let env, te, ty =
            expr ~is_func_arg:true ~accept_using_var:param.fp_accept_disposable
              ~expected:(pos, Reason.URparam, param.fp_type) env e in
          let env = call_param env param (e, ty) ~is_variadic in
          env, Some (te, ty)
        | None ->
          let env, te, ty = expr ~expected:(pos, Reason.URparam,
            (Reason.Rnone, Typing_utils.tany env)) ~is_func_arg:true env e in
          env, Some (te, ty) in

      let set_tyvar_variance_from_lambda_param env opt_param =
        match opt_param with
        | Some param ->
          let rec set_params_variance env ty =
            let env, ty = Env.expand_type env ty in
            match ty with
            | _, Tunresolved [ty] -> set_params_variance env ty
            | _, Toption ty -> set_params_variance env ty
            | _, Tfun { ft_params; ft_ret; _ } ->
              let env = List.fold ~init:env ~f:(fun env param ->
                Env.set_tyvar_variance env param.fp_type) ft_params in
              Env.set_tyvar_variance env ft_ret ~flip:true
            | _ -> env in
          set_params_variance env param.fp_type
        | None ->
          env in

      (* Given an expected function type ft, check types for the non-unpacked
       * arguments. Don't check lambda expressions if check_lambdas=false *)
      let rec check_args check_lambdas env el paraml =
        match el with
        (* We've got an argument *)
        | (e, opt_result) :: el ->
          (* Pick up next parameter type info *)
          let is_variadic, opt_param, paraml = get_next_param_info paraml in
          let env, one_result = match check_lambdas, is_lambda e with
            | false, false
            | true, true ->
              check_arg env e opt_param ~is_variadic
            | false, true ->
              let env = set_tyvar_variance_from_lambda_param env opt_param in
              env, opt_result
            | true, false ->
              env, opt_result in
          let env, rl, paraml = check_args check_lambdas env el paraml in
          env, (e, one_result)::rl, paraml

        | [] ->
          env, [], paraml in

      (* First check the non-lambda arguments. For generic functions, this
       * is likely to resolve type variables to concrete types *)
      let rl = List.map el (fun e -> (e, None)) in
      let env, rl, _ = check_args false env rl ft.ft_params in
      (* Now check the lambda arguments, hopefully with type variables resolved *)
      let env, rl, paraml = check_args true env rl ft.ft_params in
      (* We expect to see results for all arguments after this second pass *)
      let get_param opt =
        match opt with
        | Some x -> x
        | None -> failwith "missing parameter in check_args" in
      let tel, tys =
        let l = List.map rl (fun (_, opt) -> get_param opt) in
        List.unzip l in
      TR.check_call env method_call_info pos r2 ft tys;
      let env, tuel, arity, did_unpack =
        match uel with
        | [] -> env, [], List.length el, false
        | e :: _ ->
          (* Enforces that e is unpackable. If e is a tuple, check types against
           * parameter types *)
          let env, te, ty = expr env e in
          let env, ety = SubType.expand_type_and_solve
            ~description_of_expected:"an unpackable value" env (fst e) ty in
          match ety with
          | _, Ttuple tyl ->
            let rec check_elements env tyl paraml =
              match tyl with
              | [] -> env
              | ty::tyl ->
                let is_variadic, opt_param, paraml = get_next_param_info paraml in
                match opt_param with
                | None -> env
                | Some param ->
                  let env = call_param env param (e, ty) ~is_variadic in
                  check_elements env tyl paraml in
            let env = check_elements env tyl paraml in
            env, [te], List.length el + List.length tyl, false
          | _ ->
            let param_tyl = List.map paraml (fun param -> param.fp_type) in
            let add_variadic_param_ty param_tyl =
              match var_param with
              | Some param -> param.fp_type :: param_tyl
              | None -> param_tyl in
            let param_tyl = add_variadic_param_ty param_tyl in
            let pos = fst e in
            let env = List.fold_right param_tyl ~init:env
              ~f:(fun param_ty env ->
              let traversable_ty = make_unpacked_traversable_ty pos param_ty in
              Type.sub_type pos Reason.URparam env ety traversable_ty)
            in
            env, [te], List.length el, true
      in
      (* If we unpacked an array, we don't check arity exactly. Since each
       * unpacked array consumes 1 or many parameters, it is nonsensical to say
       * that not enough args were passed in (so we don't do the min check).
       *)
      let () = check_arity ~did_unpack pos pos_def arity ft.ft_arity in
      (* Variadic params cannot be inout so we can stop early *)
      let env = wfold_left2 inout_write_back env ft.ft_params el in
      let env, ret_ty =
        TR.get_adjusted_return_type env method_call_info ft.ft_ret in
      env, tel, tuel, ret_ty
    | r2, Tanon (arity, id) ->
      let env, tel, tyl = exprs ~is_func_arg:true env el in
      let expr_for_unpacked_expr_list env = function
        | [] -> env, [], None, Pos.none
        | (pos, _) as e :: _ ->
          let env, te, ety = expr env e in
          env, [te], Some ety, pos
      in
      let append_tuple_types tyl = function
        | Some (_, Ttuple tuple_tyl) -> tyl @ tuple_tyl
        | _ -> tyl
      in
      let determine_arity env min_arity pos = function
        | None
        | Some (_, Ttuple _) ->
          env, Fstandard (min_arity, min_arity)
        | Some (ety) ->
          (* We need to figure out the underlying type of the unpacked expr type.
           *
           * For example, assume the call is:
           *    $lambda(...$y);
           * where $y is a variadic or collection of strings.
           *
           * $y may have the type Tarraykind or Traversable, however we need to
           * pass Fvariadic a param of type string.
           *
           * Assuming $y has type Tarraykind, in order to get the underlying type
           * we create a fresh_type(), wrap it in a Traversable and make that
           * Traversable a super type of the expr type (Tarraykind). This way
           * we can infer the underlying type and create the correct param for
           * Fvariadic.
           *)
          let env, ty = Env.fresh_type env pos in
          let traversable_ty = make_unpacked_traversable_ty pos ty in
          let env = Type.sub_type pos Reason.URparam env ety traversable_ty in
          let param =
             { fp_pos = pos;
               fp_name = None;
               fp_type = ty;
               fp_kind = FPnormal;
               fp_accept_disposable = false;
               fp_mutability = None;
               fp_rx_annotation = None;
             }
           in
           env, Fvariadic (min_arity, param)
      in
      let env, tuel, uety_opt, uepos = expr_for_unpacked_expr_list env uel in
      let tyl = append_tuple_types tyl uety_opt in
      let env, call_arity = determine_arity env (List.length tyl) uepos uety_opt in
      let anon = Env.get_anonymous env id in
      let fpos = Reason.to_pos r2 in
      (match anon with
        | None ->
          Errors.anonymous_recursive_call pos;
          env, tel, tuel, err_witness env pos
        | Some (reactivity, is_coroutine, ftys, _, anon) ->
          let () = check_arity pos fpos (Typing_defs.arity_min call_arity) arity in
          let tyl = List.map tyl TUtils.default_fun_param in
          let env, _, ty = anon ~el env tyl call_arity in
          let fty =
            (Reason.Rlambda_use pos, Tfun {
              ft_pos = fpos;
              ft_deprecated = None;
              ft_abstract = false;
              ft_is_coroutine = is_coroutine;
              ft_arity = arity;
              ft_tparams = ([], FTKtparams);
              ft_where_constraints = [];
              ft_params = tyl;
              ft_ret = ty;
              ft_reactive = reactivity;
              ft_return_disposable = false;
              ft_mutability = None;
              ft_returns_mutable = false;
              ft_decl_errors = None;
              ft_returns_void_to_rx = false;
            }) in
          ftys := TUtils.add_function_type env fty !ftys;
          env, tel, tuel, ty)
    | _, Tarraykind _ when not (Env.is_strict env) ->
      (* Relaxing call_user_func to work with an array in partial mode *)
      let env = call_untyped_unpack env uel in
      env, [], [], (Reason.Rnone, Typing_utils.tany env)
    | ty ->
      bad_call env pos ty;
      let env = call_untyped_unpack env uel in
      env, [], [], err_witness env pos
  end in
  match resl with
  | [res] -> res
  | _ ->
    bad_call env pos fty;
    let env = call_untyped_unpack env uel in
    env, [], [], err_witness env pos

and call_param env param ((pos, _ as e), arg_ty) ~is_variadic =
  (match param.fp_name with
  | None -> ()
  | Some name -> Typing_suggest.save_param name env param.fp_type arg_ty
  );
  param_modes ~is_variadic param e;

  (* When checking params the type 'x' may be expression dependent. Since
   * we store the expression id in the local env for Lvar, we want to apply
   * it in this case.
   *)
  let env, dep_ty = match snd e with
    | Lvar _ -> ExprDepTy.make env (CIexpr e) arg_ty
    | _ -> env, arg_ty in
  Type.coerce_type pos Reason.URparam env dep_ty param.fp_type

and call_untyped_unpack env uel = match uel with
  (* In the event that we don't have a known function call type, we can still
   * verify that any unpacked arguments (`...$args`) are something that can
   * be actually unpacked. *)
  | [] -> env
  | e::_ -> begin
    let env, _, ety = expr env e in
    match ety with
    | _, Ttuple _ -> env (* tuples are always fine *)
    | _ -> begin
      let pos = fst e in
      let env, ty = Env.fresh_type env pos in
      let unpack_r = Reason.Runpack_param pos in
      let unpack_ty = MakeType.traversable unpack_r ty in
      Type.coerce_type pos Reason.URparam env ety unpack_ty
    end
  end

and bad_call env p ty =
  Errors.bad_call p (Typing_print.error env ty)

(* to be used to throw typing error if failing to satisfy subtype relation *)
and enforce_sub_ty env p ty1 ty2 =
  let env = Type.sub_type p Reason.URnone env ty1 ty2 in
  Env.expand_type env ty1

(* throws typing error if neither t <: ty nor t <: dynamic, and adds appropriate
 * constraint to env otherwise *)
and check_type ty p env t =
  let is_ty = SubType.is_sub_type env t ty in
  let is_dynamic = SubType.is_sub_type env t (MakeType.dynamic (fst ty)) in
  match is_ty, is_dynamic with
  | false, true -> enforce_sub_ty env p t (MakeType.dynamic (fst ty))
  | _ -> enforce_sub_ty env p t ty

(* does check_type with num and then gives back normalized type and env *)
and check_num env p t r =
  let env2, t2 = check_type (MakeType.num r) p env t in
  let r2 = fst t2 in
  env2, if SubType.is_sub_type env2 t (MakeType.int r2)
    then MakeType.int r2
    else if SubType.is_sub_type env2 t (MakeType.float r2)
    then MakeType.float r2
    else if SubType.is_sub_type env2 t (MakeType.num r2)
    then MakeType.num r2
    else MakeType.dynamic r2

(* does check_type with int and then gives back normalized type and env *)
and check_int env p t r =
  let env2, t2 = check_type (MakeType.int r) p env t in
  let r2 = fst t2 in
  env2, if SubType.is_sub_type env2 t (MakeType.int r2)
    then MakeType.int r2
    else MakeType.dynamic r2

and unop ~is_func_arg ~array_ref_ctx p env uop te ty =
  let make_result env te result_ty =
    env, T.make_typed_expr p result_ty (T.Unop(uop, te)), result_ty in
  let is_any = TUtils.is_any env in
  match uop with
  | Ast.Unot ->
    if is_any ty
    then make_result env te ty
    else (* args isn't any or a variant thereof so can actually do stuff *)
      (* !$x (logical not) works with any type, so we just return Tbool *)
      make_result env te (MakeType.bool (Reason.Rlogic_ret p))
  | Ast.Utild ->
      if is_any ty
      then make_result env te ty
      else (* args isn't any or a variant thereof so can actually do stuff *)
      let env, t = check_int env p ty (Reason.Rbitwise p) in
      begin
        match snd t with
        | Tdynamic -> make_result env te (MakeType.dynamic (Reason.Rbitwise_dynamic p))
        | _ -> make_result env te (MakeType.int (Reason.Rbitwise_ret p))
      end
  | Ast.Uincr
  | Ast.Upincr
  | Ast.Updecr
  | Ast.Udecr ->
      (* increment and decrement operators modify the value,
       * check for immutability violation here *)
      begin
        match te with
        | _, T.ImmutableVar (p, x) ->
            Errors.let_var_immutability_violation p (Local_id.get_name x);
            expr_error env p (Reason.Rwitness p)
        | _ ->
        if is_any ty
        then make_result env te ty
        else (* args isn't any or a variant thereof so can actually do stuff *)
        let env, t = check_num env p ty (Reason.Rarith p) in
        let env =
          if Env.env_local_reactive env then
          Typing_mutability.handle_assignment_mutability env te (Some (snd te))
          else env
        in
        match snd t with
        | Tprim Tfloat ->
          make_result env te (MakeType.float (Reason.Rarith_ret_float (p, fst t, Reason.Aonly)))
        | Tprim Tnum ->
          make_result env te (MakeType.num (Reason.Rarith_ret_num (p, fst t, Reason.Aonly)))
        | Tprim Tint -> make_result env te (MakeType.int (Reason.Rarith_ret_int p))
        | Tdynamic -> make_result env te (MakeType.dynamic (Reason.Rincdec_dynamic p))
        | _ ->  make_result env te (MakeType.num (Reason.Rarith_ret p))
      end
  | Ast.Uplus
  | Ast.Uminus ->
      if is_any ty
      then make_result env te ty
      else (* args isn't any or a variant thereof so can actually do stuff *)
      let env, t = check_num env p ty (Reason.Rarith p) in
      begin
        match snd t with
        | Tprim Tfloat ->
          make_result env te (MakeType.float (Reason.Rarith_ret_float (p, fst t, Reason.Aonly)))
        | Tprim Tnum ->
          make_result env te (MakeType.num (Reason.Rarith_ret_num (p, fst t, Reason.Aonly)))
        | Tprim Tint -> make_result env te (MakeType.int (Reason.Rarith_ret_int p))
        | _ -> make_result env te (MakeType.num (Reason.Rarith_ret p))
      end
  | Ast.Uref ->
      if Env.env_local_reactive env
         && not (TypecheckerOptions.unsafe_rx (Env.get_tcopt env))
      then Errors.reference_in_rx p;

      if array_ref_ctx <> NoArray
      then
        match array_ref_ctx with
        | ElementAccess -> Errors.binding_ref_to_array p (* &$x[0]; *)
        | ElementAssignment -> Errors.binding_ref_in_array p (* $x[0] = &y; *)
        | NoArray -> ()
      else if is_func_arg (* Normal function calls, excludes e.g. isset(&x); *)
      then
        match snd te with
        | T.Array_get _ ->
           (* foo(&x[0]); *)
           Errors.passing_array_cell_by_ref p
        | _ ->
           (* foo(&x); // permitted *)
           ()
      else Errors.reference_expr p; (* &$x; *)

      (* any check omitted because would return the same anyway *)
      make_result env te ty
  | Ast.Usilence ->
      (* Silencing does not change the type *)
      (* any check omitted because would return the same anyway *)
      make_result env te ty

and binop p env bop p1 te1 ty1 p2 te2 ty2 =
  let make_result env te1 te2 ty =
    env, T.make_typed_expr p ty (T.Binop (bop, te1, te2)), ty in
  let is_any = TUtils.is_any env in
  let contains_any = (is_any ty1) || (is_any ty2) in
  match bop with
  | Ast.Plus when not contains_any ->
    let env, t1 = check_num env p ty1 (Reason.Rarith p1) in
    let env, t2 = check_num env p ty2 (Reason.Rarith p2) in
    (* postcondition: t1 and t2 are dynamic or subtypes of num and
      annotated as such, or we are e.g. HH_FIXMEing *)
    begin
      match snd t1, snd t2 with
      | Tprim Tint, Tprim Tint -> make_result env te1 te2 (MakeType.int (Reason.Rarith_ret_int p))
      | Tprim Tfloat, _ ->
        make_result env te1 te2 (MakeType.float (Reason.Rarith_ret_float (p, fst t1, Reason.Afirst)))
      | _, Tprim Tfloat ->
        make_result env te1 te2 (MakeType.float (Reason.Rarith_ret_float (p, fst t2, Reason.Asecond)))
      | Tprim Tnum, _ ->
        make_result env te1 te2 (MakeType.num (Reason.Rarith_ret_num (p, fst t1, Reason.Afirst)))
      | _, Tprim Tnum ->
        make_result env te1 te2 (MakeType.num (Reason.Rarith_ret_num (p, fst t2, Reason.Asecond)))
      | Tdynamic, Tdynamic -> make_result env te1 te2 (MakeType.dynamic (Reason.Rsum_dynamic p))
      | _ -> make_result env te1 te2 (MakeType.num (Reason.Rarith_ret p))
    end
  | Ast.Minus | Ast.Star when not contains_any ->
    let env, t1 = check_num env p ty1 (Reason.Rarith p1) in
    let env, t2 = check_num env p ty2 (Reason.Rarith p2) in
    (* postcondition: t1 and t2 are dynamic or subtypes of num and
      annotated as such, or we are e.g. HH_FIXMEing *)
    begin
      match snd t1, snd t2 with
      | Tprim Tint, Tprim Tint -> make_result env te1 te2 (MakeType.int (Reason.Rarith_ret_int p))
      | Tprim Tfloat, _ ->
        make_result env te1 te2 (MakeType.float (Reason.Rarith_ret_float (p, fst t1, Reason.Afirst)))
      | _, Tprim Tfloat ->
        make_result env te1 te2 (MakeType.float (Reason.Rarith_ret_float (p, fst t2, Reason.Asecond)))
      | Tprim Tnum, _ ->
        make_result env te1 te2 (MakeType.num (Reason.Rarith_ret_num (p, fst t1, Reason.Afirst)))
      | _, Tprim Tnum ->
        make_result env te1 te2 (MakeType.num (Reason.Rarith_ret_num (p, fst t2, Reason.Asecond)))
      | _ -> make_result env te1 te2 (MakeType.num (Reason.Rarith_ret p))
    end
  | Ast.Slash | Ast.Starstar when not contains_any ->
    let env, t1 = check_num env p ty1 (Reason.Rarith p1) in
    let env, t2 = check_num env p ty2 (Reason.Rarith p2) in
    (* postcondition: t1 and t2 are dynamic or subtypes of num and
      annotated as such, or we are e.g. HH_FIXMEing *)
    let r = match bop with
      | Ast.Slash -> Reason.Rret_div p
      | _ -> Reason.Rarith_ret p in
    begin
      match snd t1, snd t2 with
      | Tprim Tfloat, _ ->
        make_result env te1 te2 (MakeType.float (Reason.Rarith_ret_float (p, fst t1, Reason.Afirst)))
      | _, Tprim Tfloat ->
        make_result env te1 te2 (MakeType.float (Reason.Rarith_ret_float (p, fst t2, Reason.Asecond)))
      | _ -> make_result env te1 te2 (MakeType.num r)
    end
  | Ast.Percent | Ast.Ltlt | Ast.Gtgt when not contains_any ->
    let env, _ = check_int env p ty1 (Reason.Rarith p1) in
    let env, _ = check_int env p ty2 (Reason.Rarith p2) in
    (* postcondition: t1 and t2 are dynamic or int and
      annotated as such, or we are e.g. HH_FIXMEing *)
    let r = match bop with
      | Ast.Percent -> Reason.Rarith_ret_int p
      | _ -> Reason.Rbitwise_ret p in
    make_result env te1 te2 (MakeType.int r)
  | Ast.Xor | Ast.Amp | Ast.Bar when not contains_any ->
    let env, t1 = check_int env p ty1 (Reason.Rbitwise p1) in
    let env, t2 = check_int env p ty2 (Reason.Rbitwise p2) in
    (* postcondition: t1 and t2 are dynamic or int and
      annotated as such, or we are e.g. HH_FIXMEing *)
    begin
      match snd t1, snd t2 with
      | Tdynamic, Tdynamic -> make_result env te1 te2 (MakeType.dynamic (Reason.Rbitwise_dynamic p))
      | _ -> make_result env te1 te2 (MakeType.int (Reason.Rbitwise_ret p))
    end
  | Ast.Eqeq  | Ast.Diff | Ast.Eqeqeq | Ast.Diff2 ->
      make_result env te1 te2 (MakeType.bool (Reason.Rcomp p))
  | Ast.Lt | Ast.Lte | Ast.Gt | Ast.Gte | Ast.Cmp ->
      let ty_num = MakeType.num (Reason.Rcomp p) in
      let ty_int = MakeType.int (Reason.Rcomp p) in
      let ty_bool = MakeType.bool (Reason.Rcomp p) in
      let ty_result = match bop with Ast.Cmp -> ty_int | _ -> ty_bool in
      let ty_string = MakeType.string (Reason.Rcomp p) in
      let ty_datetime = MakeType.datetime (Reason.Rcomp p) in
      let ty_datetimeimmutable = MakeType.datetime_immutable (Reason.Rcomp p) in
      let ty_dynamic = MakeType.dynamic (Reason.Rcomp p) in
      let both_sub tyl =
        (List.exists tyl ~f:(SubType.is_sub_type env ty1))
        && (List.exists tyl ~f:(SubType.is_sub_type env ty2)) in
      (*
       * Comparison here is allowed when both args are num, both string, or both
       * DateTime | DateTimeImmutable. Alternatively, either or both args can be
       * dynamic. We use both_sub to check that both arguments subtype a type.
       *
       * This actually does not properly handle union types. For instance,
       * DateTime | DateTimeImmutable is neither a subtype of DateTime nor
       * DateTimeImmutable, but it will be the type of an element coming out
       * of a vector containing both. Further, dynamic could be comparable to
       * num | string | DateTime | DateTimeImmutable | dynamic. Better union
       * handling would be an improvement.
       *)
      if not contains_any &&
        not (both_sub [ty_num; ty_dynamic]
          || both_sub [ty_string; ty_dynamic]
          || both_sub [ty_datetime; ty_datetimeimmutable; ty_dynamic])
      then begin
        let ty1 = Typing_expand.fully_expand env ty1 in
        let ty2 = Typing_expand.fully_expand env ty2 in
        let tys1 = Typing_print.error env ty1 in
        let tys2 = Typing_print.error env ty2 in
        Errors.comparison_invalid_types p
          (Reason.to_string ("This is " ^ tys1) (fst ty1))
          (Reason.to_string ("This is " ^ tys2) (fst ty2))
      end;
      make_result env te1 te2 ty_result
  | Ast.Dot ->
    (* A bit weird, this one:
     *   function(Stringish | string, Stringish | string) : string)
     *)
      let env = SubType.sub_string p1 env ty1 in
      let env = SubType.sub_string p2 env ty2 in
      make_result env te1 te2 (MakeType.string (Reason.Rconcat_ret p))
  | Ast.Barbar | Ast.Ampamp | Ast.LogXor ->
      make_result env te1 te2 (MakeType.bool (Reason.Rlogic_ret p))
  | Ast.QuestionQuestion
  | Ast.Eq _ when not contains_any ->
      assert false
  | _ ->
    assert contains_any;
    if is_any ty1
    then make_result env te1 te2 ty1
    else make_result env te1 te2 ty2

and make_a_local_of env e =
  match e with
  | p, Class_get ((_, cname), CGstring (_, member_name)) ->
    let env, local = Env.FakeMembers.make_static p env cname member_name in
    env, Some (p, local)
  | p, Obj_get ((_, This | _, Lvar _ as obj), (_, Id (_, member_name)), _) ->
    let env, local = Env.FakeMembers.make p env obj member_name in
    env, Some (p, local)
  | _, Lvar x
  | _, ImmutableVar x
  | _, Dollardollar x -> env, Some x
  | _ -> env, None

(* This function captures the common bits of logic behind refinement
 * of the type of a local variable or a class member variable as a
 * result of a dynamic check (e.g., nullity check, simple type check
 * using functions like is_int, is_string, is_array etc.).  The
 * argument refine is a function that takes the type of the variable
 * and returns a refined type (making necessary changes to the
 * environment, which is threaded through).
 *)
and refine_lvalue_type env ((_p, ty), _ as te) ~refine =
  let env, ty = refine env ty in
  let e = T.to_nast_expr te in
  let env, localopt = make_a_local_of env e in
  (* TODO TAST: generate an assignment to the fake local in the TAST *)
  match localopt with
  | Some local ->
    set_local env local ty
  | None -> env

and condition_nullity ~nonnull (env: Env.env) te =
  match te with
  (* assignment: both the rhs and lhs of the '=' must be made null/non-null *)
  | _, T.Binop (Ast.Eq None, var, te) ->
      let env = condition_nullity ~nonnull env te in
      condition_nullity ~nonnull env var
  (* case where `Shapes::idx(...)` must be made null/non-null *)
  | _, T.Call (
      _,
      (_, T.Class_const ((_, T.CI (_, shapes)), (_, idx))),
      _,
      [shape; field],
      _)
    when shapes = SN.Shapes.cShapes && idx = SN.Shapes.idx ->
    let field = T.to_nast_expr field in
    let refine env shape_ty = if nonnull
      then Typing_shapes.shapes_idx_not_null env shape_ty field
      else env, shape_ty in
    refine_lvalue_type env shape ~refine
  | (p, _), _ ->
    let refine env ty = if nonnull
      then TUtils.non_null env p ty
      else env, ty in
    refine_lvalue_type env te ~refine

and condition_isset env = function
  | _, T.Array_get (x, _) -> condition_isset env x
  | v -> condition_nullity ~nonnull:true env v

(**
 * Build an environment for the true or false branch of
 * conditional statements.
 *)
and condition ?lhs_of_null_coalesce env tparamet
    ((p, ty as pty), e as te: Tast.expr) =
  let condition = condition ?lhs_of_null_coalesce in
  let enable_instanceof_refinement =
    not (TypecheckerOptions.disable_instanceof_refinement (Env.get_tcopt env))
  in
  match e with
  | T.True
  | T.Expr_list [] when not tparamet ->
    LEnv.drop_cont env C.Next
  | T.False when tparamet ->
    LEnv.drop_cont env C.Next
  | T.Expr_list [] -> env
  | T.Expr_list [x] ->
      condition env tparamet x
  | T.Expr_list (_::xs) ->
      condition env tparamet (pty, T.Expr_list xs)
  | T.Call (Cnormal, (_, T.Id (_, func)), _, [param], [])
    when SN.PseudoFunctions.isset = func && tparamet &&
    not (Env.is_strict env) ->
      condition_isset env param
  | T.Call (Cnormal, (_, T.Id (_, func)), _, [te], [])
    when SN.StdlibFunctions.is_null = func ->
      condition_nullity ~nonnull:(not tparamet) env te
  | T.Binop ((Ast.Eqeq | Ast.Eqeqeq), (_, T.Null), e)
  | T.Binop ((Ast.Eqeq | Ast.Eqeqeq), e, (_, T.Null)) ->
      condition_nullity ~nonnull:(not tparamet) env e
  | (T.Lvar _ | T.Obj_get _ | T.Class_get _ | T.Binop (Ast.Eq None, _, _)) ->
      let env, ety = Env.expand_type env ty in
      (match ety with
      | _, Tarraykind (AKany | AKempty)
      | _, Tprim Tbool -> env
      | _, (Terr | Tany | Tnonnull | Tarraykind _ | Toption _ | Tdynamic
        | Tprim _ | Tvar _ | Tfun _ | Tabstract _ | Tclass _
        | Ttuple _ | Tanon (_, _) | Tunresolved _ | Tobject | Tshape _
        ) ->
          condition_nullity ~nonnull:tparamet env te)
  | T.Binop ((Ast.Diff | Ast.Diff2 as op), e1, e2) ->
      let op = if op = Ast.Diff then Ast.Eqeq else Ast.Eqeqeq in
      condition env (not tparamet) (pty, T.Binop (op, e1, e2))
  | T.Id (_, s) when s = SN.Rx.is_enabled ->
      (* when Rx\IS_ENABLED is false - switch env to non-reactive *)
      if not tparamet
      then Env.set_env_reactive env Nonreactive
      else env
  | T.Binop ((Ast.Ampamp | Ast.Barbar) as bop, e1, e2)
    when tparamet = (bop = Ast.Ampamp) ->
      let env = condition env tparamet e1 in
      (* This is necessary in case there is an assignment in e2
       * We essentially redo what has been undone in the
       * `Binop (AMpamp|BArbar)` case of `expr` *)
      let env, _, _ = expr env (Tast.to_nast_expr e2) in
      let env = condition env tparamet e2 in
      env
  | T.Call (Cnormal, ((p, _), T.Id (_, f)), _, [lv], [])
    when tparamet && f = SN.StdlibFunctions.is_array ->
      is_array env `PHPArray p f lv
  | T.Call (
      Cnormal,
      (_, T.Class_const ((_, T.CI (_, class_name)), (_, method_name))),
      _,
      [shape; field],
      [])
    when tparamet && class_name = SN.Shapes.cShapes && method_name = SN.Shapes.keyExists ->
      key_exists env p shape field
  | T.Unop (Ast.Unot, e) ->
      condition env (not tparamet) e
  | T.InstanceOf (ivar, (_, cid))
    when enable_instanceof_refinement && tparamet && is_instance_var (T.to_nast_expr ivar) ->
      let ivar = T.to_nast_expr ivar in
      (* Check the expession and determine its static type *)
      let env, _te, x_ty = raw_expr env ivar in

      (* What is the local variable bound to the expression? *)
      let env, ((ivar_pos, _) as ivar) = get_instance_var env ivar in

      (* The position p here is not really correct... it's the position
       * of the instanceof expression, not the class id. But we don't store
       * position data for the latter. *)
      let env, _te, obj_ty = static_class_id ~check_constraints:false p env []
        (T.to_nast_class_id_ cid) in

      let safe_instanceof_enabled =
        TypecheckerOptions.experimental_feature_enabled
          (Env.get_tcopt env) TypecheckerOptions.experimental_instanceof in
      let rec resolve_obj env obj_ty =
        (* Expand so that we don't modify x. Also, solve under new-inference
         * if it's a type variable *)
        let env, obj_ty = SubType.expand_type_and_solve
          ~description_of_expected:"a value" env ivar_pos obj_ty in
        match obj_ty with
        (* If it's a generic that's expression dependent, we need to
          look at all of its upper bounds and create an unresolved type to
          represent all of the possible types.
        *)
        | r, Tabstract (AKgeneric s, _) when AbstractKind.is_generic_dep_ty s ->
          let upper_bounds = TySet.elements (Env.get_upper_bounds env s) in
          let env, tyl = List.map_env env upper_bounds resolve_obj in
          env, (r, Tunresolved tyl)
        | _, Tabstract (AKgeneric name, _) ->
          if safe_instanceof_enabled
          then Errors.instanceof_generic_classname p name;
          env, obj_ty
        | _, Tabstract (AKdependent (`this, []), Some (_, Tclass _)) ->
          env, obj_ty
        | _, Tabstract ((AKdependent _ | AKnewtype _), Some ty) ->
          resolve_obj env ty
        | _, Tclass ((_, cid as _c), _, tyl) ->
          begin match Env.get_class env cid with
            (* Why would this happen? *)
            | None ->
              env, (Reason.Rwitness ivar_pos, Tobject)

            | Some class_info ->
              if SubType.is_sub_type env x_ty obj_ty
              then
                (* If the right side of the `instanceof` object is
                 * a super type of what we already knew. In this case,
                 * since we already have a more specialized object, we
                 * don't touch the original object. Check out the unit
                 * test srecko.php if this is unclear.
                 *
                 * Note that if x_ty is Typing_utils.tany env, no amount of subtype
                 * checking will be able to specify it
                 * further. This is arguably desirable to maintain
                 * the invariant that removing annotations gets rid
                 * of typing errors in partial mode (See also
                 * t3216948).  *)
                env, x_ty
              else
              if tyl = [] || safe_instanceof_enabled
              then safe_instanceof env p _c class_info ivar_pos x_ty obj_ty
              else env, obj_ty
          end
        | r, Tunresolved tyl ->
          let env, tyl = List.map_env env tyl resolve_obj in
          env, (r, Tunresolved tyl)
        | _, (Terr | Tany | Tnonnull| Tarraykind _ | Tprim _ | Tvar _
            | Tfun _ | Tabstract ((AKenum _ | AKnewtype _ | AKdependent _), _)
            | Ttuple _ | Tanon (_, _) | Toption _ | Tobject | Tshape _
            | Tdynamic) ->
          env, (Reason.Rwitness ivar_pos, Tobject)
      in
      let env, x_ty = resolve_obj env obj_ty in
      set_local env ivar x_ty
  | T.Is (ivar, h) when is_instance_var (T.to_nast_expr ivar) ->
    (* Stash env so we don't return an updated one if we don't refine *)
    let env' = env in
    let ety_env = { (Phase.env_with_self env) with from_class = Some CIstatic; } in
    let env, hint_ty = Phase.localize_hint ~ety_env env h in
    let env, hint_ty = Env.expand_type env hint_ty in
    let reason = Reason.Ris p in
    let refine_type hint_ty =
      let ivar_pos, ivar_ty = fst ivar in
      let env, ivar = get_instance_var env (T.to_nast_expr ivar) in
      let env, hint_ty =
        if snd hint_ty <> Tdynamic && SubType.is_sub_type env ivar_ty hint_ty
        then env, ivar_ty
        else safely_refine_type env p reason ivar_pos ivar_ty hint_ty in
      set_local env ivar hint_ty
    in
    begin match snd hint_ty with
    | _ when tparamet -> refine_type hint_ty
    | Tprim Nast.Tnull -> refine_type (reason, Tnonnull)
    | _ -> env'
    end
  | _ -> env

and safely_refine_type env p reason ivar_pos ivar_ty hint_ty =
  match snd ivar_ty, snd hint_ty with
    | _, Tclass ((_, cid) as _c, _, tyl) ->
      begin match Env.get_class env cid with
        | Some class_info ->
          let env, tparams_with_new_names, tyl_fresh =
            isexpr_generate_fresh_tparams env class_info reason tyl in
          safely_refine_class_type
            env p _c class_info ivar_ty hint_ty reason tparams_with_new_names
            tyl_fresh
        | None ->
          env, (Reason.Rwitness ivar_pos, Tobject)
      end
    | Ttuple ivar_tyl, Ttuple hint_tyl
      when (List.length ivar_tyl) = (List.length hint_tyl) ->
      let env, tyl =
        List.map2_env env ivar_tyl hint_tyl begin fun env ivar_ty hint_ty ->
          safely_refine_type env p reason ivar_pos ivar_ty hint_ty
        end
      in
      env, (reason, Ttuple tyl)
    | _, Tnonnull ->
      TUtils.non_null env p ivar_ty
    | _, (Tany | Tprim _ | Toption _ | Ttuple _
        | Tshape _ | Tvar _ | Tabstract _ | Tarraykind _ | Tanon _
        | Tunresolved _ | Tobject | Terr | Tfun _  | Tdynamic) ->
      (* TODO(kunalm) Implement the type refinement for each type *)
      env, hint_ty

and safe_instanceof env p class_name class_info ivar_pos ivar_ty obj_ty =
  (* Generate fresh names consisting of formal type parameter name
   * with unique suffix *)
  let env, (tparams_with_new_names : (decl tparam * string) option list) =
    List.map_env env (Cls.tparams class_info)
      (fun env ({tp_name = (_, name); tp_reified = reified; tp_user_attributes; _ } as tp) ->
        let enforceable = Attributes.mem SN.UserAttributes.uaEnforceable tp_user_attributes in
        let newable = Attributes.mem SN.UserAttributes.uaNewable tp_user_attributes in
        let env, name = Env.add_fresh_generic_parameter env name ~reified ~enforceable ~newable in
        env, Some (tp, name)) in
  let new_names = List.map
    ~f:(fun x -> snd @@ Option.value_exn x)
    tparams_with_new_names in
  let s =
      snd class_name ^ "<" ^
      String.concat ~sep:"," new_names
      ^ ">" in
  let reason = Reason.Rinstanceof (ivar_pos, s) in
  let tyl_fresh = List.map
      ~f:(fun new_name -> (reason, Tabstract (AKgeneric new_name, None)))
      new_names in
  let env, obj_ty =
    safely_refine_class_type
      env p class_name class_info ivar_ty obj_ty reason tparams_with_new_names tyl_fresh in
  env, obj_ty

(** If we are dealing with a refinement like
      $x is MyClass<A, B>
    then class_info is the class info of MyClass and hint_tyl corresponds
    to A, B. *)
and isexpr_generate_fresh_tparams env class_info reason hint_tyl =
  let tparams_len = List.length (Cls.tparams class_info) in
  let hint_tyl = List.take hint_tyl tparams_len in
  let pad_len = tparams_len - (List.length hint_tyl) in
  let hint_tyl =
    List.map hint_tyl (fun x -> Some x) @ (List.init pad_len (fun _ -> None)) in
  let replace_wildcard env hint_ty ({ tp_name = (_, tparam_name); tp_reified = reified; tp_user_attributes; _ } as tp) =
    let enforceable = Attributes.mem SN.UserAttributes.uaEnforceable tp_user_attributes in
    let newable = Attributes.mem SN.UserAttributes.uaNewable tp_user_attributes in
    match hint_ty with
      | Some (_, Tabstract (AKgeneric name, _))
        when Env.is_fresh_generic_parameter name ->
        env, (Some (tp, name), (reason, Tabstract (AKgeneric name, None)))
      | Some ty ->
        env, (None, ty)
      | None ->
        let env, new_name = Env.add_fresh_generic_parameter env tparam_name ~reified ~enforceable ~newable in
        env, (Some (tp, new_name), (reason, Tabstract (AKgeneric new_name, None)))
  in
  let env, tparams_and_tyl = List.map2_env env hint_tyl (Cls.tparams class_info)
    ~f:replace_wildcard in
  let tparams_with_new_names, tyl_fresh = List.unzip tparams_and_tyl in
  env, tparams_with_new_names, tyl_fresh

and safely_refine_class_type
  env p class_name class_info ivar_ty obj_ty reason
  (tparams_with_new_names : (decl tparam * string) option list)
  tyl_fresh =
  (* Type of variable in block will be class name
   * with fresh type parameters *)
  let obj_ty = (fst obj_ty, Tclass (class_name, Nonexact, tyl_fresh)) in

  (* Add in constraints as assumptions on those type parameters *)
  let ety_env = {
    type_expansions = [];
    substs = Subst.make (Cls.tparams class_info) tyl_fresh;
    this_ty = obj_ty; (* In case `this` appears in constraints *)
    from_class = None;
    validate_dty = None;
  } in
  let add_bounds env (t, ty_fresh) =
      List.fold_left t.tp_constraints ~init:env ~f:begin fun env (ck, ty) ->
        (* Substitute fresh type parameters for
         * original formals in constraint *)
        let env, ty = Phase.localize ~ety_env env ty in
        SubType.add_constraint p env ck ty_fresh ty end in
  let env =
    List.fold_left (List.zip_exn (Cls.tparams class_info) tyl_fresh)
      ~f:add_bounds ~init:env in

  (* Finally, if we have a class-test on something with static class type,
   * then we can chase the hierarchy and decompose the types to deduce
   * further assumptions on type parameters. For example, we might have
   *   class B<Tb> { ... }
   *   class C extends B<int>
   * and have obj_ty = C and x_ty = B<T> for a generic parameter T.
   * Then SubType.add_constraint will deduce that T=int and add int as
   * both lower and upper bound on T in env.lenv.tpenv
   *)
  let env = SubType.add_constraint p env Ast.Constraint_as obj_ty ivar_ty in

  (* It's often the case that the fresh name isn't necessary. For
   * example, if C<T> extends B<T>, and we have $x:B<t> for some type t
   * then $x instanceof C should refine to $x:C<t>.
   * We take a simple approach:
   *    For a fresh type parameter T#1, if
   *      - There is an eqality constraint T#1 = t,
   *      - T#1 is covariant, and T#1 has only one upper bound t
   *      - T#1 is contravariant, and t <: T#1 has only one lower bount t,
   *    then replace T#1 with t.
   * This is done in Env.simplify_tpenv
   *)
  let tparam_names = List.filter_map tparams_with_new_names
    ~f:(Option.map ~f:(fun (tp, name) -> (name, tp.tp_variance))) in
  let env, tparam_substs = Env.simplify_tpenv env tparam_names reason in
  let tyl_fresh = List.map2_exn tyl_fresh tparams_with_new_names
    ~f:(fun orig_ty tparam_opt ->
      match tparam_opt with
      | None -> orig_ty
      | Some (_tp, name) -> SMap.find name tparam_substs) in
  let obj_ty_simplified = (fst obj_ty, Tclass (class_name, Nonexact, tyl_fresh)) in
  env, obj_ty_simplified

and is_instance_var = function
  | _, (Lvar _ | This | Dollardollar _) -> true
  | _, Obj_get ((_, This), (_, Id _), _) -> true
  | _, Obj_get ((_, Lvar _), (_, Id _), _) -> true
  | _, Class_get (_, _) -> true
  | _ -> false

and get_instance_var env = function
  | p, Class_get ((_, cname), CGstring (_, member_name)) ->
    let env, local = Env.FakeMembers.make_static p env cname member_name in
    env, (p, local)
  | p, Obj_get ((_, This | _, Lvar _ as obj), (_, Id (_, member_name)), _) ->
    let env, local = Env.FakeMembers.make p env obj member_name in
    env, (p, local)
  | _, Dollardollar (p, x)
  | _, Lvar (p, x) -> env, (p, x)
  | p, This -> env, (p, this)
  | _ -> failwith "Should only be called when is_instance_var is true"

(* Refine type for is_array, is_vec, is_keyset and is_dict tests
 * `pred_name` is the function name itself (e.g. 'is_vec')
 * `p` is position of the function name in the source
 * `arg_expr` is the argument to the function
 *)
and is_array env ty p pred_name arg_expr =
  refine_lvalue_type env arg_expr ~refine:begin fun env arg_ty ->
    let r = Reason.Rpredicated (p, pred_name) in
    let env, tarrkey_name = Env.add_fresh_generic_parameter env "Tk" ~reified:Nast.Erased ~enforceable:false ~newable:false in
    let tarrkey = (r, Tabstract (AKgeneric tarrkey_name, None)) in
    let env = SubType.add_constraint p env Ast.Constraint_as tarrkey (MakeType.arraykey r) in
    let env, tfresh_name = Env.add_fresh_generic_parameter env "T" ~reified:Nast.Erased ~enforceable:false ~newable:false in
    let tfresh = (r, Tabstract (AKgeneric tfresh_name, None)) in
    (* This is the refined type of e inside the branch *)
    let refined_ty =
      match ty with
      | `HackDict ->
        MakeType.dict r tarrkey tfresh
      | `HackVec ->
        MakeType.vec r tfresh
      | `HackKeyset ->
        MakeType.keyset r tarrkey
      | `PHPArray ->
        let safe_isarray_enabled =
          TypecheckerOptions.experimental_feature_enabled
          (Env.get_tcopt env) TypecheckerOptions.experimental_isarray in
        if safe_isarray_enabled
        then (r, Tarraykind (AKvarray_or_darray tfresh))
        else (r, Tarraykind AKany) in
    (* Add constraints on generic parameters that must
     * hold for refined_ty <:arg_ty. For example, if arg_ty is Traversable<T>
     * and refined_ty is keyset<T#1> then we know T#1 <: T *)
    let env = SubType.add_constraint p env Ast.Constraint_as refined_ty arg_ty in
    env, refined_ty
  end

and key_exists env pos shape field =
  let field = T.to_nast_expr field in
  refine_lvalue_type env shape ~refine:begin fun env shape_ty ->
    match TUtils.shape_field_name env field with
    | None -> env, shape_ty
    | Some field_name -> Typing_shapes.refine_shape field_name pos env shape_ty
  end

and string2 env idl =
  let env, tel =
    List.fold_left idl ~init:(env,[]) ~f:begin fun (env,tel) x ->
      let env, te, ty = expr env x in
      let p = fst x in
      let env = SubType.sub_string p env ty in
      env, te::tel
    end in
  env, List.rev tel

(* If the current class inherits from classes that take type arguments, we need
 * to check that the arguments provided are consistent with the constraints on
 * the type parameters. *)
and check_implements_tparaml (env: Env.env) ht =
  let _r, (p, c), paraml = TUtils.unwrap_class_type ht in
  let class_ = Decl_env.get_class_dep env.Env.decl_env c in
  match class_ with
  | None ->
      (* The class lives in PHP land *)
      ()
  | Some class_ ->
      let size1 = List.length class_.dc_tparams in
      let size2 = List.length paraml in
      if size1 <> size2 then Errors.class_arity p class_.dc_pos c size1;
      let subst = Inst.make_subst class_.dc_tparams paraml in
      iter2_shortest begin fun t ty ->
        let ty_pos = Reason.to_pos (fst ty) in
        List.iter t.tp_constraints begin fun (ck, cstr) ->
          (* Constraint might contain uses of generic type parameters *)
          let cstr = Inst.instantiate subst cstr in
          match ck with
          | Ast.Constraint_as ->
            Type.sub_type_decl ty_pos Reason.URnone env ty cstr
          | Ast.Constraint_eq ->
            (* This code could well be unreachable, because we don't allow
             * equality constraints on class generics. *)
            Type.sub_type_decl ty_pos Reason.URnone env ty cstr;
            Type.sub_type_decl ty_pos Reason.URnone env cstr ty
          | Ast.Constraint_super ->
            Type.sub_type_decl ty_pos Reason.URnone env cstr ty
          | Ast.Constraint_pu_from ->
            failwith "Typing.check_implements_tparaml: \
                      implement typing for 'from' constraints"
        end
      end class_.dc_tparams paraml

(* In order to type-check a class, we need to know what "parent"
 * refers to. Sometimes people write "parent::", when that happens,
 * we need to know the type of parent.
 *)
and class_def_parent env class_def class_type =
  match class_def.c_extends with
  | (_, Happly ((_, x), _) as parent_ty) :: _ ->
      let parent_type = Decl_env.get_class_dep env.Env.decl_env x in
      (match parent_type with
      | Some parent_type -> check_parent class_def class_type parent_type
      | None -> ());
      let parent_ty = Decl_hint.hint env.Env.decl_env parent_ty in
      env, Some x, parent_ty
  (* The only case where we have more than one parent class is when
   * dealing with interfaces and interfaces cannot use parent.
   *)
  | _ :: _
  | _ -> env, None, (Reason.Rnone, Typing_utils.tany env)

and check_parent class_def class_type parent_type =
  let position = fst class_def.c_name in
  if (Cls.const class_type) && not parent_type.dc_const
  then Errors.self_const_parent_not position;
  if parent_type.dc_const && not (Cls.const class_type)
  then Errors.parent_const_self_not position;
  (* Are all the parents in Hack? Do we know all their methods?
   * If so, let's check that the abstract methods have been implemented.
   *)
  if (Cls.members_fully_known class_type)
  then check_parent_abstract position parent_type class_type;
  if parent_type.dc_final
  then Errors.extend_final position parent_type.dc_pos parent_type.dc_name
  else ()

and check_parent_sealed child_type parent_type =
  match parent_type.dc_sealed_whitelist with
    | None -> ()
    | Some whitelist ->
      let parent_pos = parent_type.dc_pos in
      let parent_name = parent_type.dc_name in
      let child_pos = (Cls.pos child_type) in
      let child_name = (Cls.name child_type) in
      let check kind action =
        if not (SSet.mem child_name whitelist)
        then Errors.extend_sealed child_pos parent_pos parent_name kind action in
      begin match parent_type.dc_kind, (Cls.kind child_type) with
        | Ast.Cinterface, Ast.Cinterface -> check "interface" "extend"
        | Ast.Cinterface, _ -> check "interface" "implement"
        | Ast.Ctrait, _ -> check "trait" "use"
        | Ast.Cabstract, _
        | Ast.Cnormal, _ -> check "class" "extend"
        | Ast.Cenum, _ -> ()
        | Ast.Crecord, _ -> ()
     end

and check_parents_sealed env child_def child_type =
  let parents = child_def.c_extends @ child_def.c_implements @ child_def.c_uses in
  List.iter parents begin function
    | _, Happly ((_, name), _) ->
      begin match Decl_env.get_class_dep env.Env.decl_env name with
        | Some parent_type -> check_parent_sealed child_type parent_type
        | None -> ()
      end
    | _ -> ()
  end

and check_parent_abstract position parent_type class_type =
  let is_final = (Cls.final class_type) in
  if parent_type.dc_kind = Ast.Cabstract &&
    ((Cls.kind class_type) <> Ast.Cabstract || is_final)
  then begin
    check_extend_abstract_meth ~is_final position (Cls.methods class_type);
    check_extend_abstract_meth ~is_final position (Cls.smethods class_type);
    check_extend_abstract_const ~is_final position (Cls.consts class_type);
    check_extend_abstract_typeconst
      ~is_final position (Cls.typeconsts class_type);
  end else ()

and shallow_decl_enabled () =
  TCO.shallow_class_decl (GlobalNamingOptions.get ())

and class_def tcopt c =
  let env = EnvFromDef.class_env tcopt c in
  let tc = Env.get_class env (snd c.c_name) in
  add_decl_errors (Option.(map tc (fun tc -> value_exn (Cls.decl_errors tc))));
  let c = TNBody.class_meth_bodies c in
  NastCheck.class_ env c;
  NastInitCheck.class_ env c;
  match tc with
  | None ->
      (* This can happen if there was an error during the declaration
       * of the class. *)
      None
  | Some tc ->
    Typing_requirements.check_class env tc;
    if shallow_decl_enabled () then
      Typing_inheritance.check_class env tc;
    Some (class_def_ env c tc)

and class_def_ env c tc =
  let env =
    let kind = match c.c_kind with
      | Ast.Cenum -> SN.AttributeKinds.enum
      | _ -> SN.AttributeKinds.cls in
    Typing_attributes.check_def env new_object kind c.c_user_attributes in
  let env =
    { env with Env.inside_ppl_class =
        Attributes.mem SN.UserAttributes.uaProbabilisticModel c.c_user_attributes
    } in
  let pc, _ = c.c_name in
  let impl = List.map
    (c.c_extends @ c.c_implements @ c.c_uses)
    (Decl_hint.hint env.Env.decl_env) in
  let env, constraints =
    Phase.localize_generic_parameters_with_bounds env c.c_tparams.c_tparam_list
      ~ety_env:(Phase.env_with_self env) in
  let env = add_constraints (fst c.c_name) env constraints in
  Typing_variance.class_ (Env.get_tcopt env) (snd c.c_name) tc impl;
  List.iter impl (check_implements_tparaml env);
  check_parents_sealed env c tc;

  let env, parent_id, parent = class_def_parent env c tc in
  let is_final = (Cls.final tc) in
  if ((Cls.kind tc) = Ast.Cnormal || is_final) && (Cls.members_fully_known tc)
  then begin
    check_extend_abstract_meth ~is_final pc (Cls.methods tc);
    check_extend_abstract_meth ~is_final pc (Cls.smethods tc);
    check_extend_abstract_const ~is_final pc (Cls.consts tc);
    check_extend_abstract_typeconst ~is_final pc (Cls.typeconsts tc);
  end;
  let env = Env.set_parent env parent in
  let env = match parent_id with
    | None -> env
    | Some parent_id -> Env.set_parent_id env parent_id in
  if (Cls.final tc) then begin
    match c.c_kind with
    | Ast.Cinterface -> Errors.interface_final (fst c.c_name)
    | Ast.Cabstract -> ()
    | Ast.Ctrait -> Errors.trait_final (fst c.c_name)
    | Ast.Cenum
    | Ast.Crecord ->
      Errors.internal_error pc ("The parser should not parse final on" ^
        (if c.c_kind = Ast.Cenum then "enums" else "records"))
    | Ast.Cnormal -> ()
  end;
  List.iter c.c_static_vars ~f:begin fun {cv_id=(p,id); _} ->
    check_static_class_element (Cls.get_prop tc) ~elt_type:`Property id p
  end;
  List.iter c.c_vars ~f:begin fun {cv_id=(p,id); _} ->
    check_dynamic_class_element (Cls.get_sprop tc) ~elt_type:`Property id p
  end;
  List.iter c.c_static_methods ~f:begin fun {m_name=(p,id); _} ->
    check_static_class_element (Cls.get_method tc) ~elt_type:`Method id p
  end;
  List.iter c.c_methods ~f:begin fun {m_name=(p,id); _} ->
    check_dynamic_class_element (Cls.get_smethod tc) ~elt_type:`Method id p
  end;
  (* get a map of method names to list of traits from which they were removed *)
  let alist = List.map c.c_method_redeclarations ~f:(fun m ->
    let _, name = m.mt_method in
    let _, trait, _ = Decl_utils.unwrap_class_hint m.mt_trait in
    name, trait) in
  let removals = String.Map.of_alist_fold alist ~init:[] ~f:(Fn.flip List.cons) in
  List.iter impl (class_implements_type env c removals);
  let env = List.fold c.c_method_redeclarations ~init:env ~f:(supertype_redeclared_method tc) in
  if (Cls.is_disposable tc)
    then List.iter (c.c_extends @ c.c_uses) (Typing_disposable.enforce_is_disposable env);
  let typed_vars = List.map c.c_vars (class_var_def env ~is_static:false c) in
  let typed_method_redeclarations = [] in
  let typed_methods = List.filter_map c.c_methods (method_def env) in
  let typed_typeconsts = List.map c.c_typeconsts (typeconst_def env) in
  let typed_consts, const_types =
    List.unzip (List.map c.c_consts (class_const_def env)) in
  let env = Typing_enum.enum_class_check env tc c.c_consts const_types in
  let typed_constructor = class_constr_def env c in
  let env = Env.set_static env in
  let typed_static_vars =
    List.map c.c_static_vars (class_var_def env ~is_static:true c) in
  let typed_static_methods = List.filter_map c.c_static_methods (method_def env) in
  let filename = Pos.filename (fst c.c_name) in
  let droot = env.Env.decl_env.Decl_env.droot in
  let file_attrs =
    file_attributes (Env.get_tcopt env) filename c.c_mode droot c.c_file_attributes in
  {
    T.c_span = c.c_span;
    T.c_annotation = Env.save env.Env.lenv.Env.tpenv env;
    T.c_mode = c.c_mode;
    T.c_final = c.c_final;
    T.c_is_xhp = c.c_is_xhp;
    T.c_kind = c.c_kind;
    T.c_name = c.c_name;
    T.c_tparams = class_type_param env c.c_tparams;
    T.c_extends = c.c_extends;
    T.c_uses = c.c_uses;
    T.c_method_redeclarations = typed_method_redeclarations;
    T.c_xhp_attr_uses = c.c_xhp_attr_uses;
    T.c_xhp_category = c.c_xhp_category;
    T.c_req_extends = c.c_req_extends;
    T.c_req_implements = c.c_req_implements;
    T.c_implements = c.c_implements;
    T.c_consts = typed_consts;
    T.c_typeconsts = typed_typeconsts;
    T.c_static_vars = typed_static_vars;
    T.c_vars = typed_vars;
    T.c_constructor = typed_constructor;
    T.c_static_methods = typed_static_methods;
    T.c_methods = typed_methods;
    T.c_file_attributes = file_attrs;
    T.c_user_attributes = List.map c.c_user_attributes (user_attribute env);
    T.c_namespace = c.c_namespace;
    T.c_enum = c.c_enum;
    T.c_doc_comment = c.c_doc_comment;
    T.c_attributes = [];
    T.c_xhp_children = c.c_xhp_children;
    T.c_xhp_attrs = [];
    T.c_pu_enums = []; (* TODO PU (typing) *)
  }

and check_dynamic_class_element get_static_elt element_name dyn_pos ~elt_type =
  (* The non-static properties that we get passed do not start with '$', but the
     static properties we want to look up do, so add it. *)
  let id =
    match elt_type with
    | `Method -> element_name
    | `Property -> "$"^element_name
  in
  match get_static_elt id with
  | None -> ()
  | Some static_element ->
    let lazy (static_element_reason, _) = static_element.ce_type in
    Errors.static_redeclared_as_dynamic
      dyn_pos
      (Reason.to_pos static_element_reason)
      element_name
      ~elt_type

and check_static_class_element get_dyn_elt element_name static_pos ~elt_type =
  (* The static properties that we get passed in start with '$', but the
     non-static properties we're matching against don't, so we need to detect
     that and remove it if present. *)
  let element_name = String_utils.lstrip element_name "$" in
  match get_dyn_elt element_name with
  | None -> ()
  | Some dyn_element ->
    let lazy (dyn_element_reason, _) = dyn_element.ce_type in
    Errors.dynamic_redeclared_as_static
      static_pos
      (Reason.to_pos dyn_element_reason)
      element_name
      ~elt_type

and check_extend_abstract_meth ~is_final p seq =
  Sequence.iter seq begin fun (x, ce) ->
    match ce.ce_type with
    | lazy (r, Tfun { ft_abstract = true; _ }) ->
        Errors.implement_abstract ~is_final p (Reason.to_pos r) "method" x
    | _ -> ()
  end

(* Type constants must be bound to a concrete type for non-abstract classes.
 *)
and check_extend_abstract_typeconst ~is_final p seq =
  Sequence.iter seq begin fun (x, tc) ->
    if tc.ttc_type = None then
      Errors.implement_abstract ~is_final p (fst tc.ttc_name) "type constant" x
  end

and check_extend_abstract_const ~is_final p seq =
  Sequence.iter seq begin fun (x, cc) ->
    if cc.cc_abstract && not cc.cc_synthesized then
      let cc_pos = Reason.to_pos (fst cc.cc_type) in
      Errors.implement_abstract ~is_final p cc_pos "constant" x
  end

and typeconst_abstract_kind = function
  | Nast.TCAbstract default -> T.TCAbstract default
  | Nast.TCPartiallyAbstract -> T.TCPartiallyAbstract
  | Nast.TCConcrete -> T.TCConcrete

and typeconst_def env {
  c_tconst_abstract;
  c_tconst_name = (pos, _) as id;
  c_tconst_constraint;
  c_tconst_type;
  c_tconst_user_attributes;
} =
  let env, cstr = opt Phase.localize_hint_with_self env c_tconst_constraint in
  let env, ty = opt Phase.localize_hint_with_self env c_tconst_type in
  ignore (
    Option.map2 ty cstr ~f:(Type.sub_type pos Reason.URtypeconst_cstr env)
  );
  let env = Typing_attributes.check_def env new_object
    SN.AttributeKinds.typeconst c_tconst_user_attributes in
  {
    T.c_tconst_abstract = typeconst_abstract_kind c_tconst_abstract;
    T.c_tconst_name = id;
    T.c_tconst_constraint = c_tconst_constraint;
    T.c_tconst_type = c_tconst_type;
    T.c_tconst_user_attributes = List.map c_tconst_user_attributes (user_attribute env);
  }

and class_const_def env (h, id, e) =
  let env, ty, opt_expected =
    match h with
    | None ->
      let env, ty = Env.fresh_type env (fst id) in
      env, ty, None
    | Some h ->
      let env, ty = Phase.localize_hint_with_self env h in
      env, ty, Some (fst id, Reason.URhint, ty)
  in
  match e with
    | Some e ->
      let env, te, ty' = expr ?expected:opt_expected env e in
      ignore (Type.coerce_type (fst id) Reason.URhint env ty' ty);
      (h, id, Some te), ty'
    | None ->
      (h, id, None), ty

and class_constr_def env c =
  let env = { env with Env.inside_constructor = true } in
  match Option.map c.c_constructor (method_def env) with
  | Some (Some c) -> Some c
  | Some None | None -> None

and class_implements_type env c1 removals ctype2 =
  let params =
    List.map c1.c_tparams.c_tparam_list begin fun { tp_name = (p, s); _ } ->
      (Reason.Rwitness p, Tgeneric s)
    end in
  let r = Reason.Rwitness (fst c1.c_name) in
  let ctype1 = r, Tapply (c1.c_name, params) in
  Typing_extends.check_implements env removals ctype2 ctype1;
  ()

(* Type-check a property declaration, with optional initializer *)
and class_var_def env ~is_static c cv =
  (* First pick up and localize the hint if it exists *)
  let env, expected =
    match cv.cv_type with
    | None ->
      env, None
    | Some (p, _ as cty) ->
      let env =
        (* If this is an XHP attribute and we're in strict mode,
           relax to partial mode to allow the use of the "array"
           annotation without specifying type parameters. Until
           recently HHVM did not allow "array" with type parameters
           in XHP attribute declarations, so this is a temporary
           hack to support existing code for now. *)
        (* Task #5815945: Get rid of this Hack *)
        if cv.cv_is_xhp && (Env.is_strict env)
          then Env.set_mode env FileInfo.Mpartial
          else env in
      let cty = Decl_hint.hint env.Env.decl_env cty in
      let env, cty = Phase.localize_with_self env cty in
      env, Some (p, Reason.URhint, cty) in
  (* Next check the expression, passing in expected type if present *)
  let env, typed_cv_expr, ty =
    match cv.cv_expr with
    | None ->
      let env, ty = Env.fresh_type env (fst cv.cv_id) in
      env, None, ty
    | Some e ->
      let env, te, ty = expr ?expected env e in
      (* Check that the inferred type is a subtype of the expected type.
       * Eventually this will be the responsibility of `expr`
       *)
      let env =
        match expected with
        | None -> env
        | Some (p, ur, cty) -> Type.coerce_type p ur env ty cty in
      env, Some te, ty in
  let env =
    if is_static
    then Typing_attributes.check_def env new_object
      SN.AttributeKinds.staticProperty cv.cv_user_attributes
    else Typing_attributes.check_def env new_object
      SN.AttributeKinds.instProperty cv.cv_user_attributes in
  begin
    if Option.is_none cv.cv_type
    then begin
      if Env.is_strict env
      then Errors.add_a_typehint (fst cv.cv_id)
      else
        let pos, name = cv.cv_id in
        let name = if is_static then "$" ^ name else name in
        let var_type = Reason.Rwitness pos, Typing_utils.tany env in
        if Option.is_none cv.cv_expr
        then Typing_suggest.uninitialized_member (snd c.c_name) name env var_type ty
        else Typing_suggest.save_member name env var_type ty
      end;
    {
      T.cv_final = cv.cv_final;
      T.cv_is_xhp = cv.cv_is_xhp;
      T.cv_visibility = cv.cv_visibility;
      T.cv_type = cv.cv_type;
      T.cv_id = cv.cv_id;
      T.cv_expr = typed_cv_expr;
      T.cv_user_attributes = List.map cv.cv_user_attributes (user_attribute env);
      T.cv_is_promoted_variadic = cv.cv_is_promoted_variadic;
      T.cv_doc_comment = cv.cv_doc_comment; (* Can make None to save space *)
    }
  end

and add_constraints p env constraints =
  let add_constraint env (ty1, ck, ty2) =
    SubType.add_constraint p env ck ty1 ty2 in
  List.fold_left constraints ~f:add_constraint ~init: env

and supertype_redeclared_method tc env m =
  let pos, name = m.mt_name in
  let get_method = if m.mt_static then Env.get_static_member else Env.get_member in

  let class_member_opt = get_method true env tc name in
  let _, trait, _ = Decl_utils.unwrap_class_hint m.mt_trait in
  let _, trait_method = m.mt_method in
  let open Option in
  let trait_member_opt = Env.get_class env trait >>= (fun trait_tc ->
    get_method true env trait_tc trait_method) in

  ignore (map2 trait_member_opt class_member_opt ~f:begin fun trait_member class_member ->
    match trait_member.ce_type, class_member.ce_type with
    | lazy (r_child, Tfun ft_child), lazy (r_parent, Tfun ft_parent) ->
      Errors.try_
      (fun () ->
        ignore (Typing_subtype.(subtype_method
          ~check_return:true
          ~extra_info:{ method_info = None; class_ty = None; parent_class_ty = None }
          env
          r_child
          ft_child
          r_parent
          ft_parent
        ))
      ) (fun errorl ->
        Errors.try_
        (fun () -> Errors.bad_method_override pos name errorl)
        (fun ierrorl ->
          Errors.bad_decl_override
          (Reason.to_pos r_parent)
          trait
          (Cls.pos tc)
          (Cls.name tc)
          ierrorl)
      )
    | _ -> ()
  end); env

and file_attributes tcopt file mode droot file_attrs =
  let env = Env.empty tcopt file ~droot in
  let env = Env.set_mode env mode in
  let uas = List.concat_map ~f:(fun fa -> fa.fa_user_attributes) file_attrs in
  let env =
    Typing_attributes.check_def env new_object SN.AttributeKinds.file uas in
  List.map
    ~f:(fun fa ->
      { T.fa_user_attributes = List.map ~f:(user_attribute env) fa.fa_user_attributes;
        T.fa_namespace = fa.fa_namespace;
      })
    file_attrs

and user_attribute env ua =
  let typed_ua_params =
    List.map ua.ua_params (fun e -> let _env, te, _ty = expr env e in te) in
  {
    T.ua_name = ua.ua_name;
    T.ua_params = typed_ua_params;
  }

and reify_kind = function
  | Nast.Erased -> T.Erased
  | Nast.SoftReified -> T.SoftReified
  | Nast.Reified -> T.Reified


and type_param env t =
  let env = Typing_attributes.check_def env new_object
    SN.AttributeKinds.typeparam t.tp_user_attributes in
  {
    T.tp_variance = t.tp_variance;
    T.tp_name = t.tp_name;
    T.tp_constraints = t.tp_constraints;
    T.tp_reified = reify_kind t.tp_reified;
    T.tp_user_attributes = List.map t.tp_user_attributes (user_attribute env);
  }

and class_type_param env ct =
  {
    T.c_tparam_list = List.map ~f:(type_param env) ct.c_tparam_list;
    T.c_tparam_constraints = SMap.map (Tuple.T2.map_fst ~f:reify_kind) ct.c_tparam_constraints;
  }

and method_def env m =
  with_timeout (Env.get_tcopt env) m.m_name ~do_:begin fun () ->
  (* reset the expression dependent display ids for each method body *)
  Reason.expr_display_id_map := IMap.empty;
  let pos = fst m.m_name in
  let env = Env.reinitialize_locals env in
  let env = Env.set_env_function_pos env pos in
  let env = Typing_attributes.check_def env new_object
    SN.AttributeKinds.mthd m.m_user_attributes in
  let reactive = fun_reactivity env.Env.decl_env m.m_user_attributes m.m_params in
  let mut =
    match TUtils.fun_mutable m.m_user_attributes with
    | None ->
      (* <<__Mutable>> is implicit on constructors  *)
      if snd m.m_name = SN.Members.__construct
      then Some Param_borrowed_mutable
      else None
    | x -> x in
  let env = Env.set_env_reactive env reactive in
  let env = Env.set_fun_mutable env mut in
  let ety_env =
    { (Phase.env_with_self env) with from_class = Some CIstatic; } in
  let env, constraints =
    Phase.localize_generic_parameters_with_bounds env m.m_tparams
    ~ety_env:ety_env in
  let env = add_constraints pos env constraints in
  let env =
    Phase.localize_where_constraints ~ety_env env m.m_where_constraints in
  let env =
    if Env.is_static env then env
    else Env.set_local env this (Env.get_self env) in
  let env =
    match Env.get_class env (Env.get_self_id env) with
    | None -> env
    | Some c ->
      (* Mark $this as a using variable if it has a disposable type *)
      if (Cls.is_disposable c)
      then Env.set_using_var env this
      else env in
  let env = Env.clear_params env in
  let env, ty = match m.m_ret with
    | None ->
      env, Typing_return.make_default_return env m.m_name
    | Some ret ->
      let ret = Decl_hint.hint env.Env.decl_env ret in
      (* If a 'this' type appears it needs to be compatible with the
       * late static type
       *)
      let ety_env =
        { (Phase.env_with_self env) with
          from_class = Some CIstatic } in
      Phase.localize ~ety_env env ret in
  let return = Typing_return.make_info m.m_fun_kind m.m_user_attributes env
    ~is_explicit:(Option.is_some m.m_ret) ty in
  let env, param_tys =
    List.map_env env m.m_params make_param_local_ty in
  if Env.is_strict env then begin
    List.iter2_exn ~f:(check_param env) m.m_params param_tys;
  end;
  Typing_memoize.check_method env m;
  let env, typed_params =
    List.map_env env (List.zip_exn param_tys m.m_params) bind_param in
  let env, t_variadic = match m.m_variadic with
    | FVvariadicArg vparam ->
      let env, ty = make_param_local_ty env vparam in
      if Env.is_strict env then
        check_param env vparam ty;
      let env, t_variadic = bind_param env (ty, vparam) in
      env, (T.FVvariadicArg t_variadic)
    | FVellipsis p -> env, T.FVellipsis p
    | FVnonVariadic -> env, T.FVnonVariadic in
  let nb = Nast.assert_named_body m.m_body in
  let local_tpenv = env.Env.lenv.Env.tpenv in
  let env, tb =
    fun_ ~abstract:m.m_abstract env return pos nb m.m_fun_kind in
  let env = SubType.solve_all_unsolved_tyvars env in
  Typing_subtype.log_prop env;
  let env = Env.check_todo env in
  (* restore original method reactivity  *)
  let env = Env.set_env_reactive env reactive in
  let m_ret =
    match m.m_ret with
    | None when
         snd m.m_name = SN.Members.__destruct
      || snd m.m_name = SN.Members.__construct ->
      Some (pos, Happly((pos, "void"), []))
    | None when Env.is_strict env ->
      Typing_return.suggest_return env pos return.Typing_env_return_info.return_type; None
    | None -> let (pos, id) = m.m_name in
              let id = (Env.get_self_id env) ^ "::" ^ id in
              Typing_suggest.save_fun_or_method (pos, id);
              m.m_ret
    | Some hint ->
      Typing_return.async_suggest_return (m.m_fun_kind) hint (fst m.m_name); m.m_ret in
  let m = { m with m_ret = m_ret; } in
  let annotation =
    if Nast.named_body_is_unsafe nb
    then Tast.Annotations.FuncBodyAnnotation.HasUnsafeBlocks
    else Tast.Annotations.FuncBodyAnnotation.NoUnsafeBlocks in
  let method_def = {
    T.m_annotation = Env.save local_tpenv env;
    T.m_span = m.m_span;
    T.m_final = m.m_final;
    T.m_static = m.m_static;
    T.m_abstract = m.m_abstract;
    T.m_visibility = m.m_visibility;
    T.m_name = m.m_name;
    T.m_tparams = List.map m.m_tparams (type_param env);
    T.m_where_constraints = m.m_where_constraints;
    T.m_variadic = t_variadic;
    T.m_params = typed_params;
    T.m_fun_kind = m.m_fun_kind;
    T.m_user_attributes = List.map m.m_user_attributes (user_attribute env);
    T.m_ret = m.m_ret;
    T.m_body = { T.fb_ast = tb; fb_annotation = annotation };
    T.m_external = m.m_external;
    T.m_doc_comment = m.m_doc_comment;
  } in
  Typing_lambda_ambiguous.suggest_method_def env method_def
  end (* with_timeout *)

and typedef_def tcopt typedef  =
  let env = EnvFromDef.typedef_env tcopt typedef in
  let tdecl = Env.get_typedef env (snd typedef.t_name) in
  add_decl_errors (Option.(map tdecl (fun tdecl -> value_exn tdecl.td_decl_errors)));
  let env, constraints =
    Phase.localize_generic_parameters_with_bounds env typedef.t_tparams
              ~ety_env:(Phase.env_with_self env) in
  let env = add_constraints (fst typedef.t_name) env constraints in
  NastCheck.typedef env typedef;
  let {
    t_annotation = ();
    t_name = t_pos, _;
    t_tparams = _;
    t_constraint = tcstr;
    t_kind = hint;
    t_user_attributes = _;
    t_vis = _;
    t_mode = _;
    t_namespace = _;
  } = typedef in
  let ty = Decl_hint.hint env.Env.decl_env hint in
  let env, ty = Phase.localize_with_self env ty in
  let env = begin match tcstr with
    | Some tcstr ->
      let cstr = Decl_hint.hint env.Env.decl_env tcstr in
      let env, cstr = Phase.localize_with_self env cstr in
      Typing_ops.sub_type t_pos Reason.URnewtype_cstr env ty cstr
    | _ -> env
  end in
  let env = begin match hint with
    | pos, Hshape { nsi_allows_unknown_fields=_; nsi_field_map } ->
      let get_name sfi = sfi.sfi_name in
      check_shape_keys_validity env pos (List.map ~f:get_name nsi_field_map)
    | _ -> env
  end in
  let env = Typing_attributes.check_def env new_object
    SN.AttributeKinds.typealias typedef.t_user_attributes in
  {
    T.t_annotation = Env.save env.Env.lenv.Env.tpenv env;
    T.t_name = typedef.t_name;
    T.t_mode = typedef.t_mode;
    T.t_vis = typedef.t_vis;
    T.t_user_attributes = List.map typedef.t_user_attributes (user_attribute env);
    T.t_constraint = typedef.t_constraint;
    T.t_kind = typedef.t_kind;
    T.t_tparams = List.map typedef.t_tparams (type_param env);
    T.t_namespace = typedef.t_namespace;
  }

and gconst_def tcopt cst =
  let env = EnvFromDef.gconst_env tcopt cst in
  add_decl_errors (Option.map (Env.get_gconst env (snd cst.cst_name)) ~f:snd);

  let typed_cst_value, env =
    match cst.cst_value with
    | None -> None, env
    | Some value ->
      match cst.cst_type with
      | Some hint ->
        let ty = Decl_hint.hint env.Env.decl_env hint in
        let env, dty = Phase.localize_with_self env ty in
        let env, te, value_type =
          expr ~expected:(fst hint, Reason.URhint, dty) env value in
        let env = Typing_utils.sub_type env value_type dty in
        Some te, env
      | None ->
        let env, te, _value_type = expr env value in
        Some te, env
  in
  { T.cst_annotation = Env.save env.Env.lenv.Env.tpenv env;
    T.cst_mode = cst.cst_mode;
    T.cst_name = cst.cst_name;
    T.cst_type = cst.cst_type;
    T.cst_value = typed_cst_value;
    T.cst_namespace = cst.cst_namespace;
    T.cst_span = cst.cst_span;
  }

(* Calls the method of a class, but allows the f callback to override the
 * return value type *)
and overload_function make_call fpos p env (cpos, class_id) method_id el uel f =
  let env, tcid, ty = static_class_id ~check_constraints:false cpos env [] class_id in
  let env, _tel, _ = exprs ~is_func_arg:true env el in
  let env, fty, _ =
    class_get ~is_method:true ~is_const:false env ty method_id class_id in
  (* call the function as declared to validate arity and input types,
     but ignore the result and overwrite with custom one *)
  let (env, tel, tuel, res), has_error = Errors.try_with_error
    (* TODO: Should we be passing hints here *)
    (fun () -> (call ~expected:None p env fty el uel), false)
    (fun () -> (env, [], [], (Reason.Rwitness p, Typing_utils.tany env)), true) in
  (* if there are errors already stop here - going forward would
   * report them twice *)
  if has_error
  then env, T.make_typed_expr p res T.Any, res
  else
    let env, ty = f env fty res el in
    let fty =
      match fty with
      | r, Tfun ft -> r, Tfun {ft with ft_ret = ty}
      | _ -> fty in
    let te = T.make_typed_expr fpos fty (T.Class_const(tcid, method_id)) in
    make_call env te [] tel tuel ty

and update_array_type ?lhs_of_null_coalesce p env e1 e2 valkind  =
  let type_mapper =
    Typing_arrays.update_array_type p ~is_map:(Option.is_some e2) in
  match valkind with
    | `lvalue | `lvalue_subexpr ->
      let env, te1, ty1 =
        raw_expr ~valkind:`lvalue_subexpr ~check_defined:true env e1 in
      let env, ty1 = type_mapper env ty1 in
      begin match e1 with
        | (_, Lvar (_, x)) ->
          (* type_mapper has updated the type in ty1 typevars, but we
             need to update the local variable type too *)
          let env, ty1 = set_valid_rvalue p env x ty1 in
          env, te1, ty1
        | _ -> env, te1, ty1
      end
    | _ ->
      raw_expr ?lhs_of_null_coalesce env e1

let nast_to_tast opts nast =
  let convert_def = function
    | Nast.Fun f ->
      begin match fun_def opts f with
      | Some f -> T.Fun f
      | None -> failwith @@ Printf.sprintf
          "Error when typechecking function: %s" (snd f.f_name)
      end
    | Nast.Constant gc -> T.Constant (gconst_def opts gc)
    | Nast.Typedef td  -> T.Typedef (typedef_def opts td)
    | Nast.Class c -> begin
      match class_def opts c with
      | Some c -> (T.Class c)
      | None -> failwith @@ Printf.sprintf
          "Error in declaration of class: %s" (snd c.c_name)
    end
    (* We don't typecheck top level statements:
     * https://docs.hhvm.com/hack/unsupported/top-level
     * so just create the minimal env for us to construct a Stmt.
     *)
    | Nast.Stmt s ->
    let env = Env.empty opts Relative_path.default None in
      T.Stmt (snd (stmt env s))
    | Nast.Namespace _
    | Nast.NamespaceUse _
    | Nast.SetNamespaceEnv _
    | Nast.FileAttributes _ ->
      failwith "Invalid nodes in NAST. These nodes should be removed during naming."
  in
  Nast_check.program nast;
  let tast = List.map nast convert_def in
  Tast_check.program tast;
  tast

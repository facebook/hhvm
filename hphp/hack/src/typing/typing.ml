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
open Tast
open Typing_defs
open Typing_env_types
open Utils
module TFTerm = Typing_func_terminality
module TUtils = Typing_utils
module Reason = Typing_reason
module Inst = Decl_instantiate
module Type = Typing_ops
module Env = Typing_env
module LEnv = Typing_lenv
module Async = Typing_async
module SubType = Typing_subtype
module Union = Typing_union
module Inter = Typing_intersection
module SN = Naming_special_names
module TVis = Typing_visibility
module TNBody = Typing_naming_body
module Phase = Typing_phase
module TOG = Typing_object_get
module Subst = Decl_subst
module ExprDepTy = Typing_dependent_type.ExprDepTy
module TCO = TypecheckerOptions
module IM = TCO.InferMissing
module EnvFromDef = Typing_env_from_def
module C = Typing_continuations
module CMap = C.Map
module Try = Typing_try
module TR = Typing_reactivity
module FL = FeatureLogging
module MakeType = Typing_make_type
module Decl_provider = Decl_provider_ctx
module Cls = Decl_provider.Class
module Partial = Partial_provider
module Fake = Typing_fake_members

exception InvalidPocketUniverse

module ExpectedTy : sig
  [@@@warning "-32"]

  type t = private {
    pos: Pos.t;
    reason: Typing_reason.ureason;
    ty: locl_possibly_enforced_ty;
  }
  [@@deriving show]

  [@@@warning "+32"]

  val make : Pos.t -> Typing_reason.ureason -> locl_ty -> t

  (* We will allow coercion to this expected type, if et_enforced=true *)
  val make_and_allow_coercion :
    Pos.t -> Typing_reason.ureason -> locl_possibly_enforced_ty -> t
end = struct
  (* Some mutually recursive inference functions in typing.ml pass around an ~expected argument that
   * enables bidirectional type checking. This module abstracts away that type so that it can be
   * extended and modified without having to touch every consumer. *)
  type t = {
    pos: Pos.t;
    reason: Typing_reason.ureason;
    ty: locl_possibly_enforced_ty;
        [@printer Pp_type.pp_possibly_enforced_ty Pp_type.pp_locl]
  }
  [@@deriving show]

  let make_and_allow_coercion pos reason ty = { pos; reason; ty }

  let make pos reason locl_ty =
    make_and_allow_coercion pos reason (MakeType.unenforced locl_ty)
end

let map_funcbody_annotation an =
  match an with
  | Nast.NamedWithUnsafeBlocks -> Tast.HasUnsafeBlocks
  | Nast.Named -> Tast.NoUnsafeBlocks
  | Nast.Unnamed _ -> failwith "Should not map over unnamed body"

(*****************************************************************************)
(* Debugging *)
(*****************************************************************************)

(* A guess as to the last position we were typechecking, for use in debugging,
 * such as figuring out what a runaway hh_server thread is doing. Updated
 * only best-effort -- it's an approximation to point debugging in the right
 * direction, nothing more. *)
let debug_last_pos = ref Pos.none

let debug_print_last_pos _ =
  print_endline (Pos.string (Pos.to_absolute !debug_last_pos))

(****************************************************************************)
(* Hooks *)
(****************************************************************************)

let expr_hook = ref None

let with_expr_hook hook f =
  with_context
    ~enter:(fun () -> expr_hook := Some hook)
    ~exit:(fun () -> expr_hook := None)
    ~do_:f

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let err_witness env p = (Reason.Rwitness p, Typing_utils.terr env)

(* Set all the types in an expression to the given type. *)
let with_type ty env (e : Nast.expr) : Tast.expr =
  let visitor =
    object
      inherit [_] Aast.map

      method on_'ex _ p = (p, ty)

      method on_'fb _ _ = Tast.HasUnsafeBlocks

      method on_'en _ _ = env

      method on_'hi _ _ = ty
    end
  in
  visitor#on_expr () e

let expr_error env (r : Reason.t) (e : Nast.expr) =
  let ty = (r, Typing_utils.terr env) in
  (env, with_type ty Tast.dummy_saved_env e, ty)

let expr_any env r e =
  let ty = (r, Typing_utils.tany env) in
  (env, with_type ty Tast.dummy_saved_env e, ty)

let compare_field_kinds x y =
  match (x, y) with
  | (AFvalue (p1, _), AFkvalue ((p2, _), _))
  | (AFkvalue ((p2, _), _), AFvalue (p1, _)) ->
    Errors.field_kinds p1 p2;
    false
  | _ -> true

let check_consistent_fields x l = List.for_all l (compare_field_kinds x)

let unbound_name env (pos, name) e =
  let strictish = Partial.should_check_error (Env.get_mode env) 4107 in
  match Env.get_mode env with
  | FileInfo.Mstrict
  | FileInfo.Mexperimental ->
    Errors.unbound_name_typing pos name;
    expr_error env (Reason.Rwitness pos) e
  | FileInfo.Mpartial when strictish ->
    Errors.unbound_name_typing pos name;
    expr_error env (Reason.Rwitness pos) e
  | FileInfo.Mdecl
  | FileInfo.Mpartial
  | FileInfo.Mphp ->
    expr_any env (Reason.Rwitness pos) e

(* Is this type Traversable<vty> or Container<vty> for some vty? *)
let get_value_collection_inst ty =
  match ty with
  | (_, Tclass ((_, c), _, [vty]))
    when String.equal c SN.Collections.cTraversable
         || String.equal c SN.Collections.cContainer ->
    Some vty
  (* If we're expecting a mixed or a nonnull then we can just assume
   * that the element type is mixed *)
  | (_, Tnonnull) -> Some (MakeType.mixed Reason.Rnone)
  | (_, Tany _) -> Some ty
  | _ -> None

(* Is this type KeyedTraversable<kty,vty>
 *           or KeyedContainer<kty,vty>
 * for some kty, vty?
 *)
let get_key_value_collection_inst p ty =
  match ty with
  | (_, Tclass ((_, c), _, [kty; vty]))
    when String.equal c SN.Collections.cKeyedTraversable
         || String.equal c SN.Collections.cKeyedContainer ->
    Some (kty, vty)
  (* If we're expecting a mixed or a nonnull then we can just assume
   * that the key type is arraykey and the value type is mixed *)
  | (_, Tnonnull) ->
    let arraykey = MakeType.arraykey (Reason.Rkey_value_collection_key p) in
    let mixed = MakeType.mixed Reason.Rnone in
    Some (arraykey, mixed)
  | (_, Tany _) -> Some (ty, ty)
  | _ -> None

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
    when String.equal c (Nast.vc_kind_to_name vc_kind) ->
    Some vty
  | _ -> get_value_collection_inst ty

(* Is this type array<vty> or a supertype for some vty? *)
let get_akvarray_inst ty =
  match ty with
  | (_, Tarraykind (AKvarray vty)) -> Some vty
  | _ -> get_value_collection_inst ty

(* Is this type array<kty,vty> or a supertype for some kty and vty? *)
let get_akdarray_inst p ty =
  match ty with
  | (_, Tarraykind (AKdarray (kty, vty))) -> Some (kty, vty)
  | _ -> get_key_value_collection_inst p ty

(* Is this type one of the three key-value collection types
 * e.g. dict<kty,vty> or a supertype for some kty and vty? *)
let get_kvc_inst p kvc_kind ty =
  match ty with
  | (_, Tclass ((_, c), _, [kty; vty]))
    when String.equal c (Nast.kvc_kind_to_name kvc_kind) ->
    Some (kty, vty)
  | _ -> get_key_value_collection_inst p ty

(* Is this type darray<kty, vty> or a supertype for some kty and vty? *)
let get_darray_inst p ty =
  match ty with
  (* It's darray<kty, vty> *)
  | (_, Tarraykind (AKdarray (kty, vty))) -> Some (kty, vty)
  | _ -> get_key_value_collection_inst p ty

let with_timeout env fun_name ~(do_ : env -> 'b) : 'b option =
  let timeout = (Env.get_tcopt env).GlobalOptions.tco_timeout in
  if Int.equal timeout 0 then
    Some (do_ env)
  else
    let big_envs = ref [] in
    let env = { env with big_envs } in
    Timeout.with_timeout
      ~timeout
      ~on_timeout:(fun _ ->
        List.iter !big_envs (fun (p, env) ->
            Typing_log.log_key "WARN: environment is too big.";
            Typing_log.hh_show_env p env);
        Errors.typechecker_timeout fun_name timeout;
        None)
      ~do_:(fun _ -> Some (do_ env))

(*****************************************************************************)
(* Handling function/method arguments *)
(*****************************************************************************)
let param_has_attribute param attr =
  List.exists param.param_user_attributes (fun { ua_name; _ } ->
      String.equal attr (snd ua_name))

let has_accept_disposable_attribute param =
  param_has_attribute param SN.UserAttributes.uaAcceptDisposable

let get_param_mutability param =
  if param_has_attribute param SN.UserAttributes.uaMutable then
    Some Param_borrowed_mutable
  else if param_has_attribute param SN.UserAttributes.uaMaybeMutable then
    Some Param_maybe_mutable
  else if param_has_attribute param SN.UserAttributes.uaOwnedMutable then
    Some Param_owned_mutable
  else
    None

(* Check whether this is a function type that (a) either returns a disposable
 * or (b) has the <<__ReturnDisposable>> attribute
 *)
let is_return_disposable_fun_type env ty =
  match Env.expand_type env ty with
  | (_env, (_, Tfun ft)) ->
    ft.ft_return_disposable
    || Option.is_some
         (Typing_disposable.is_disposable_type env ft.ft_ret.et_type)
  | _ -> false

let enforce_param_not_disposable env param ty =
  if has_accept_disposable_attribute param then
    ()
  else
    let p = param.param_pos in
    match Typing_disposable.is_disposable_type env ty with
    | Some class_name -> Errors.invalid_disposable_hint p (strip_ns class_name)
    | None -> ()

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

let merge_hint_with_decl_hint env type_hint decl_hint =
  match (type_hint, decl_hint) with
  | (None, Some hint) -> Some hint
  | (Some hint, _) -> Some (Decl_hint.hint env.decl_env hint)
  | (None, None) -> None

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
    | None ->
      if IM.can_infer_params @@ TCO.infer_missing (Env.get_tcopt env) then
        Env.fresh_type_reason ~variance:Ast_defs.Contravariant env r
      else
        (env, (r, TUtils.tany env))
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
    match ty with
    | (_, t) when param.param_is_variadic ->
      (* when checking the body of a function with a variadic
       * argument, "f(C ...$args)", $args is a varray<C> *)
      let r = Reason.Rvar_param param.param_pos in
      let arr_values = (r, t) in
      (r, Tarraykind (AKvarray arr_values))
    | x -> x
  in
  Typing_reactivity.disallow_atmost_rx_as_rxfunc_on_non_functions env param ty;
  (env, ty)

(* Return a map describing all the fields in this record, including
   inherited fields, and whether they have a default value. *)
let all_record_fields (env : env) (rd : Decl_provider.record_def_decl) :
    (sid * Typing_defs.record_field_req) SMap.t =
  let record_fields rd =
    List.fold
      rd.rdt_fields
      ~init:SMap.empty
      ~f:(fun acc (((_, name), _) as f) -> SMap.add name f acc)
  in
  let rec loop rd fields decls_seen =
    match rd.rdt_extends with
    | Some (_, parent_name) when SSet.mem parent_name decls_seen ->
      (* Inheritance loop, so we've seen all the records. *)
      fields
    | Some (_, parent_name) ->
      (match Decl_provider.get_record_def (Env.get_ctx env) parent_name with
      | Some rd ->
        loop
          rd
          (SMap.union fields (record_fields rd))
          (SSet.add (snd rd.rdt_name) decls_seen)
      | None -> fields)
    | None -> fields
  in
  loop rd (record_fields rd) (SSet.singleton (snd rd.rdt_name))

(* Given a localized parameter type and parameter information, infer
 * a type for the parameter default expression (if present) and check that
 * it is a subtype of the parameter type (if present). If no parameter type
 * is specified, then union with Tany. (So it's as though we did a conditional
 * assignment of the default expression to the parameter).
 * Set the type of the parameter in the locals environment *)
let rec bind_param env (ty1, param) =
  let (env, param_te, ty1) =
    match param.param_expr with
    | None -> (env, None, ty1)
    | Some e ->
      let decl_hint =
        Option.map
          ~f:(Decl_hint.hint env.decl_env)
          (hint_of_type_hint param.param_type_hint)
      in
      let enforced =
        match decl_hint with
        | None -> false
        | Some ty -> Typing_enforceability.is_enforceable env ty
      in
      let ty1_enforced = { et_type = ty1; et_enforced = enforced } in
      let expected =
        ExpectedTy.make_and_allow_coercion
          param.param_pos
          Reason.URparam
          ty1_enforced
      in
      let (env, te, ty2) = expr ~expected env e in
      Typing_sequencing.sequence_check_expr e;
      let (env, ty1) =
        if
          Option.is_none (hint_of_type_hint param.param_type_hint)
          && not
             @@ IM.can_infer_params
             @@ TCO.infer_missing (Env.get_tcopt env)
          (* ty1 will be Tany iff we have no type hint and we are not in
           * 'infer missing mode'. When it ty1 is Tany we just union it with
           * the type of the default expression *)
        then
          Union.union env ty1 ty2
        (* Otherwise we have an explicit type, and the default expression type
         * must be a subtype *)
        else
          let env =
            Typing_coercion.coerce_type
              param.param_pos
              Reason.URhint
              env
              ty2
              ty1_enforced
              Errors.parameter_default_value_wrong_type
          in
          (env, ty1)
      in
      (env, Some te, ty1)
  in
  let (env, user_attributes) =
    List.map_env env param.param_user_attributes user_attribute
  in
  let tparam =
    {
      Aast.param_annotation = Tast.make_expr_annotation param.param_pos ty1;
      Aast.param_type_hint = (ty1, hint_of_type_hint param.param_type_hint);
      Aast.param_is_reference = param.param_is_reference;
      Aast.param_is_variadic = param.param_is_variadic;
      Aast.param_pos = param.param_pos;
      Aast.param_name = param.param_name;
      Aast.param_expr = param_te;
      Aast.param_callconv = param.param_callconv;
      Aast.param_user_attributes = user_attributes;
      Aast.param_visibility = param.param_visibility;
    }
  in
  let mode = get_param_mode param.param_is_reference param.param_callconv in
  let id = Local_id.make_unscoped param.param_name in
  let env = Env.set_local env id ty1 in
  let env = Env.set_param env id (ty1, mode) in
  let env =
    if has_accept_disposable_attribute param then
      Env.set_using_var env id
    else
      env
  in
  let env =
    match get_param_mutability param with
    | Some Param_borrowed_mutable ->
      Env.add_mutable_var
        env
        id
        (param.param_pos, Typing_mutability_env.Borrowed)
    | Some Param_owned_mutable ->
      Env.add_mutable_var env id (param.param_pos, Typing_mutability_env.Mutable)
    | Some Param_maybe_mutable ->
      Env.add_mutable_var
        env
        id
        (param.param_pos, Typing_mutability_env.MaybeMutable)
    | None ->
      Env.add_mutable_var
        env
        id
        (param.param_pos, Typing_mutability_env.Immutable)
  in
  (env, tparam)

(* In strict mode, we force you to give a type declaration on a parameter *)
(* But the type checker is nice: it makes a suggestion :-) *)
and check_param env param ty is_code_error =
  let env =
    if is_code_error 4231 then
      Typing_attributes.check_def
        env
        new_object
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

and check_inout_return env =
  let params = Local_id.Map.elements (Env.get_params env) in
  List.fold params ~init:env ~f:(fun env (id, ((r, ty), mode)) ->
      match mode with
      | FPinout ->
        (* Whenever the function exits normally, we require that each local
         * corresponding to an inout parameter be compatible with the original
         * type for the parameter (under subtyping rules). *)
        let local_ty = Env.get_local env id in
        let (env, ety) = Env.expand_type env local_ty in
        let pos = Reason.to_pos (fst ety) in
        let param_ty = (Reason.Rinout_param (Reason.to_pos r), ty) in
        Type.sub_type
          pos
          Reason.URassign_inout
          env
          ety
          param_ty
          Errors.inout_return_type_mismatch
      | _ -> env)

and add_decl_errors = function
  | None -> ()
  | Some errors -> Errors.merge_into_current errors

and get_callable_variadicity
    ?(is_function = false) ~partial_callback ~pos env variadicity_decl_ty =
  function
  | FVvariadicArg vparam ->
    let (env, ty) = make_param_local_ty env variadicity_decl_ty vparam in
    check_param env vparam ty partial_callback;
    let (env, t_variadic) = bind_param env (ty, vparam) in
    (env, Aast.FVvariadicArg t_variadic)
  | FVellipsis p ->
    if is_function && Partial.should_check_error (Env.get_mode env) 4223 then
      Errors.ellipsis_strict_mode ~require:`Type_and_param_name pos;
    (env, Aast.FVellipsis p)
  | FVnonVariadic -> (env, Aast.FVnonVariadic)

(*****************************************************************************)
(* Now we are actually checking stuff! *)
(*****************************************************************************)
and fun_def tcopt f :
    (Tast.fun_def * Typing_env_types.global_tvenv_with_pos) option =
  let env = EnvFromDef.fun_env tcopt f in
  with_timeout env f.f_name ~do_:(fun env ->
      (* reset the expression dependent display ids for each function body *)
      Reason.expr_display_id_map := IMap.empty;
      let pos = fst f.f_name in
      let decl_header = get_decl_function_header env (snd f.f_name) in
      let nb = TNBody.func_body f in
      add_decl_errors
        (Option.map
           (Env.get_fun env (snd f.f_name))
           ~f:(fun x -> Option.value_exn x.fe_decl_errors));
      let env = Env.open_tyvars env (fst f.f_name) in
      let env = Env.set_env_function_pos env pos in
      let env = Env.set_env_pessimize env in
      let env =
        Typing_attributes.check_def
          env
          new_object
          SN.AttributeKinds.fn
          f.f_user_attributes
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
      let (decl_ty, params_decl_ty, variadicity_decl_ty) =
        merge_decl_header_with_hints
          ~params:f.f_params
          ~ret:f.f_ret
          ~variadic:f.f_variadic
          decl_header
          env
      in
      let (env, locl_ty) =
        match decl_ty with
        | None ->
          Typing_return.make_default_return
            ~is_method:false
            ~is_infer_missing_on:
              (IM.can_infer_return @@ TCO.infer_missing (Env.get_tcopt env))
            env
            f.f_name
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
          locl_ty
          decl_ty
      in
      let (env, param_tys) =
        List.zip_exn f.f_params params_decl_ty
        |> List.map_env env ~f:(fun env (param, hint) ->
               make_param_local_ty env hint param)
      in
      let partial_callback = Partial.should_check_error (Env.get_mode env) in
      let param_fn p t = check_param env p t partial_callback in
      List.iter2_exn ~f:param_fn f.f_params param_tys;
      Typing_memoize.check_function env f;
      let (env, typed_params) =
        List.map_env env (List.zip_exn param_tys f.f_params) bind_param
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
      let env = set_tyvars_variance_in_callable env locl_ty param_tys in
      let local_tpenv = Env.get_tpenv env in
      let disable =
        Naming_attributes.mem
          SN.UserAttributes.uaDisableTypecheckerInternal
          f.f_user_attributes
      in
      let (env, tb) = fun_ ~disable env return pos nb f.f_fun_kind in
      (* restore original reactivity *)
      let env = Env.set_env_reactive env reactive in
      begin
        match hint_of_type_hint f.f_ret with
        | None ->
          if partial_callback 4030 then Errors.expecting_return_type_hint pos
        | Some hint -> Typing_return.async_suggest_return f.f_fun_kind hint pos
      end;
      let (env, file_attrs) = file_attributes env f.f_file_attributes in
      let (env, tparams) = List.map_env env f.f_tparams type_param in
      let (env, user_attributes) =
        List.map_env env f.f_user_attributes user_attribute
      in
      let env =
        Typing_solver.close_tyvars_and_solve env Errors.bad_function_typevar
      in
      let env =
        Typing_solver.solve_all_unsolved_tyvars env Errors.bad_function_typevar
      in
      let env = Typing_solver.expand_bounds_of_global_tyvars env in
      let fundef =
        {
          Aast.f_annotation = Env.save local_tpenv env;
          Aast.f_span = f.f_span;
          Aast.f_mode = f.f_mode;
          Aast.f_ret = (locl_ty, hint_of_type_hint f.f_ret);
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
      ( Typing_lambda_ambiguous.suggest_fun_def env fundef,
        (pos, env.global_tvenv) ))

(*****************************************************************************)
(* function used to type closures, functions and methods *)
(*****************************************************************************)
and fun_ ?(abstract = false) ?(disable = false) env return pos named_body f_kind
    =
  Env.with_env env (fun env ->
      debug_last_pos := pos;
      let env = Env.set_return env return in
      let (env, tb) =
        if disable then
          let () =
            Errors.internal_error
              pos
              ( "Type inference for this function has been disabled by the "
              ^ SN.UserAttributes.uaDisableTypecheckerInternal
              ^ " attribute" )
          in
          block env []
        else
          block env named_body.fb_ast
      in
      Typing_sequencing.sequence_check_block named_body.fb_ast;
      let { Typing_env_return_info.return_type = ret; _ } =
        Env.get_return env
      in
      let env =
        if
          (not @@ LEnv.has_next env)
          || abstract
          || Nast.named_body_is_unsafe named_body
        then
          env
        else
          fun_implicit_return env pos ret.et_type f_kind
      in
      debug_last_pos := Pos.none;
      (env, tb))

and fun_implicit_return env pos ret = function
  | Ast_defs.FGenerator
  | Ast_defs.FAsyncGenerator ->
    env
  | Ast_defs.FCoroutine
  | Ast_defs.FSync ->
    (* A function without a terminal block has an implicit return; the
     * "void" type *)
    let env = check_inout_return env in
    let r = Reason.Rno_return pos in
    let rty = MakeType.void r in
    Typing_return.implicit_return env pos ~expected:ret ~actual:rty
  | Ast_defs.FAsync ->
    (* An async function without a terminal block has an implicit return;
     * the Awaitable<void> type *)
    let r = Reason.Rno_return_async pos in
    let rty = MakeType.awaitable r (MakeType.void r) in
    Typing_return.implicit_return env pos ~expected:ret ~actual:rty

and block env stl = List.map_env env stl ~f:stmt

(* Set a local; must not be already assigned if it is a using variable *)
and set_local ?(is_using_clause = false) env (pos, x) ty =
  if Env.is_using_var env x then
    if is_using_clause then
      Errors.duplicate_using_var pos
    else
      Errors.illegal_disposable pos "assigned";
  let env = Env.set_local env x ty in
  if is_using_clause then
    Env.set_using_var env x
  else
    env

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
  | Binop (Ast_defs.Eq None, (lvar_pos, Lvar lvar), e) ->
    let (env, te, ty) = expr ~is_using_clause:true env e in
    let env =
      Typing_disposable.enforce_is_disposable_type env has_await (fst e) ty
    in
    let env = set_local ~is_using_clause:true env lvar ty in
    (* We are assigning a new value to the local variable, so we need to
     * generate a new expression id
     *)
    let env = Env.set_local_expr_id env (snd lvar) (Ident.tmp ()) in
    ( env,
      ( Tast.make_typed_expr
          pos
          ty
          (Aast.Binop
             ( Ast_defs.Eq None,
               Tast.make_typed_expr lvar_pos ty (Aast.Lvar lvar),
               te )),
        [snd lvar] ) )
  (* Arbitrary expression. This will be assigned to a temporary *)
  | _ ->
    let (env, typed_using_clause, ty) =
      expr ~is_using_clause:true env using_clause
    in
    let env =
      Typing_disposable.enforce_is_disposable_type env has_await pos ty
    in
    (env, (typed_using_clause, []))

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
    let (env, pairs) =
      List.map_env env using_clauses (check_using_expr has_await)
    in
    let (typed_using_clauses, vars_list) = List.unzip pairs in
    let ty_ = Ttuple (List.map typed_using_clauses Tast.get_type) in
    let ty = (Reason.Rnone, ty_) in
    ( env,
      Tast.make_typed_expr pos ty (Aast.Expr_list typed_using_clauses),
      List.concat vars_list )
  | _ ->
    let (env, (typed_using_clause, vars)) =
      check_using_expr has_await env using_clause
    in
    (env, typed_using_clause, vars)

(* Require a new construct with disposable *)
and enforce_return_disposable _env e =
  match e with
  | (_, New _) -> ()
  | (_, Call _) -> ()
  | (_, Await (_, Call _)) -> ()
  | (p, _) -> Errors.invalid_return_disposable p

(* Wrappers around the function with the same name in Typing_lenv, which only
 * performs the move/save and merge operation if we are in a try block or in a
 * function with return type 'noreturn'.
 * This enables significant perf improvement, because this is called at every
 * function of method call, when most calls are outside of a try block. *)
and move_and_merge_next_in_catch env =
  if env.in_try || TFTerm.is_noreturn env then
    LEnv.move_and_merge_next_in_cont env C.Catch
  else
    LEnv.drop_cont env C.Next

and save_and_merge_next_in_catch env =
  if env.in_try || TFTerm.is_noreturn env then
    LEnv.save_and_merge_next_in_cont env C.Catch
  else
    env

and might_throw env = save_and_merge_next_in_catch env

and stmt env (pos, st) =
  let (env, st) = stmt_ env pos st in
  Typing_debug.log_env_if_too_big pos env;
  (env, (pos, st))

and stmt_ env pos st =
  (* Type check a loop. f env = (env, result) checks the body of the loop.
   * We iterate over the loop until the "next" continuation environment is
   * stable. alias_depth is supposed to be an upper bound on this; but in
   * certain cases this fails (e.g. a generic type grows unboundedly). TODO:
   * fix this.
   *)
  let infer_loop env f =
    let in_loop_outer = env.in_loop in
    let alias_depth =
      if in_loop_outer then
        1
      else
        Typing_alias.get_depth (pos, st)
    in
    let env = { env with in_loop = true } in
    let rec loop env n =
      (* Remember the old environment *)
      let old_next_entry = Env.next_cont_opt env in
      let (env, result) = f env in
      let new_next_entry = Env.next_cont_opt env in
      (* Finish if we reach the bound, or if the environments match *)
      if
        Int.equal n alias_depth
        || Typing_per_cont_ops.is_sub_opt_entry
             Typing_subtype.is_sub_type
             env
             new_next_entry
             old_next_entry
      then
        let env = { env with in_loop = in_loop_outer } in
        (env, result)
      else
        loop env (n + 1)
    in
    loop env 1
  in
  let env = Env.open_tyvars env pos in
  (fun (env, tb) ->
    (Typing_solver.close_tyvars_and_solve env Errors.unify_error, tb))
  @@
  match st with
  | Fallthrough ->
    let env =
      if env.in_case then
        LEnv.move_and_merge_next_in_cont env C.Fallthrough
      else
        env
    in
    (env, Aast.Fallthrough)
  | Goto (_, label) ->
    let env = LEnv.move_and_merge_next_in_cont env (C.Goto label) in
    (env, Aast.Noop)
  | GotoLabel (_, label) ->
    let env = LEnv.update_next_from_conts env [C.Next; C.Goto label] in
    (env, Aast.Noop)
  | Noop -> (env, Aast.Noop)
  | Expr e ->
    let (env, te, _) = expr env e in
    let env =
      if TFTerm.typed_expression_exits te then
        LEnv.move_and_merge_next_in_cont env C.Exit
      else
        env
    in
    (env, Aast.Expr te)
  | If (e, b1, b2) ->
    let (env, te, _) = expr env e in
    let (env, tb1, tb2) =
      branch
        env
        (fun env ->
          let env = condition env true te in
          block env b1)
        (fun env ->
          let env = condition env false te in
          block env b2)
    in
    (* TODO TAST: annotate with joined types *)
    (env, Aast.If (te, tb1, tb2))
  | Return None ->
    let env = check_inout_return env in
    let rty = MakeType.void (Reason.Rwitness pos) in
    let { Typing_env_return_info.return_type = expected_return; _ } =
      Env.get_return env
    in
    let expected_return =
      Typing_return.strip_awaitable (Env.get_fn_kind env) env expected_return
    in
    let env =
      match Env.get_fn_kind env with
      | Ast_defs.FGenerator
      | Ast_defs.FAsyncGenerator ->
        env
      | _ ->
        Typing_return.implicit_return
          env
          pos
          ~expected:expected_return.et_type
          ~actual:rty
    in
    let env = LEnv.move_and_merge_next_in_cont env C.Exit in
    (env, Aast.Return None)
  | Return (Some e) ->
    let env = check_inout_return env in
    let expr_pos = fst e in
    let Typing_env_return_info.
          {
            return_type;
            return_disposable;
            return_mutable;
            return_explicit;
            return_void_to_rx;
          } =
      Env.get_return env
    in
    let return_type =
      Typing_return.strip_awaitable (Env.get_fn_kind env) env return_type
    in
    let expected =
      if return_explicit then
        Some
          (ExpectedTy.make_and_allow_coercion
             expr_pos
             Reason.URreturn
             return_type)
      else
        None
    in
    if return_disposable then enforce_return_disposable env e;
    let (env, te, rty) =
      expr ~is_using_clause:return_disposable ?expected env e
    in
    let env =
      if not (equal_reactivity (env_reactivity env) Nonreactive) then
        Typing_mutability.handle_value_in_return
          ~function_returns_mutable:return_mutable
          ~function_returns_void_for_rx:return_void_to_rx
          env
          env.function_pos
          te
      else
        env
    in
    let return_type =
      {
        return_type with
        et_type = TR.strip_condition_type_in_return env return_type.et_type;
      }
    in
    (* This is a unify_error rather than a return_type_mismatch because the return
     * statement is the problem, not the return type itself. *)
    let env =
      Typing_coercion.coerce_type
        expr_pos
        Reason.URreturn
        env
        rty
        return_type
        Errors.unify_error
    in
    let env = LEnv.move_and_merge_next_in_cont env C.Exit in
    (env, Aast.Return (Some te))
  | Do (b, e) ->
    (* NOTE: leaks scope as currently implemented; this matches
       the behavior in naming (cf. `do_stmt` in naming/naming.ml).
     *)
    let (env, (tb, te)) =
      LEnv.stash_and_do env [C.Continue; C.Break; C.Do] (fun env ->
          let env = LEnv.save_and_merge_next_in_cont env C.Do in
          let (env, _) = block env b in
          (* saving the locals in continue here even if there is no continue
           * statement because they must be merged at the end of the loop, in
           * case there is no iteration *)
          let env = LEnv.save_and_merge_next_in_cont env C.Continue in
          let (env, tb) =
            infer_loop env (fun env ->
                let env =
                  LEnv.update_next_from_conts env [C.Continue; C.Next]
                in
                (* The following is necessary in case there is an assignment in the
                 * expression *)
                let (env, te, _) = expr env e in
                let env = condition env true te in
                let env = LEnv.update_next_from_conts env [C.Do; C.Next] in
                let (env, tb) = block env b in
                (env, tb))
          in
          let env = LEnv.update_next_from_conts env [C.Continue; C.Next] in
          let (env, te, _) = expr env e in
          let env = condition env false te in
          let env = LEnv.update_next_from_conts env [C.Break; C.Next] in
          (env, (tb, te)))
    in
    (env, Aast.Do (tb, te))
  | While (e, b) ->
    let (env, (te, tb)) =
      LEnv.stash_and_do env [C.Continue; C.Break] (fun env ->
          let env = LEnv.save_and_merge_next_in_cont env C.Continue in
          let (env, tb) =
            infer_loop env (fun env ->
                let env =
                  LEnv.update_next_from_conts env [C.Continue; C.Next]
                in
                (* The following is necessary in case there is an assignment in the
                 * expression *)
                let (env, te, _) = expr env e in
                let env = condition env true te in
                (* TODO TAST: avoid repeated generation of block *)
                let (env, tb) = block env b in
                (env, tb))
          in
          let env = LEnv.update_next_from_conts env [C.Continue; C.Next] in
          let (env, te, _) = expr env e in
          let env = condition env false te in
          let env = LEnv.update_next_from_conts env [C.Break; C.Next] in
          (env, (te, tb)))
    in
    (env, Aast.While (te, tb))
  | Using
      {
        us_has_await = has_await;
        us_expr = using_clause;
        us_block = using_block;
        us_is_block_scoped;
      } ->
    let (env, typed_using_clause, using_vars) =
      check_using_clause env has_await using_clause
    in
    let (env, typed_using_block) = block env using_block in
    (* Remove any using variables from the environment, as they should not
     * be in scope outside the block *)
    let env = List.fold_left using_vars ~init:env ~f:Env.unset_local in
    ( env,
      Aast.Using
        Aast.
          {
            us_has_await = has_await;
            us_expr = typed_using_clause;
            us_block = typed_using_block;
            us_is_block_scoped;
          } )
  | For (e1, e2, e3, b) ->
    let (env, (te1, te2, te3, tb)) =
      LEnv.stash_and_do env [C.Continue; C.Break] (fun env ->
          (* For loops leak their initalizer, but nothing that's defined in the
           body
         *)
          let (env, te1, _) = expr env e1 in
          (* initializer *)
          let env = LEnv.save_and_merge_next_in_cont env C.Continue in
          let (env, (tb, te3)) =
            infer_loop env (fun env ->
                (* The following is necessary in case there is an assignment in the
                 * expression *)
                let (env, te2, _) = expr env e2 in
                let env = condition env true te2 in
                let (env, tb) = block env b in
                let env =
                  LEnv.update_next_from_conts env [C.Continue; C.Next]
                in
                let (env, te3, _) = expr env e3 in
                (env, (tb, te3)))
          in
          let env = LEnv.update_next_from_conts env [C.Continue; C.Next] in
          let (env, te2, _) = expr env e2 in
          let env = condition env false te2 in
          let env = LEnv.update_next_from_conts env [C.Break; C.Next] in
          (env, (te1, te2, te3, tb)))
    in
    (env, Aast.For (te1, te2, te3, tb))
  | Switch (((pos, _) as e), cl) ->
    let (env, te, ty) = expr env e in
    (* NB: A 'continue' inside a 'switch' block is equivalent to a 'break'.
     * See the note in
     * http://php.net/manual/en/control-structures.continue.php *)
    let (env, (te, tcl)) =
      LEnv.stash_and_do env [C.Continue; C.Break] (fun env ->
          let parent_locals = LEnv.get_all_locals env in
          let case_list env = case_list parent_locals ty env pos cl in
          let (env, tcl) = Env.in_case env case_list in
          let env =
            LEnv.update_next_from_conts env [C.Continue; C.Break; C.Next]
          in
          (env, (te, tcl)))
    in
    (env, Aast.Switch (te, tcl))
  | Foreach (e1, e2, b) ->
    (* It's safe to do foreach over a disposable, as no leaking is possible *)
    let (env, te1, ty1) = expr ~accept_using_var:true env e1 in
    let (env, (te1, te2, tb)) =
      LEnv.stash_and_do env [C.Continue; C.Break] (fun env ->
          let env = LEnv.save_and_merge_next_in_cont env C.Continue in
          let (env, tk, tv) = as_expr env ty1 (fst e1) e2 in
          let (env, (te2, tb)) =
            infer_loop env (fun env ->
                let env =
                  LEnv.update_next_from_conts env [C.Continue; C.Next]
                in
                let (env, te2) = bind_as_expr env (fst e1) tk tv e2 in
                let (env, tb) = block env b in
                (env, (te2, tb)))
          in
          let env =
            LEnv.update_next_from_conts env [C.Continue; C.Break; C.Next]
          in
          (env, (te1, te2, tb)))
    in
    (env, Aast.Foreach (te1, te2, tb))
  | Try (tb, cl, fb) ->
    let (env, ttb, tcl, tfb) = try_catch env tb cl fb in
    (env, Aast.Try (ttb, tcl, tfb))
  | Def_inline _ ->
    (* Do nothing, this doesn't occur in Hack code. *)
    failwith "Should never typecheck nested definitions"
  | Awaitall (el, b) ->
    let env = might_throw env in
    let (env, el) =
      List.fold_left el ~init:(env, []) ~f:(fun (env, tel) (e1, e2) ->
          let (env, te2, ty2) = expr env e2 in
          let (env, ty2) =
            Async.overload_extract_from_awaitable env (fst e2) ty2
          in
          match e1 with
          | Some e1 ->
            let (env, _, _) = assign (fst e1) env (fst e1, Lvar e1) ty2 in
            (env, (Some e1, te2) :: tel)
          | None -> (env, (None, te2) :: tel))
    in
    let (env, b) = block env b in
    (env, Aast.Awaitall (el, b))
  | Throw e ->
    let p = fst e in
    let (env, te, ty) = expr env e in
    let env = exception_ty p env ty in
    let env = move_and_merge_next_in_catch env in
    (env, Aast.Throw te)
  | Continue ->
    let env = LEnv.move_and_merge_next_in_cont env C.Continue in
    (env, Aast.Continue)
  | Break ->
    let env = LEnv.move_and_merge_next_in_cont env C.Break in
    (env, Aast.Break)
  | Block _
  | Markup _ ->
    failwith
      "Unexpected nodes in AST. These nodes should have been removed in naming."

and branch :
    type res. env -> (env -> env * res) -> (env -> env * res) -> env * res * res
    =
 fun env branch1 branch2 ->
  let parent_lenv = env.lenv in
  let (env, tbr1) = branch1 env in
  let lenv1 = env.lenv in
  let env = { env with lenv = parent_lenv } in
  let (env, tbr2) = branch2 env in
  let lenv2 = env.lenv in
  let env = LEnv.union_lenvs env parent_lenv lenv1 lenv2 in
  (env, tbr1, tbr2)

and finally_cont fb env ctx =
  let env = LEnv.replace_cont env C.Next (Some ctx) in
  let (env, _tfb) = block env fb in
  (env, LEnv.get_all_locals env)

and finally env fb =
  match fb with
  | [] ->
    let env = LEnv.update_next_from_conts env [C.Next; C.Finally] in
    (env, [])
  | _ ->
    let parent_locals = LEnv.get_all_locals env in
    (* First typecheck the finally block against all continuations merged
    * together.
    * During this phase, record errors found in the finally block, but discard
    * the resulting environment. *)
    let all_conts = Env.all_continuations env in
    let env = LEnv.update_next_from_conts env all_conts in
    let (env, tfb) = block env fb in
    let env = LEnv.restore_conts_from env parent_locals all_conts in
    (* Second, typecheck the finally block once against each continuation. This
    * helps be more clever about what each continuation will be after the
    * finally block.
    * We don't want to record errors during this phase, because certain types
    * of errors will fire wrongly. For example, if $x is nullable in some
    * continuations but not in others, then we must use `?->` on $x, but an
    * error will fire when typechecking the finally block againts continuations
    * where $x is non-null.  *)
    let finally_cont env _key = finally_cont fb env in
    let (env, locals_map) =
      Errors.ignore_ (fun () -> CMap.map_env finally_cont env parent_locals)
    in
    let (env, locals) = Try.finally_merge env locals_map in
    (Env.env_with_locals env locals, tfb)

and try_catch env tb cl fb =
  let parent_locals = LEnv.get_all_locals env in
  let env =
    LEnv.drop_conts env [C.Break; C.Continue; C.Exit; C.Catch; C.Finally]
  in
  let (env, (ttb, tcb)) =
    Env.in_try env (fun env ->
        let (env, ttb) = block env tb in
        let env = LEnv.move_and_merge_next_in_cont env C.Finally in
        let catchctx = LEnv.get_cont_option env C.Catch in
        let (env, lenvtcblist) = List.map_env env ~f:(catch catchctx) cl in
        let (lenvl, tcb) = List.unzip lenvtcblist in
        let env = LEnv.union_lenv_list env env.lenv lenvl in
        let env = LEnv.move_and_merge_next_in_cont env C.Finally in
        (env, (ttb, tcb)))
  in
  let (env, tfb) = finally env fb in
  let env = LEnv.drop_cont env C.Finally in
  let env =
    LEnv.restore_and_merge_conts_from
      env
      parent_locals
      [C.Break; C.Continue; C.Exit; C.Catch; C.Finally]
  in
  (env, ttb, tcb, tfb)

and case_list parent_locals ty env switch_pos cl =
  let initialize_next_cont env =
    let env = LEnv.restore_conts_from env parent_locals [C.Next] in
    let env = LEnv.update_next_from_conts env [C.Next; C.Fallthrough] in
    LEnv.drop_cont env C.Fallthrough
  in
  let check_fallthrough env switch_pos case_pos block rest_of_list ~is_default =
    if not @@ List.is_empty block then
      match rest_of_list with
      | []
      | [Default (_, [])] ->
        ()
      | _ ->
        begin
          match LEnv.get_cont_option env C.Next with
          | Some _ ->
            if is_default then
              Errors.default_fallthrough switch_pos
            else
              Errors.case_fallthrough switch_pos case_pos
          | None -> ()
        end
    (* match *)
    (* match *)
    else
      ()
  in
  let make_exhaustive_equivalent_case_list env cl =
    let has_default =
      List.exists cl ~f:(function
          | Default _ -> true
          | _ -> false)
    in
    let (env, ty) =
      (* If it hasn't got a default clause then we need to solve type variables
       * in order to check for an enum *)
      if has_default then
        Env.expand_type env ty
      else
        Typing_solver.expand_type_and_solve
          env
          ~description_of_expected:"a value"
          switch_pos
          ty
          Errors.unify_error
    in
    let rec is_enum env ty =
      let (env, ty) = Env.expand_type env ty in
      let is_enum =
        match snd ty with
        | Tunion [ty; ty'] ->
          let (env, ty_is_enum) = is_enum env ty in
          let (env, ty'_is_enum) = is_enum env ty' in
          let is_sub_dyn env ty =
            SubType.is_sub_type_for_union env ty (MakeType.dynamic Reason.Rnone)
          in
          (ty_is_enum && is_sub_dyn env ty')
          || (is_sub_dyn env ty && ty'_is_enum)
        | Tnewtype (cid, _, _) -> Env.is_enum env cid
        | _ -> false
      in
      (env, is_enum)
    in
    let (env, is_enum) = is_enum env ty in
    (* If there is no default case and this is not a switch on enum (since
     * exhaustiveness is garanteed elsewhere on enums),
     * then add a default case for control flow correctness
     *)
    if has_default || is_enum then
      (env, cl, false)
    else
      (env, cl @ [Default (Pos.none, [])], true)
  in
  let rec case_list env = function
    | [] -> (env, [])
    | Default (pos, b) :: rl ->
      let env = initialize_next_cont env in
      let (env, tb) = block env b in
      check_fallthrough env switch_pos pos b rl ~is_default:true;
      let (env, tcl) = case_list env rl in
      (env, Aast.Default (pos, tb) :: tcl)
    | Case (((pos, _) as e), b) :: rl ->
      let env = initialize_next_cont env in
      let (env, te, _) = expr env e in
      let (env, tb) = block env b in
      check_fallthrough env switch_pos pos b rl ~is_default:false;
      let (env, tcl) = case_list env rl in
      (env, Aast.Case (te, tb) :: tcl)
  in
  let (env, cl, added_empty_default) =
    make_exhaustive_equivalent_case_list env cl
  in
  let (env, tcl) = case_list env cl in
  let tcl =
    if added_empty_default then
      List.take tcl (List.length tcl - 1)
    else
      tcl
  in
  (env, tcl)

and catch catchctx env (sid, exn, b) =
  let env = LEnv.replace_cont env C.Next catchctx in
  let cid = CI sid in
  let ety_p = fst sid in
  let (env, _, _, _) = instantiable_cid ety_p env cid [] in
  let (env, _tal, _te, ety) =
    static_class_id ~check_constraints:false ety_p env [] cid
  in
  let env = exception_ty ety_p env ety in
  let env = set_local env exn ety in
  let (env, tb) = block env b in
  (env, (env.lenv, (sid, exn, tb)))

and as_expr env ty1 pe e =
  let env = Env.open_tyvars env pe in
  (fun (env, expected_ty, tk, tv) ->
    let rec distribute_union env ty =
      let (env, ty) = Env.expand_type env ty in
      match ty with
      | (_, Tunion tyl) -> List.fold tyl ~init:env ~f:distribute_union
      | _ ->
        if SubType.is_sub_type_for_union env ty (MakeType.dynamic Reason.Rnone)
        then
          let env = SubType.sub_type env ty tk Errors.unify_error in
          let env = SubType.sub_type env ty tv Errors.unify_error in
          env
        else
          let ur = Reason.URforeach in
          Type.sub_type pe ur env ty expected_ty Errors.unify_error
    in
    let env = distribute_union env ty1 in
    let env = Env.set_tyvar_variance env expected_ty in
    (Typing_solver.close_tyvars_and_solve env Errors.unify_error, tk, tv))
  @@
  let (env, tv) = Env.fresh_type env pe in
  match e with
  | As_v _ ->
    let tk = MakeType.mixed Reason.Rnone in
    (env, MakeType.traversable (Reason.Rforeach pe) tv, tk, tv)
  | As_kv _ ->
    let (env, tk) = Env.fresh_type env pe in
    (env, MakeType.keyed_traversable (Reason.Rforeach pe) tk tv, tk, tv)
  | Await_as_v _ ->
    let tk = MakeType.mixed Reason.Rnone in
    (env, MakeType.async_iterator (Reason.Rasyncforeach pe) tv, tk, tv)
  | Await_as_kv _ ->
    let (env, tk) = Env.fresh_type env pe in
    (env, MakeType.async_keyed_iterator (Reason.Rasyncforeach pe) tk tv, tk, tv)

and bind_as_expr env p ty1 ty2 aexpr =
  let check_reassigned_mutable env te =
    if Env.env_local_reactive env then
      Typing_mutability.handle_assignment_mutability env te None
    else
      env
  in
  match aexpr with
  | As_v ev ->
    let (env, te, _) = assign p env ev ty2 in
    let env = check_reassigned_mutable env te in
    (env, Aast.As_v te)
  | Await_as_v (p, ev) ->
    let (env, te, _) = assign p env ev ty2 in
    let env = check_reassigned_mutable env te in
    (env, Aast.Await_as_v (p, te))
  | As_kv ((p, Lvar ((_, k) as id)), ev) ->
    let env = set_valid_rvalue p env k ty1 in
    let (env, te, _) = assign p env ev ty2 in
    let tk = Tast.make_typed_expr p ty1 (Aast.Lvar id) in
    let env = check_reassigned_mutable env tk in
    let env = check_reassigned_mutable env te in
    (env, Aast.As_kv (tk, te))
  | Await_as_kv (p, (p1, Lvar ((_, k) as id)), ev) ->
    let env = set_valid_rvalue p env k ty1 in
    let (env, te, _) = assign p env ev ty2 in
    let tk = Tast.make_typed_expr p1 ty1 (Aast.Lvar id) in
    let env = check_reassigned_mutable env tk in
    let env = check_reassigned_mutable env te in
    (env, Aast.Await_as_kv (p, tk, te))
  | _ ->
    (* TODO Probably impossible, should check that *)
    assert false

and expr
    ?(expected : ExpectedTy.t option)
    ?(accept_using_var = false)
    ?(is_using_clause = false)
    ?(valkind = `other)
    ?(check_defined = true)
    env
    ((p, _) as e) =
  try
    begin
      match expected with
      | None -> ()
      | Some ExpectedTy.{ reason = r; ty = { et_type = ty; _ }; _ } ->
        Typing_log.(
          log_with_level env "typing" 1 (fun () ->
              log_types
                p
                env
                [
                  Log_head
                    ( "Typing.expr " ^ Typing_reason.string_of_ureason r,
                      [Log_type ("expected_ty", ty)] );
                ]))
    end;
    raw_expr
      ~accept_using_var
      ~is_using_clause
      ~valkind
      ~check_defined
      ?expected
      env
      e
  with e ->
    let stack = Caml.Printexc.get_raw_backtrace () in
    let pos = Pos.string (Pos.to_absolute p) in
    prerr_endline
      (Printf.sprintf
         "Exception while typechecking expression at position %s"
         pos);
    Caml.Printexc.raise_with_backtrace e stack

and raw_expr
    ?(accept_using_var = false)
    ?(is_using_clause = false)
    ?(expected : ExpectedTy.t option)
    ?lhs_of_null_coalesce
    ?(valkind = `other)
    ?(check_defined = true)
    env
    e =
  debug_last_pos := fst e;
  let (env, te, ty) =
    expr_
      ~accept_using_var
      ~is_using_clause
      ?expected
      ?lhs_of_null_coalesce
      ~valkind
      ~check_defined
      env
      e
  in
  let () =
    match !expr_hook with
    | Some f -> f e (Typing_expand.fully_expand env ty)
    | None -> ()
  in
  (env, te, ty)

and lvalue env e =
  let valkind = `lvalue in
  expr_ ~valkind ~check_defined:false env e

and lvalues env el =
  match el with
  | [] -> (env, [], [])
  | e :: el ->
    let (env, te, ty) = lvalue env e in
    let (env, tel, tyl) = lvalues env el in
    (env, te :: tel, ty :: tyl)

and is_pseudo_function s =
  String.equal s SN.PseudoFunctions.hh_show
  || String.equal s SN.PseudoFunctions.hh_show_env
  || String.equal s SN.PseudoFunctions.hh_log_level
  || String.equal s SN.PseudoFunctions.hh_force_solve
  || String.equal s SN.PseudoFunctions.hh_loop_forever

and loop_forever env =
  (* forever = up to 10 minutes, to avoid accidentally stuck processes *)
  for i = 1 to 600 do
    (* Look up things in shared memory occasionally to have a chance to be
     * interrupted *)
    match Env.get_class env "FOR_TEST_ONLY" with
    | None -> Unix.sleep 1
    | _ -> assert false
  done;
  Utils.assert_false_log_backtrace
    (Some "hh_loop_forever was looping for more than 10 minutes")

(* $x ?? 0 is handled similarly to $x ?: 0, except that the latter will also
 * look for sketchy null checks in the condition. *)
(* TODO TAST: type refinement should be made explicit in the typed AST *)
and eif env ~(expected : ExpectedTy.t option) p c e1 e2 =
  let condition = condition ~lhs_of_null_coalesce:false in
  let (env, tc, tyc) = raw_expr ~lhs_of_null_coalesce:false env c in
  let parent_lenv = env.lenv in
  let env = condition env true tc in
  let (env, te1, ty1) =
    match e1 with
    | None ->
      let (env, ty) = Typing_solver.non_null env p tyc in
      (env, None, ty)
    | Some e1 ->
      let (env, te1, ty1) = expr ?expected env e1 in
      (env, Some te1, ty1)
  in
  let lenv1 = env.lenv in
  let env = { env with lenv = parent_lenv } in
  let env = condition env false tc in
  let (env, te2, ty2) = expr ?expected env e2 in
  let lenv2 = env.lenv in
  let env = LEnv.union_lenvs env parent_lenv lenv1 lenv2 in
  let (env, ty) = Union.union env ty1 ty2 in
  make_result env p (Aast.Eif (tc, te1, te2)) ty

and is_parameter env x = Local_id.Map.mem x (Env.get_params env)

and check_escaping_var env (pos, x) =
  if Env.is_using_var env x then
    if Local_id.equal x this then
      Errors.escaping_this pos
    else if is_parameter env x then
      Errors.escaping_disposable_parameter pos
    else
      Errors.escaping_disposable pos
  else
    ()

and exprs
    ?(accept_using_var = false)
    ?(expected : ExpectedTy.t option)
    ?(valkind = `other)
    ?(check_defined = true)
    env
    el =
  match el with
  | [] -> (env, [], [])
  | e :: el ->
    let (env, te, ty) =
      expr ~accept_using_var ?expected ~valkind ~check_defined env e
    in
    let (env, tel, tyl) =
      exprs ~accept_using_var ?expected ~valkind ~check_defined env el
    in
    (env, te :: tel, ty :: tyl)

and exprs_expected (pos, ur, expected_tyl) env el =
  match (el, expected_tyl) with
  | ([], _) -> (env, [], [])
  | (e :: el, expected_ty :: expected_tyl) ->
    let expected = ExpectedTy.make pos ur expected_ty in
    let (env, te, ty) = expr ~expected env e in
    let (env, tel, tyl) = exprs_expected (pos, ur, expected_tyl) env el in
    (env, te :: tel, ty :: tyl)
  | (el, []) -> exprs env el

and make_result env p te ty =
  (* Set the variance of any type variables that were generated according
   * to how they appear in the expression type *)
  let env = Env.set_tyvar_variance env ty in
  (env, Tast.make_typed_expr p ty te, ty)

and localize_targ env ta =
  let pos = fst ta in
  let (env, targ) = Phase.localize_targ env ta in
  (env, targ, ExpectedTy.make pos Reason.URhint (fst targ))

and expr_
    ?(expected : ExpectedTy.t option)
    ?(accept_using_var = false)
    ?(is_using_clause = false)
    ?lhs_of_null_coalesce
    ~(valkind : [> `lvalue | `lvalue_subexpr | `other ])
    ~check_defined
    env
    ((p, e) as outer) =
  let env = Env.open_tyvars env p in
  (fun (env, te, ty) ->
    let env = Typing_solver.close_tyvars_and_solve env Errors.unify_error in
    (env, te, ty))
  @@
  let expr = expr ~check_defined in
  let exprs = exprs ~check_defined in
  let raw_expr = raw_expr ~check_defined in
  (*
   * Given a list of types, computes their supertype. If any of the types are
   * unknown (e.g., comes from PHP), the supertype will be Typing_utils.tany env.
   *)
  let compute_supertype ~(expected : ExpectedTy.t option) ~reason r env tys =
    let p = Reason.to_pos r in
    let (env, supertype) =
      match expected with
      | None -> Env.fresh_type_reason env r
      | Some ExpectedTy.{ ty = { et_type = ty; _ }; _ } -> (env, ty)
    in
    match supertype with
    (* No need to check individual subtypes if expected type is mixed or any! *)
    | (_, Tany _) -> (env, supertype)
    | _ ->
      let subtype_value env ty =
        Type.sub_type p reason env ty supertype Errors.unify_error
      in
      let env = List.fold_left tys ~init:env ~f:subtype_value in
      if
        List.exists tys (fun (_, ty) ->
            equal_locl_ty_ ty (Typing_utils.tany env))
      then
        (* If one of the values comes from PHP land, we have to be conservative
         * and consider that we don't know what the type of the values are. *)
        (env, (Reason.Rwitness p, Typing_utils.tany env))
      else
        (env, supertype)
  in
  (*
   * Given a 'a list and a method to extract an expr and its ty from a 'a, this
   * function extracts a list of exprs from the list, and computes the supertype
   * of all of the expressions' tys.
   *)
  let compute_exprs_and_supertype
      ~(expected : ExpectedTy.t option)
      ?(reason = Reason.URarray_value)
      r
      env
      l
      extract_expr_and_ty =
    let (env, exprs_and_tys) =
      List.map_env env l (extract_expr_and_ty ~expected)
    in
    let (exprs, tys) = List.unzip exprs_and_tys in
    let (env, supertype) = compute_supertype ~expected ~reason r env tys in
    (env, exprs, supertype)
  in
  let forget_fake_members env p callexpr =
    (* Some functions are well known to not change the types of members, e.g.
     * `is_null`.
     * There are a lot of usages like
     *   if (!is_null($x->a) && !is_null($x->a->b))
     * where the second is_null call invalidates the first condition.
     * This function is a bit best effort. Add stuff here when you want
     * To avoid adding too many undue HH_FIXMEs. *)
    match callexpr with
    | (_, Id (_, func))
      when String.equal func SN.StdlibFunctions.is_null
           || String.equal func SN.PseudoFunctions.isset ->
      env
    | _ -> Env.forget_members env (Fake.Blame_call p)
  in
  let check_call
      ~is_using_clause
      ~(expected : ExpectedTy.t option)
      env
      p
      call_type
      e
      explicit_targs
      el
      unpacked_element
      ~in_suspend =
    let (env, te, result) =
      dispatch_call
        ~is_using_clause
        ~expected
        p
        env
        call_type
        e
        explicit_targs
        el
        unpacked_element
        ~in_suspend
    in
    let env = forget_fake_members env p e in
    (env, te, result)
  in
  match e with
  | Import _
  | Collection _
  | BracedExpr _ ->
    failwith "AST should not contain these nodes"
  | Omitted ->
    let r = Reason.Rwitness p in
    let ty = (r, Typing_utils.tany env) in
    make_result env p Aast.Omitted ty
  | ParenthesizedExpr e ->
    let (env, te, ty) = expr env e in
    make_result env p (Aast.ParenthesizedExpr te) ty
  | Any -> expr_error env (Reason.Rwitness p) outer
  | Array [] ->
    (* TODO: use expected type to determine expected element type *)
    make_result env p (Aast.Array []) (Reason.Rwitness p, Tarraykind AKempty)
  | Array (x :: rl as l) ->
    (* True if all fields are values, or all fields are key => value *)
    let fields_consistent = check_consistent_fields x rl in
    let is_vec =
      match x with
      | AFvalue _ -> true
      | AFkvalue _ -> false
    in
    if fields_consistent && is_vec then
      (* Use expected type to determine expected element type *)
      let (env, elem_expected) =
        match expand_expected env expected with
        | (env, Some (pos, ur, ety)) ->
          begin
            match get_akvarray_inst ety with
            | Some vty -> (env, Some (ExpectedTy.make pos ur vty))
            | None -> (env, None)
          end
        | _ -> (env, None)
      in
      let (env, tel, arraykind) =
        let (env, tel, value_ty) =
          compute_exprs_and_supertype
            ~expected:elem_expected
            (Reason.Rtype_variable_generics (p, "T", "array"))
            env
            l
            array_field_value
        in
        (env, tel, AKvarray value_ty)
      in
      make_result
        env
        p
        (Aast.Array (List.map tel (fun e -> Aast.AFvalue e)))
        (Reason.Rwitness p, Tarraykind arraykind)
    else if (* TODO TAST: produce a typed expression here *)
            is_vec then
      (* Use expected type to determine expected element type *)
      let (env, vexpected) =
        match expand_expected env expected with
        | (env, Some (pos, ur, ety)) ->
          begin
            match get_akvarray_inst ety with
            | Some vty -> (env, Some (ExpectedTy.make pos ur vty))
            | None -> (env, None)
          end
        | _ -> (env, None)
      in
      let (env, _value_exprs, value_ty) =
        compute_exprs_and_supertype
          ~expected:vexpected
          (Reason.Rtype_variable_generics (p, "T", "array"))
          env
          l
          array_field_value
      in
      make_result
        env
        p
        Aast.Any
        (Reason.Rwitness p, Tarraykind (AKvarray value_ty))
    else
      (* Use expected type to determine expected element type *)
      let (env, kexpected, vexpected) =
        match expand_expected env expected with
        | (env, Some (pos, reason, ety)) ->
          begin
            match get_akdarray_inst p ety with
            | Some (kty, vty) ->
              let k_expected = ExpectedTy.make pos reason kty in
              let v_expected = ExpectedTy.make pos reason vty in
              (env, Some k_expected, Some v_expected)
            | None -> (env, None, None)
          end
        | _ -> (env, None, None)
      in
      let (env, key_exprs, key_ty) =
        compute_exprs_and_supertype
          ~expected:kexpected
          (Reason.Rtype_variable_generics (p, "Tk", "array"))
          env
          l
          array_field_key
      in
      let (env, value_exprs, value_ty) =
        compute_exprs_and_supertype
          ~expected:vexpected
          (Reason.Rtype_variable_generics (p, "Tv", "array"))
          env
          l
          array_field_value
      in
      make_result
        env
        p
        (Aast.Array
           (List.map (List.zip_exn key_exprs value_exprs) (fun (tek, tev) ->
                Aast.AFkvalue (tek, tev))))
        (Reason.Rwitness p, Tarraykind (AKdarray (key_ty, value_ty)))
  | Varray (th, el)
  | ValCollection (_, th, el) ->
    let (get_expected_kind, name, subtype_val, make_expr, make_ty) =
      match e with
      | ValCollection (kind, _, _) ->
        let class_name = Nast.vc_kind_to_name kind in
        let subtype_val =
          match kind with
          | Set
          | ImmSet
          | Keyset ->
            arraykey_value p class_name
          | Vector
          | ImmVector
          | Vec
          | Pair_ ->
            array_value
        in
        ( get_vc_inst kind,
          class_name,
          subtype_val,
          (fun th elements -> Aast.ValCollection (kind, th, elements)),
          fun value_ty ->
            MakeType.class_type (Reason.Rwitness p) class_name [value_ty] )
      | Varray _ ->
        ( get_varray_inst,
          "varray",
          array_value,
          (fun th elements -> Aast.Varray (th, elements)),
          (fun value_ty -> (Reason.Rwitness p, Tarraykind (AKvarray value_ty)))
        )
      | _ ->
        (* The parent match makes this case impossible *)
        failwith "impossible match case"
    in
    (* Use expected type to determine expected element type *)
    let (env, elem_expected, th) =
      match th with
      | Some (_, tv) ->
        let (env, tv, tv_expected) = localize_targ env tv in
        (env, Some tv_expected, Some tv)
      | _ ->
        begin
          match expand_expected env expected with
          | (env, Some (pos, ur, ety)) ->
            begin
              match get_expected_kind ety with
              | Some vty -> (env, Some (ExpectedTy.make pos ur vty), None)
              | None -> (env, None, None)
            end
          | _ -> (env, None, None)
        end
    in
    let (env, tel, elem_ty) =
      compute_exprs_and_supertype
        ~expected:elem_expected
        ~reason:Reason.URvector
        (Reason.Rtype_variable_generics (p, "T", strip_ns name))
        env
        el
        subtype_val
    in
    make_result env p (make_expr th tel) (make_ty elem_ty)
  | Darray (th, l)
  | KeyValCollection (_, th, l) ->
    let (get_expected_kind, name, make_expr, make_ty) =
      match e with
      | KeyValCollection (kind, _, _) ->
        let class_name = Nast.kvc_kind_to_name kind in
        ( get_kvc_inst p kind,
          class_name,
          (fun th pairs -> Aast.KeyValCollection (kind, th, pairs)),
          (fun k v -> MakeType.class_type (Reason.Rwitness p) class_name [k; v])
        )
      | Darray _ ->
        ( get_darray_inst p,
          "darray",
          (fun th pairs -> Aast.Darray (th, pairs)),
          (fun k v -> (Reason.Rwitness p, Tarraykind (AKdarray (k, v)))) )
      | _ ->
        (* The parent match makes this case impossible *)
        failwith "impossible match case"
    in
    (* Use expected type to determine expected key and value types *)
    let (env, kexpected, vexpected, th) =
      match th with
      | Some ((_, tk), (_, tv)) ->
        let (env, tk, tk_expected) = localize_targ env tk in
        let (env, tv, tv_expected) = localize_targ env tv in
        (env, Some tk_expected, Some tv_expected, Some (tk, tv))
      | _ ->
        (* no explicit typehint, fallback to supplied expect *)
        begin
          match expand_expected env expected with
          | (env, Some (pos, reason, ety)) ->
            begin
              match get_expected_kind ety with
              | Some (kty, vty) ->
                let k_expected = ExpectedTy.make pos reason kty in
                let v_expected = ExpectedTy.make pos reason vty in
                (env, Some k_expected, Some v_expected, None)
              | None -> (env, None, None, None)
            end
          | _ -> (env, None, None, None)
        end
    in
    let (kl, vl) = List.unzip l in
    let (env, tkl, k) =
      compute_exprs_and_supertype
        ~expected:kexpected
        ~reason:Reason.URkey
        (Reason.Rtype_variable_generics (p, "Tk", strip_ns name))
        env
        kl
        (arraykey_value p name)
    in
    let (env, tvl, v) =
      compute_exprs_and_supertype
        ~expected:vexpected
        ~reason:Reason.URvalue
        (Reason.Rtype_variable_generics (p, "Tv", strip_ns name))
        env
        vl
        array_value
    in
    let pairs = List.zip_exn tkl tvl in
    make_result env p (make_expr th pairs) (make_ty k v)
  | Clone e ->
    let (env, te, ty) = expr env e in
    (* Clone only works on objects; anything else fatals at runtime *)
    let tobj = (Reason.Rwitness p, Tobject) in
    let env = Type.sub_type p Reason.URclone env ty tobj Errors.unify_error in
    make_result env p (Aast.Clone te) ty
  | This ->
    let (r, _) = Env.get_self env in
    if Reason.equal r Reason.Rnone then Errors.this_var_outside_class p;
    if not accept_using_var then check_escaping_var env (p, this);
    let (_, ty) = Env.get_local env this in
    let r = Reason.Rwitness p in
    let ty = (r, TUtils.this_of (r, ty)) in
    make_result env p Aast.This ty
  | Assert (AE_assert e) ->
    let (env, te, _) = expr env e in
    let env = LEnv.save_and_merge_next_in_cont env C.Exit in
    let env = condition env true te in
    make_result
      env
      p
      (Aast.Assert (Aast.AE_assert te))
      (MakeType.void (Reason.Rwitness p))
  | True -> make_result env p Aast.True (MakeType.bool (Reason.Rwitness p))
  | False -> make_result env p Aast.False (MakeType.bool (Reason.Rwitness p))
  (* TODO TAST: consider checking that the integer is in range. Right now
   * it's possible for HHVM to fail on well-typed Hack code
   *)
  | Int s -> make_result env p (Aast.Int s) (MakeType.int (Reason.Rwitness p))
  | Float s ->
    make_result env p (Aast.Float s) (MakeType.float (Reason.Rwitness p))
  (* TODO TAST: consider introducing a "null" type, and defining ?t to
   * be null | t
   *)
  | Null -> make_result env p Aast.Null (MakeType.null (Reason.Rwitness p))
  | String s ->
    make_result env p (Aast.String s) (MakeType.string (Reason.Rwitness p))
  | String2 idl ->
    let (env, tel) = string2 env idl in
    make_result env p (Aast.String2 tel) (MakeType.string (Reason.Rwitness p))
  | PrefixedString (n, e) ->
    if String.( <> ) n "re" then (
      Errors.experimental_feature
        p
        "String prefixes other than `re` are not yet supported.";
      expr_error env Reason.Rnone outer
    ) else
      let (env, te, ty) = expr env e in
      let pe = fst e in
      let env = Typing_substring.sub_string pe env ty in
      (match snd e with
      | String _ ->
        begin
          try
            make_result
              env
              p
              (Aast.PrefixedString (n, te))
              (Typing_regex.type_pattern e)
          with
          | Pcre.Error (Pcre.BadPattern (s, i)) ->
            let s = s ^ " [" ^ string_of_int i ^ "]" in
            Errors.bad_regex_pattern pe s;
            expr_error env (Reason.Rregex pe) e
          | Typing_regex.Empty_regex_pattern ->
            Errors.bad_regex_pattern pe "This pattern is empty";
            expr_error env (Reason.Rregex pe) e
          | Typing_regex.Missing_delimiter ->
            Errors.bad_regex_pattern pe "Missing delimiter(s)";
            expr_error env (Reason.Rregex pe) e
          | Typing_regex.Invalid_global_option ->
            Errors.bad_regex_pattern pe "Invalid global option(s)";
            expr_error env (Reason.Rregex pe) e
        end
      | String2 _ ->
        Errors.re_prefixed_non_string pe "Strings with embedded expressions";
        expr_error env (Reason.Rregex pe) e
      | _ ->
        Errors.re_prefixed_non_string pe "Non-strings";
        expr_error env (Reason.Rregex pe) e)
  | Fun_id x ->
    let (env, fty, _tal) = fun_type_of_id env x [] [] in
    make_result env p (Aast.Fun_id x) fty
  | Id ((cst_pos, cst_name) as id) ->
    (match Env.get_gconst env cst_name with
    | None when Partial.should_check_error (Env.get_mode env) 4106 ->
      Errors.unbound_global cst_pos;
      let ty = (Reason.Rwitness cst_pos, Typing_utils.terr env) in
      make_result env cst_pos (Aast.Id id) ty
    | None ->
      make_result
        env
        p
        (Aast.Id id)
        (Reason.Rwitness cst_pos, Typing_utils.tany env)
    | Some (ty, _) ->
      let (env, ty) = Phase.localize_with_self ~pos:p env ty in
      make_result env p (Aast.Id id) ty)
  | Method_id (instance, meth) ->
    (* Method_id is used when creating a "method pointer" using the magic
     * inst_meth function.
     *
     * Typing this is pretty simple, we just need to check that instance->meth
     * is public+not static and then return its type.
     *)
    let (env, te, ty1) = expr env instance in
    let (env, (result, _tal)) =
      TOG.obj_get_
        ~inst_meth:true
        ~obj_pos:p
        ~is_method:true
        ~nullsafe:None
        ~coerce_from_ty:None
        ~pos_params:None
        ~is_nonnull:false
        env
        ty1
        (CIexpr instance)
        meth
        (fun x -> x)
        Errors.unify_error
    in
    let (env, result) =
      Env.FakeMembers.check_instance_invalid env instance (snd meth) result
    in
    make_result env p (Aast.Method_id (te, meth)) result
  | Method_caller (((pos, class_name) as pos_cname), meth_name) ->
    (* meth_caller('X', 'foo') desugars to:
     * $x ==> $x->foo()
     *)
    let class_ = Env.get_class env class_name in
    (match class_ with
    | None -> unbound_name env pos_cname outer
    | Some class_ ->
      (* Create a class type for the given object instantiated with unresolved
       * types for its type parameters.
       *)
      let (env, tvarl) =
        List.map_env env (Cls.tparams class_) (fun env _ ->
            Env.fresh_type env p)
      in
      let params =
        List.map (Cls.tparams class_) (fun { tp_name = (p, n); _ } ->
            (Reason.Rwitness p, Tgeneric n))
      in
      let obj_type = (Reason.Rwitness p, Tapply (pos_cname, params)) in
      let ety_env =
        {
          (Phase.env_with_self env) with
          substs = Subst.make_locl (Cls.tparams class_) tvarl;
        }
      in
      let (env, local_obj_ty) = Phase.localize ~ety_env env obj_type in
      let (env, (fty, _tal)) =
        TOG.obj_get
          ~obj_pos:pos
          ~is_method:true
          ~nullsafe:None
          ~coerce_from_ty:None
          env
          local_obj_ty
          (CI (pos, class_name))
          meth_name
          Errors.unify_error
      in
      let (env, fty) = Env.expand_type env fty in
      (match fty with
      | (reason, Tfun ftype) ->
        (* We are creating a fake closure:
         * function(Class $x, arg_types_of(Class::meth_name))
                 : return_type_of(Class::meth_name)
         *)
        let ety_env =
          { ety_env with substs = Subst.make_locl (Cls.tparams class_) tvarl }
        in
        let env =
          Phase.check_tparams_constraints
            ~use_pos:p
            ~ety_env
            env
            (Cls.tparams class_)
        in
        let (env, local_obj_ty) = Phase.localize ~ety_env env obj_type in
        let local_obj_fp = TUtils.default_fun_param local_obj_ty in
        let fty = { ftype with ft_params = local_obj_fp :: ftype.ft_params } in
        let fun_arity =
          match fty.ft_arity with
          | Fstandard (min, max) -> Fstandard (min + 1, max + 1)
          | Fvariadic (min, x) -> Fvariadic (min + 1, x)
          | Fellipsis (min, p) -> Fellipsis (min + 1, p)
        in
        let caller =
          {
            (* propagate 'is_coroutine' from the method being called*)
            ft_is_coroutine = fty.ft_is_coroutine;
            ft_arity = fun_arity;
            ft_tparams = fty.ft_tparams;
            ft_where_constraints = fty.ft_where_constraints;
            ft_params = fty.ft_params;
            ft_ret = fty.ft_ret;
            ft_fun_kind = fty.ft_fun_kind;
            ft_reactive = fty.ft_reactive;
            ft_mutability = fty.ft_mutability;
            ft_returns_mutable = fty.ft_returns_mutable;
            ft_return_disposable = fty.ft_return_disposable;
            ft_returns_void_to_rx = fty.ft_returns_void_to_rx;
          }
        in
        make_result
          env
          p
          (Aast.Method_caller (pos_cname, meth_name))
          (reason, Tfun caller)
      | _ ->
        (* This can happen if the method lives in PHP *)
        make_result
          env
          p
          (Aast.Method_caller (pos_cname, meth_name))
          (Reason.Rwitness pos, Typing_utils.tany env)))
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
      unbound_name env c outer
    | Some class_ ->
      let smethod = Env.get_static_member true env class_ (snd meth) in
      (match smethod with
      | None ->
        (* The static method wasn't found. *)
        TOG.smember_not_found
          p
          ~is_const:false
          ~is_method:true
          class_
          (snd meth)
          Errors.unify_error;
        expr_error env Reason.Rnone outer
      | Some
          {
            ce_type = (lazy ty);
            ce_visibility;
            ce_deprecated;
            ce_pos = (lazy ce_pos);
            _;
          } ->
        let cid = CI c in
        let (env, _tal, _te, cid_ty) =
          static_class_id ~check_constraints:true (fst c) env [] cid
        in
        let (env, cid_ty) = Env.expand_type env cid_ty in
        let tyargs =
          match cid_ty with
          | (_, Tclass (_, _, tyargs)) -> tyargs
          | _ -> []
        in
        let ety_env =
          {
            type_expansions = [];
            substs = Subst.make_locl (Cls.tparams class_) tyargs;
            this_ty = cid_ty;
            from_class = Some cid;
            quiet = true;
            on_error = Errors.unify_error_at p;
          }
        in
        (match ty with
        | (r, Tfun ft) ->
          let ft =
            Typing_enforceability.compute_enforced_and_pessimize_fun_type env ft
          in
          let def_pos = ce_pos in
          let (env, tal) =
            Phase.localize_targs
              ~is_method:true
              ~def_pos:ce_pos
              ~use_pos:p
              ~use_name:(strip_ns (snd meth))
              env
              (fst ft.ft_tparams)
              []
          in
          let (env, ft) =
            Phase.(
              localize_ft
                ~instantiation:
                  Phase.
                    {
                      use_name = strip_ns (snd meth);
                      use_pos = p;
                      explicit_targs = tal;
                    }
                ~ety_env
                ~def_pos:ce_pos
                env
                ft)
          in
          let ty = (r, Tfun ft) in
          let use_pos = fst meth in
          TVis.check_deprecated ~use_pos ~def_pos ce_deprecated;
          (match ce_visibility with
          | Vpublic -> make_result env p (Aast.Smethod_id (c, meth)) ty
          | Vprivate _ ->
            Errors.private_class_meth ~def_pos ~use_pos;
            expr_error env r outer
          | Vprotected _ ->
            Errors.protected_class_meth ~def_pos ~use_pos;
            expr_error env r outer)
        | (r, _) ->
          Errors.internal_error p "We have a method which isn't callable";
          expr_error env r outer)))
  | Lplaceholder p ->
    let r = Reason.Rplaceholder p in
    let ty = MakeType.void r in
    make_result env p (Aast.Lplaceholder p) ty
  | Dollardollar _ when phys_equal valkind `lvalue ->
    Errors.dollardollar_lvalue p;
    expr_error env (Reason.Rwitness p) outer
  | Dollardollar id ->
    let ty = Env.get_local_check_defined env id in
    let env = might_throw env in
    make_result env p (Aast.Dollardollar id) ty
  | Lvar ((_, x) as id) ->
    if not accept_using_var then check_escaping_var env id;
    let ty =
      if check_defined then
        Env.get_local_check_defined env id
      else
        Env.get_local env x
    in
    make_result env p (Aast.Lvar id) ty
  | List el ->
    let (env, tel, tyl) =
      match valkind with
      | `lvalue
      | `lvalue_subexpr ->
        lvalues env el
      | `other ->
        let (env, expected) = expand_expected env expected in
        (match expected with
        | Some (pos, ur, (_, Ttuple expected_tyl)) ->
          exprs_expected (pos, ur, expected_tyl) env el
        | _ -> exprs env el)
    in
    let ty = (Reason.Rwitness p, Ttuple tyl) in
    make_result env p (Aast.List tel) ty
  | Pair (e1, e2) ->
    (* Use expected type to determine expected element types *)
    let (env, expected1, expected2) =
      match expand_expected env expected with
      | (env, Some (pos, reason, (_, Tclass ((_, k), _, [ty1; ty2]))))
        when String.equal k SN.Collections.cPair ->
        let ty1_expected = ExpectedTy.make pos reason ty1 in
        let ty2_expected = ExpectedTy.make pos reason ty2 in
        (env, Some ty1_expected, Some ty2_expected)
      | _ -> (env, None, None)
    in
    let (env, te1, ty1) = expr ?expected:expected1 env e1 in
    let (env, te2, ty2) = expr ?expected:expected2 env e2 in
    let ty = MakeType.pair (Reason.Rwitness p) ty1 ty2 in
    make_result env p (Aast.Pair (te1, te2)) ty
  | Expr_list el ->
    (* TODO: use expected type to determine tuple component types *)
    let (env, tel, tyl) = exprs env el in
    let ty = (Reason.Rwitness p, Ttuple tyl) in
    make_result env p (Aast.Expr_list tel) ty
  | Array_get (e, None) ->
    let (env, te, _) = update_array_type p env e None valkind in
    let env = might_throw env in
    (* NAST check reports an error if [] is used for reading in an
         lvalue context. *)
    let ty = (Reason.Rwitness p, Typing_utils.terr env) in
    make_result env p (Aast.Array_get (te, None)) ty
  | Array_get (e1, Some e2) ->
    let (env, te1, ty1) =
      update_array_type ?lhs_of_null_coalesce p env e1 (Some e2) valkind
    in
    let (env, te2, ty2) = expr env e2 in
    let env = might_throw env in
    let is_lvalue = phys_equal valkind `lvalue in
    let (env, ty) =
      Typing_array_access.array_get
        ~array_pos:(fst e1)
        ~expr_pos:p
        ?lhs_of_null_coalesce
        is_lvalue
        env
        ty1
        e2
        ty2
    in
    make_result env p (Aast.Array_get (te1, Some te2)) ty
  | Call (Cnormal, (pos_id, Id ((_, s) as id)), [], el, None)
    when is_pseudo_function s ->
    let (env, tel, tys) = exprs ~accept_using_var:true env el in
    let env =
      if String.equal s SN.PseudoFunctions.hh_show then (
        List.iter tys (Typing_log.hh_show p env);
        env
      ) else if String.equal s SN.PseudoFunctions.hh_show_env then (
        Typing_log.hh_show_env p env;
        env
      ) else if String.equal s SN.PseudoFunctions.hh_log_level then
        match el with
        | [(_, String key_str); (_, Int level_str)] ->
          Env.set_log_level env key_str (int_of_string level_str)
        | _ -> env
      else if String.equal s SN.PseudoFunctions.hh_force_solve then
        Typing_solver.solve_all_unsolved_tyvars env Errors.unify_error
      else if String.equal s SN.PseudoFunctions.hh_loop_forever then (
        loop_forever env;
        env
      ) else
        env
    in
    let ty = MakeType.void (Reason.Rwitness p) in
    make_result
      env
      p
      (Aast.Call
         ( Cnormal,
           Tast.make_typed_expr
             pos_id
             (Reason.Rnone, TUtils.tany env)
             (Aast.Id id),
           [],
           tel,
           None ))
      ty
  | Call (call_type, e, explicit_targs, el, unpacked_element) ->
    let env = might_throw env in
    let (env, te, ty) =
      check_call
        ~is_using_clause
        ~expected
        env
        p
        call_type
        e
        explicit_targs
        el
        unpacked_element
        ~in_suspend:false
    in
    (env, te, ty)
  | Binop (Ast_defs.QuestionQuestion, e1, e2) ->
    let (env, te1, ty1) = raw_expr ~lhs_of_null_coalesce:true env e1 in
    let (env, te2, ty2) = expr ?expected env e2 in
    let (env, ty1') = Env.fresh_type env (fst e1) in
    let env =
      SubType.sub_type
        env
        ty1
        (MakeType.nullable_locl Reason.Rnone ty1')
        Errors.unify_error
    in
    (* Essentially mimic a call to
     *   function coalesce<Tr, Ta as Tr, Tb as Tr>(?Ta, Tb): Tr
     * That way we let the constraint solver take care of the union logic.
     *)
    let (env, ty_result) = Env.fresh_type env (fst e2) in
    let env = SubType.sub_type env ty1' ty_result Errors.unify_error in
    let env = SubType.sub_type env ty2 ty_result Errors.unify_error in
    make_result
      env
      p
      (Aast.Binop (Ast_defs.QuestionQuestion, te1, te2))
      ty_result
  (* For example, e1 += e2. This is typed and translated as if
   * written e1 = e1 + e2.
   * TODO TAST: is this right? e1 will get evaluated more than once
   *)
  | Binop (Ast_defs.Eq (Some op), e1, e2) ->
    begin
      match (op, snd e1) with
      | (Ast_defs.QuestionQuestion, Class_get _) ->
        Errors.experimental_feature
          p
          "null coalesce assignment operator with static properties";
        expr_error env Reason.Rnone outer
      | _ ->
        let e_fake =
          (p, Binop (Ast_defs.Eq None, e1, (p, Binop (op, e1, e2))))
        in
        let (env, te_fake, ty) = raw_expr env e_fake in
        begin
          match snd te_fake with
          | Aast.Binop (_, te1, (_, Aast.Binop (_, _, te2))) ->
            let te = Aast.Binop (Ast_defs.Eq (Some op), te1, te2) in
            make_result env p te ty
          | _ -> assert false
        end
    end
  | Binop (Ast_defs.Eq None, e1, e2) ->
    let (env, te2, ty2) = raw_expr env e2 in
    let (env, te1, ty) = assign p env e1 ty2 in
    let env =
      if Env.env_local_reactive env then
        Typing_mutability.handle_assignment_mutability env te1 (Some (snd te2))
      else
        env
    in
    (* If we are assigning a local variable to another local variable then
     * the expression ID associated with e2 is transferred to e1
     *)
    (match (e1, e2) with
    | ((_, Lvar (_, x1)), (_, Lvar (_, x2))) ->
      let eid2 = Env.get_local_expr_id env x2 in
      let env =
        Option.value_map eid2 ~default:env ~f:(Env.set_local_expr_id env x1)
      in
      make_result env p (Aast.Binop (Ast_defs.Eq None, te1, te2)) ty
    | _ -> make_result env p (Aast.Binop (Ast_defs.Eq None, te1, te2)) ty)
  | Binop (((Ast_defs.Ampamp | Ast_defs.Barbar) as bop), e1, e2) ->
    let c = Ast_defs.(equal_bop bop Ampamp) in
    let (env, te1, _) = expr env e1 in
    let lenv = env.lenv in
    let env = condition env c te1 in
    let (env, te2, _) = expr env e2 in
    let env = { env with lenv } in
    make_result
      env
      p
      (Aast.Binop (bop, te1, te2))
      (MakeType.bool (Reason.Rlogic_ret p))
  | Binop (bop, e1, e2) ->
    let (env, te1, ty1) = raw_expr env e1 in
    let (env, te2, ty2) = raw_expr env e2 in
    let env =
      match bop with
      (* TODO: This could be less conservative: we only need to account for
       * the possibility of exception if the operator is `/` or `/=`.
       *)
      | Ast_defs.Eqeqeq
      | Ast_defs.Diff2 ->
        env
      | _ -> might_throw env
    in
    let (env, te3, ty) =
      Typing_arithmetic.binop p env bop (fst e1) te1 ty1 (fst e2) te2 ty2
    in
    (env, te3, ty)
  | Pipe (e0, e1, e2) ->
    (* If it weren't for local variable assignment or refinement the pipe
     * expression e1 |> e2 could be typed using this rule (E is environment with
     * types for locals):
     *
     *    E |- e1 : ty1    E[$$:ty1] |- e2 : ty2
     *    --------------------------------------
     *                E |- e1|>e2 : ty2
     *
     * The possibility of e2 changing the types of locals in E means that E
     * can evolve, and so we need to restore $$ to its original state.
     *)
    let (env, te1, ty1) = expr env e1 in
    let dd_var = Local_id.make_unscoped SN.SpecialIdents.dollardollar in
    let dd_old_ty =
      if Env.is_local_defined env dd_var then
        Some (Env.get_local env dd_var)
      else
        None
    in
    let env = Env.set_local env dd_var ty1 in
    let (env, te2, ty2) = expr env e2 in
    let env =
      match dd_old_ty with
      | None -> Env.unset_local env dd_var
      | Some ty -> Env.set_local env dd_var ty
    in
    make_result env p (Aast.Pipe (e0, te1, te2)) ty2
  | Unop (uop, e) ->
    let (env, te, ty) = raw_expr env e in
    let env = might_throw env in
    Typing_arithmetic.unop p env uop te ty
  | Eif (c, e1, e2) -> eif env ~expected p c e1 e2
  | Typename sid ->
    begin
      match Env.get_typedef env (snd sid) with
      | Some { td_tparams = tparaml; _ } ->
        (* Typedef type parameters cannot have constraints *)
        let params =
          List.map
            ~f:
              begin
                fun { tp_name = (p, x); _ } ->
                (Reason.Rwitness p, Tgeneric x)
              end
            tparaml
        in
        let tdef = (Reason.Rwitness (fst sid), Tapply (sid, params)) in
        let typename =
          (Reason.Rwitness p, Tapply ((p, SN.Classes.cTypename), [tdef]))
        in
        let (env, tparams) =
          List.map_env env tparaml (fun env tp ->
              Env.fresh_type env (fst tp.tp_name))
        in
        let ety_env =
          {
            (Phase.env_with_self env) with
            substs = Subst.make_locl tparaml tparams;
          }
        in
        let env =
          Phase.check_tparams_constraints ~use_pos:p ~ety_env env tparaml
        in
        let (env, ty) = Phase.localize ~ety_env env typename in
        make_result env p (Aast.Typename sid) ty
      | None ->
        (* Should never hit this case since we only construct this AST node
         * if in the expression Foo::class, Foo is a type def.
         *)
        expr_error env (Reason.Rwitness p) outer
    end
  | Class_const (cid, mid) -> class_const env p (cid, mid)
  | Class_get ((cpos, cid), CGstring mid)
    when Env.FakeMembers.is_valid_static env cid (snd mid) ->
    let (env, local) = Env.FakeMembers.make_static env cid (snd mid) in
    let local = (p, Lvar (p, local)) in
    let (env, _, ty) = expr env local in
    let (env, _tal, te, _) =
      static_class_id ~check_constraints:false cpos env [] cid
    in
    make_result env p (Aast.Class_get (te, Aast.CGstring mid)) ty
  | Class_get ((cpos, cid), CGstring mid) ->
    let (env, _tal, te, cty) =
      static_class_id ~check_constraints:false cpos env [] cid
    in
    let env = might_throw env in
    let (env, (ty, _tal)) =
      class_get
        ~is_method:false
        ~is_const:false
        ~coerce_from_ty:None
        env
        cty
        mid
        cid
    in
    let (env, ty) = Env.FakeMembers.check_static_invalid env cid (snd mid) ty in
    make_result env p (Aast.Class_get (te, Aast.CGstring mid)) ty
  (* Fake member property access. For example:
   *   if ($x->f !== null) { ...$x->f... }
   *)
  | Class_get (_, CGexpr _) ->
    failwith "AST should not have any CGexprs after naming"
  | Obj_get (e, (pid, Id (py, y)), nf) when Env.FakeMembers.is_valid env e y ->
    let env = might_throw env in
    let (env, local) = Env.FakeMembers.make env e y in
    let local = (p, Lvar (p, local)) in
    let (env, _, ty) = expr env local in
    let (env, t_lhs, _) = expr ~accept_using_var:true env e in
    let t_rhs = Tast.make_typed_expr pid ty (Aast.Id (py, y)) in
    make_result env p (Aast.Obj_get (t_lhs, t_rhs, nf)) ty
  (* Statically-known instance property access e.g. $x->f *)
  | Obj_get (e1, (pm, Id m), nullflavor) ->
    let nullsafe =
      match nullflavor with
      | OG_nullthrows -> None
      | OG_nullsafe -> Some p
    in
    let (env, te1, ty1) = expr ~accept_using_var:true env e1 in
    let env = might_throw env in
    (* We typecheck Obj_get by checking whether it is a subtype of
    Thas_member(m, #1) where #1 is a fresh type variable. *)
    let (env, mem_ty) = Env.fresh_type env p in
    let r = Reason.Rwitness (fst e1) in
    let has_member_ty = MakeType.has_member r m mem_ty (CIexpr e1) in
    let on_error = Errors.unify_error in
    let lty1 = LoclType ty1 in
    let (env, result_ty) =
      match nullsafe with
      | None ->
        let env = SubType.sub_type_i env lty1 has_member_ty on_error in
        (env, mem_ty)
      | Some _ ->
        (* In that case ty1 is a subtype of ?Thas_member(m, #1)
        and the result is ?#1 if ty1 is nullable. *)
        let r = Reason.Rnullsafe_op p in
        let null_ty = MakeType.null r in
        let (env, null_has_mem_ty) =
          Union.union_i env r has_member_ty null_ty
        in
        let env = SubType.sub_type_i env lty1 null_has_mem_ty on_error in
        let (env, null_or_nothing_ty) = Inter.intersect env ~r null_ty ty1 in
        let (env, result_ty) = Union.union env null_or_nothing_ty mem_ty in
        (env, result_ty)
    in
    let (env, result_ty) =
      Env.FakeMembers.check_instance_invalid env e1 (snd m) result_ty
    in
    make_result
      env
      p
      (Aast.Obj_get
         (te1, Tast.make_typed_expr pm result_ty (Aast.Id m), nullflavor))
      result_ty
  (* Dynamic instance property access e.g. $x->$f *)
  | Obj_get (e1, e2, nullflavor) ->
    let (env, te1, ty1) = expr ~accept_using_var:true env e1 in
    let (env, te2, _) = expr env e2 in
    let ty =
      if TUtils.is_dynamic env ty1 then
        MakeType.dynamic (Reason.Rwitness p)
      else
        (Reason.Rwitness p, Typing_utils.tany env)
    in
    let ((pos, _), te2) = te2 in
    let env = might_throw env in
    let te2 = Tast.make_typed_expr pos ty te2 in
    make_result env p (Aast.Obj_get (te1, te2, nullflavor)) ty
  | Yield_break ->
    make_result env p Aast.Yield_break (Reason.Rwitness p, Typing_utils.tany env)
  | Yield af ->
    let (env, (taf, opt_key, value)) = array_field env af in
    let (env, send) = Env.fresh_type env p in
    let (env, key) =
      match (af, opt_key) with
      | (AFvalue (p, _), None) ->
        begin
          match Env.get_fn_kind env with
          | Ast_defs.FCoroutine
          | Ast_defs.FSync
          | Ast_defs.FAsync ->
            Errors.internal_error p "yield found in non-generator";
            (env, (Reason.Rwitness p, Typing_utils.tany env))
          | Ast_defs.FGenerator -> (env, MakeType.int (Reason.Rwitness p))
          | Ast_defs.FAsyncGenerator ->
            let (env, ty) = Env.fresh_type env p in
            (env, MakeType.nullable_locl (Reason.Ryield_asyncnull p) ty)
        end
      | (_, Some x) -> (env, x)
      | (_, _) -> assert false
    in
    let rty =
      match Env.get_fn_kind env with
      | Ast_defs.FCoroutine ->
        (* yield in coroutine is already reported as error in NastCheck *)
        let (_, _, ty) = expr_error env (Reason.Rwitness p) outer in
        ty
      | Ast_defs.FGenerator ->
        MakeType.generator (Reason.Ryield_gen p) key value send
      | Ast_defs.FAsyncGenerator ->
        MakeType.async_generator (Reason.Ryield_asyncgen p) key value send
      | Ast_defs.FSync
      | Ast_defs.FAsync ->
        failwith "Parsing should never allow this"
    in
    let Typing_env_return_info.{ return_type = expected_return; _ } =
      Env.get_return env
    in
    let env =
      Typing_coercion.coerce_type
        p
        Reason.URyield
        env
        rty
        expected_return
        Errors.unify_error
    in
    let env = Env.forget_members env (Fake.Blame_call p) in
    let env = LEnv.save_and_merge_next_in_cont env C.Exit in
    make_result
      env
      p
      (Aast.Yield taf)
      (MakeType.nullable_locl (Reason.Ryield_send p) send)
  | Yield_from e ->
    let (env, key) = Env.fresh_type env p in
    let (env, value) = Env.fresh_type env p in
    let (env, te, yield_from_ty) = expr ~is_using_clause env e in
    (* Expected type of `e` in `yield from e` is KeyedTraversable<Tk,Tv> (but might be dynamic)*)
    let expected_yield_from_ty =
      MakeType.keyed_traversable (Reason.Ryield_gen p) key value
    in
    let from_dynamic =
      Typing_solver.is_sub_type
        env
        yield_from_ty
        (MakeType.dynamic (fst yield_from_ty))
    in
    let env =
      if from_dynamic then
        env
      (* all set if dynamic, otherwise need to check against KeyedTraversable *)
      else
        Typing_coercion.coerce_type
          p
          Reason.URyield_from
          env
          yield_from_ty
          (MakeType.unenforced expected_yield_from_ty)
          Errors.unify_error
    in
    let rty =
      match Env.get_fn_kind env with
      | Ast_defs.FCoroutine ->
        (* yield in coroutine is already reported as error in NastCheck *)
        let (_, _, ty) = expr_error env (Reason.Rwitness p) outer in
        ty
      | Ast_defs.FGenerator ->
        if from_dynamic then
          MakeType.dynamic (Reason.Ryield_gen p)
        (*TODO: give better reason*)
        else
          MakeType.generator
            (Reason.Ryield_gen p)
            key
            value
            (MakeType.void (Reason.Rwitness p))
      | Ast_defs.FSync
      | Ast_defs.FAsync
      | Ast_defs.FAsyncGenerator ->
        failwith "Parsing should never allow this"
    in
    let Typing_env_return_info.{ return_type = expected_return; _ } =
      Env.get_return env
    in
    let env =
      Typing_coercion.coerce_type
        p
        Reason.URyield_from
        env
        rty
        { expected_return with et_enforced = false }
        Errors.unify_error
    in
    let env = Env.forget_members env (Fake.Blame_call p) in
    make_result env p (Aast.Yield_from te) (MakeType.void (Reason.Rwitness p))
  | Await e ->
    let env = might_throw env in
    (* Await is permitted in a using clause e.g. using (await make_handle()) *)
    let (env, te, rty) = expr ~is_using_clause env e in
    let (env, ty) = Async.overload_extract_from_awaitable env p rty in
    make_result env p (Aast.Await te) ty
  | Suspend e ->
    let (env, te, ty) =
      match e with
      | (_, Call (call_type, e, explicit_targs, el, unpacked_element)) ->
        let env = Env.open_tyvars env p in
        (fun (env, te, ty) ->
          (Typing_solver.close_tyvars_and_solve env Errors.unify_error, te, ty))
        @@ check_call
             ~is_using_clause
             ~expected
             env
             p
             call_type
             e
             explicit_targs
             el
             unpacked_element
             ~in_suspend:true
      | (epos, _) ->
        let (env, te, ty) = expr env e in
        (* not a call - report an error *)
        Errors.non_call_argument_in_suspend
          epos
          (Reason.to_string ("This is " ^ Typing_print.error env ty) (fst ty));
        (env, te, ty)
    in
    make_result env p (Aast.Suspend te) ty
  | New ((pos, c), explicit_targs, el, unpacked_element, p1) ->
    let env = might_throw env in
    let (env, tc, tal, tel, typed_unpack_element, ty, ctor_fty) =
      new_object
        ~expected
        ~is_using_clause
        ~check_parent:false
        ~check_not_abstract:true
        pos
        env
        c
        explicit_targs
        el
        unpacked_element
    in
    let env = Env.forget_members env (Fake.Blame_call p) in
    make_result
      env
      p
      (Aast.New (tc, tal, tel, typed_unpack_element, (p1, ctor_fty)))
      ty
  | Record ((pos, id), _is_array, field_values) ->
    (match Decl_provider.get_record_def (Env.get_ctx env) id with
    | Some rd ->
      if rd.rdt_abstract then Errors.new_abstract_record (pos, id);

      let field_name (pos, expr_) =
        match expr_ with
        | Aast.String name -> Some (pos, name)
        | _ ->
          (* TODO T44306013: Ensure that other values for field names are banned. *)
          None
      in
      let fields_declared = all_record_fields env rd in
      let fields_present =
        List.map field_values ~f:(fun (name, _value) -> field_name name)
        |> List.filter_opt
      in
      (* Check for missing required fields. *)
      let fields_present_names =
        List.map ~f:snd fields_present |> SSet.of_list
      in
      SMap.iter
        (fun field_name info ->
          let ((field_pos, _), req) = info in
          match req with
          | Typing_defs.ValueRequired
            when not (SSet.mem field_name fields_present_names) ->
            Errors.missing_record_field_name
              ~field_name
              ~new_pos:pos
              ~record_name:id
              ~field_decl_pos:field_pos
          | _ -> ())
        fields_declared;

      (* Check for unknown fields.*)
      List.iter fields_present ~f:(fun (pos, field_name) ->
          if not (SMap.mem field_name fields_declared) then
            Errors.unexpected_record_field_name
              ~field_name
              ~field_pos:pos
              ~record_name:id
              ~decl_pos:(fst rd.rdt_name))
    | None -> ());

    expr_error env (Reason.Rwitness p) outer
  | Cast ((_, Harray (None, None)), _)
    when Partial.should_check_error (Env.get_mode env) 4007
         || TCO.migration_flag_enabled (Env.get_tcopt env) "array_cast" ->
    Errors.array_cast p;
    expr_error env (Reason.Rwitness p) outer
  | Cast (hint, e) ->
    let (env, te, ty2) = expr env e in
    let env = might_throw env in
    let env =
      if
        TypecheckerOptions.experimental_feature_enabled
          (Env.get_tcopt env)
          TypecheckerOptions.experimental_forbid_nullable_cast
        && not (TUtils.is_mixed env ty2)
      then
        SubType.sub_type_or_fail
          env
          ty2
          (MakeType.nonnull (fst ty2))
          (fun () ->
            Errors.nullable_cast
              p
              (Typing_print.error env ty2)
              (Reason.to_pos (fst ty2)))
      else
        env
    in
    let (env, ty) = Phase.localize_hint_with_self env hint in
    make_result env p (Aast.Cast (hint, te)) ty
  | Is (e, hint) ->
    let (env, te, _) = expr env e in
    make_result env p (Aast.Is (te, hint)) (MakeType.bool (Reason.Rwitness p))
  | As (e, hint, is_nullable) ->
    let refine_type env lpos lty rty =
      let reason = Reason.Ras lpos in
      let (env, rty) = Env.expand_type env rty in
      let (env, rty) = class_for_refinement env p reason lpos lty rty in
      Inter.intersect env reason lty rty
    in
    let (env, te, expr_ty) = expr env e in
    let env = might_throw env in
    let ety_env =
      { (Phase.env_with_self env) with from_class = Some CIstatic }
    in
    let (env, hint_ty) = Phase.localize_hint ~ety_env env hint in
    let (env, hint_ty) =
      if Typing_utils.is_dynamic env hint_ty then
        let env =
          if is_instance_var e then
            let (env, ivar) = get_instance_var env e in
            set_local env ivar hint_ty
          else
            env
        in
        (env, hint_ty)
      else if is_nullable then
        let (env, hint_ty) = refine_type env (fst e) expr_ty hint_ty in
        (env, MakeType.nullable_locl (Reason.Rwitness p) hint_ty)
      else if is_instance_var e then
        let (env, _, ivar_ty) = raw_expr env e in
        let (env, ((ivar_pos, _) as ivar)) = get_instance_var env e in
        let (env, hint_ty) = refine_type env ivar_pos ivar_ty hint_ty in
        let env = set_local env ivar hint_ty in
        (env, hint_ty)
      else
        refine_type env (fst e) expr_ty hint_ty
    in
    make_result env p (Aast.As (te, hint, is_nullable)) hint_ty
  | Efun (f, idl)
  | Lfun (f, idl) ->
    let is_anon =
      match e with
      | Efun _ -> true
      | Lfun _ -> false
      | _ -> assert false
    in
    (* This is the function type as declared on the lambda itself.
     * If type hints are absent then use Tany instead. *)
    let declared_fe = Decl.fun_decl_in_env env.decl_env f in
    let (declared_pos, declared_ft) =
      match declared_fe with
      | { fe_type = (_, Tfun ft); fe_pos; _ } -> (fe_pos, ft)
      | _ -> failwith "Not a function"
    in
    let declared_ft =
      Typing_enforceability.compute_enforced_and_pessimize_fun_type
        env
        declared_ft
    in
    (* When creating a closure, the 'this' type will mean the late bound type
     * of the current enclosing class
     *)
    let ety_env =
      { (Phase.env_with_self env) with from_class = Some CIstatic }
    in
    let (env, declared_ft) =
      Phase.(
        localize_ft
          ~instantiation:
            { use_name = "lambda"; use_pos = p; explicit_targs = [] }
          ~ety_env
          ~def_pos:declared_pos
          env
          declared_ft)
    in
    List.iter idl (check_escaping_var env);

    (* Ensure lambda arity is not Fellipsis in strict mode *)
    begin
      match declared_ft.ft_arity with
      | Fellipsis _ when Partial.should_check_error (Env.get_mode env) 4223 ->
        Errors.ellipsis_strict_mode ~require:`Param_name p
      | _ -> ()
    end;

    (* Is the return type declared? *)
    let is_explicit_ret = Option.is_some (hint_of_type_hint f.f_ret) in
    let reactivity =
      Decl_fun_utils.fun_reactivity_opt env.decl_env f.f_user_attributes
      |> Option.value
           ~default:(TR.strip_conditional_reactivity (env_reactivity env))
    in
    let check_body_under_known_params env ?ret_ty ft =
      let old_reactivity = env_reactivity env in
      let env = Env.set_env_reactive env reactivity in
      let old_inside_ppl_class = env.inside_ppl_class in
      let env = { env with inside_ppl_class = false } in
      let ft = { ft with ft_reactive = reactivity } in
      let (is_coroutine, _counter, _, anon) =
        anon_make env p f ft idl is_anon outer
      in
      let ft = { ft with ft_is_coroutine = is_coroutine } in
      let (env, tefun, ty) = anon ?ret_ty env ft.ft_params ft.ft_arity in
      let env = Env.set_env_reactive env old_reactivity in
      let env = { env with inside_ppl_class = old_inside_ppl_class } in
      let inferred_ty =
        ( Reason.Rwitness p,
          Tfun
            {
              ft with
              ft_ret =
                ( if is_explicit_ret then
                  declared_ft.ft_ret
                else
                  MakeType.unenforced ty );
            } )
      in
      (env, tefun, inferred_ty)
    in
    let (env, eexpected) = expand_expected env expected in
    (* TODO: move this into the expand_expected function and prune its callsites
     * Strip like type from function type hint *)
    let eexpected =
      match eexpected with
      | Some (pos, ur, (_, Tunion [(_, Tdynamic); ((_, Tfun _) as f)])) ->
        Some (pos, ur, f)
      | _ -> eexpected
    in
    begin
      match eexpected with
      | Some (_pos, _ur, (r, Tfun expected_ft)) ->
        (* First check that arities match up *)
        check_lambda_arity
          p
          (Reason.to_pos r)
          declared_ft.ft_arity
          expected_ft.ft_arity;

        (* Use declared types for parameters in preference to those determined
         * by the context: they might be more general. *)
        let rec replace_non_declared_types
            params declared_ft_params expected_ft_params =
          match (params, declared_ft_params, expected_ft_params) with
          | ( param :: params,
              declared_ft_param :: declared_ft_params,
              expected_ft_param :: expected_ft_params ) ->
            let rest =
              replace_non_declared_types
                params
                declared_ft_params
                expected_ft_params
            in
            let resolved_ft_param =
              if Option.is_some (hint_of_type_hint param.param_type_hint) then
                declared_ft_param
              else
                { declared_ft_param with fp_type = expected_ft_param.fp_type }
            in
            resolved_ft_param :: rest
          | (_, _, _) ->
            (* This means the expected_ft params list can have more parameters
             * than declared parameters in the lambda. For variadics, this is OK,
             * for non-variadics, this will be caught elsewhere in arity checks.
             *)
            expected_ft_params
        in
        let replace_non_declared_arity variadic declared_arity expected_arity =
          match variadic with
          | FVvariadicArg { param_type_hint = (_, Some _); _ } -> declared_arity
          | FVvariadicArg _ ->
            begin
              match (declared_arity, expected_arity) with
              | (Fvariadic (min_arity, declared), Fvariadic (_, expected)) ->
                Fvariadic
                  (min_arity, { declared with fp_type = expected.fp_type })
              | (_, _) -> declared_arity
            end
          | _ -> declared_arity
        in
        let expected_ft =
          {
            expected_ft with
            ft_arity =
              replace_non_declared_arity
                f.f_variadic
                declared_ft.ft_arity
                expected_ft.ft_arity;
          }
        in
        let expected_ft =
          {
            expected_ft with
            ft_params =
              replace_non_declared_types
                f.f_params
                declared_ft.ft_params
                expected_ft.ft_params;
          }
        in
        (* Don't bother passing in `void` if there is no explicit return *)
        let ret_ty =
          match expected_ft.ft_ret.et_type with
          | (_, Tprim Tvoid) when not is_explicit_ret -> None
          | _ -> Some expected_ft.ft_ret.et_type
        in
        Typing_log.increment_feature_count env FL.Lambda.contextual_params;
        check_body_under_known_params env ?ret_ty expected_ft
      | _ ->
        let explicit_variadic_param_or_non_variadic =
          match f.f_variadic with
          | FVvariadicArg { param_type_hint; _ } ->
            Option.is_some (hint_of_type_hint param_type_hint)
          | FVellipsis _ -> false
          | _ -> true
        in
        (* If all parameters are annotated with explicit types, then type-check
         * the body under those assumptions and pick up the result type *)
        let all_explicit_params =
          List.for_all f.f_params (fun param ->
              Option.is_some (hint_of_type_hint param.param_type_hint))
        in
        if all_explicit_params && explicit_variadic_param_or_non_variadic then (
          Typing_log.increment_feature_count
            env
            ( if List.is_empty f.f_params then
              FL.Lambda.no_params
            else
              FL.Lambda.explicit_params );
          check_body_under_known_params env declared_ft
        ) else (
          match expected with
          | Some ExpectedTy.{ ty = { et_type = (_, Tany _); _ }; _ } ->
            (* If the expected type is Tany env then we're passing a lambda to an untyped
             * function and we just assume every parameter has type Tany env *)
            Typing_log.increment_feature_count env FL.Lambda.untyped_context;
            check_body_under_known_params env declared_ft
          | Some _ ->
            (* If the expected type is something concrete but not a function
             * then we should reject in strict mode. Check body anyway *)
            if Partial.should_check_error (Env.get_mode env) 4224 then
              Errors.untyped_lambda_strict_mode p;
            Typing_log.increment_feature_count
              env
              FL.Lambda.non_function_typed_context;
            check_body_under_known_params env declared_ft
          | _ ->
            (* If we're in partial mode then type-check definition anyway,
             * so treating parameters without type hints as "untyped"
             *)
            if not (Env.is_strict env) then (
              Typing_log.increment_feature_count
                env
                FL.Lambda.non_strict_unknown_params;
              check_body_under_known_params env declared_ft
            ) else if
                (* If new_inference_lambda is enabled, check lambda using constraints *)
                TypecheckerOptions.new_inference_lambda (Env.get_tcopt env)
              then (
              Typing_log.increment_feature_count
                env
                FL.Lambda.fresh_tyvar_params;

              (* Replace uses of Tany that originated from "untyped" parameters or return type
               * with fresh type variables *)
              let freshen_ftype env ft =
                let freshen_ty env pos et =
                  match snd et.et_type with
                  | Tany _ ->
                    let (env, ty) = Env.fresh_type env pos in
                    (env, { et with et_type = ty })
                  | Tclass (id, e, [(_, Tany _)])
                    when String.equal (snd id) SN.Classes.cAwaitable ->
                    let (env, t) = Env.fresh_type env pos in
                    ( env,
                      {
                        et with
                        et_type = (fst et.et_type, Tclass (id, e, [t]));
                      } )
                  | _ -> (env, et)
                in
                let freshen_untyped_param env ft_param =
                  let (env, fp_type) =
                    freshen_ty env ft_param.fp_pos ft_param.fp_type
                  in
                  (env, { ft_param with fp_type })
                in
                let (env, ft_params) =
                  List.map_env env ft.ft_params freshen_untyped_param
                in
                let (env, ft_ret) = freshen_ty env declared_pos ft.ft_ret in
                (env, { ft with ft_params; ft_ret })
              in
              let (env, declared_ft) = freshen_ftype env declared_ft in
              let env =
                Env.set_tyvar_variance env (Reason.Rnone, Tfun declared_ft)
              in
              check_body_under_known_params
                env
                ~ret_ty:declared_ft.ft_ret.et_type
                declared_ft
              (* Legacy lambda inference *)
            ) else (
              Typing_log.increment_feature_count env FL.Lambda.unknown_params;

              (* check for recursive function calls *)
              let reactivity =
                fun_reactivity env.decl_env f.f_user_attributes f.f_params
              in
              let old_reactivity = env_reactivity env in
              let env = Env.set_env_reactive env reactivity in
              let (is_coroutine, counter, pos, anon) =
                anon_make env p f declared_ft idl is_anon outer
              in
              let (env, tefun, _, anon_id) =
                Errors.try_with_error
                  (fun () ->
                    let (_, tefun, ty) =
                      anon env declared_ft.ft_params declared_ft.ft_arity
                    in
                    let anon_fun =
                      {
                        rx = reactivity;
                        typecheck = anon;
                        is_coroutine;
                        counter;
                        pos;
                      }
                    in
                    let (env, anon_id) = Env.add_anonymous env anon_fun in
                    (env, tefun, ty, anon_id))
                  (fun () ->
                    (* If the anonymous function declaration has errors itself, silence
                     them in any subsequent usages. *)
                    let anon_ign ?el:_ ?ret_ty:_ env fun_params =
                      Errors.ignore_ (fun () -> anon env fun_params)
                    in
                    let (_, tefun, ty) =
                      anon_ign env declared_ft.ft_params declared_ft.ft_arity
                    in
                    let anon_fun =
                      {
                        rx = reactivity;
                        typecheck = anon;
                        is_coroutine;
                        counter;
                        pos;
                      }
                    in
                    let (env, anon_id) = Env.add_anonymous env anon_fun in
                    (env, tefun, ty, anon_id))
              in
              let env = Env.set_env_reactive env old_reactivity in
              let anon_ty =
                (Reason.Rwitness p, Tanon (declared_ft.ft_arity, anon_id))
              in
              let ((ep, _efun_ty), efun) = tefun in
              let tefun = ((ep, anon_ty), efun) in
              (env, tefun, anon_ty)
            )
        )
    end
  | Xml (sid, attrl, el) ->
    let cid = CI sid in
    let (env, _tal, _te, classes) =
      class_id_for_new ~exact:Nonexact p env cid []
    in
    let class_info =
      match classes with
      | [] -> None
      (* OK to ignore rest of list; class_info only used for errors, and
       * cid = CI sid cannot produce a union of classes anyhow *)
      | (_, class_info, _) :: _ -> Some class_info
    in
    let (env, _te, obj) =
      expr env (fst sid, New ((fst sid, cid), [], [], None, fst sid))
    in
    let (env, typed_attrs, attr_types) =
      xhp_attribute_exprs env class_info attrl
    in
    let (env, tel) =
      List.map_env env el ~f:(fun env e ->
          let (env, te, _) = expr env e in
          (env, te))
    in
    let txml = Aast.Xml (sid, typed_attrs, List.rev tel) in
    (match class_info with
    | None -> make_result env p txml (Reason.Runknown_class p, Tobject)
    | Some class_info ->
      let env =
        List.fold_left
          attr_types
          ~f:
            begin
              fun env attr ->
              let (namepstr, valpty) = attr in
              let (valp, valty) = valpty in
              let (env, (declty, _tal)) =
                TOG.obj_get
                  ~obj_pos:(fst sid)
                  ~is_method:false
                  ~nullsafe:None
                  ~coerce_from_ty:None
                  env
                  obj
                  cid
                  namepstr
                  Errors.unify_error
              in
              let ureason = Reason.URxhp (Cls.name class_info, snd namepstr) in
              Typing_coercion.coerce_type
                valp
                ureason
                env
                valty
                (MakeType.unenforced declty)
                Errors.xhp_attribute_does_not_match_hint
            end
          ~init:env
      in
      make_result env p txml obj)
  | Callconv (kind, e) ->
    let (env, te, ty) = expr env e in
    make_result env p (Aast.Callconv (kind, te)) ty
  | Shape fdm ->
    let (env, fdm_with_expected) =
      match expand_expected env expected with
      | (env, Some (pos, ur, (_, Tshape (_, expected_fdm)))) ->
        let fdme =
          List.map
            ~f:(fun (k, v) ->
              match ShapeMap.find_opt k expected_fdm with
              | None -> (k, (v, None))
              | Some sft -> (k, (v, Some (ExpectedTy.make pos ur sft.sft_ty))))
            fdm
        in
        (env, fdme)
      | _ -> (env, List.map ~f:(fun (k, v) -> (k, (v, None))) fdm)
    in
    (* allow_inter adds a type-variable *)
    let (env, tfdm) =
      List.map_env
        ~f:(fun env (key, (e, expected)) ->
          let (env, te, ty) = expr ?expected env e in
          (env, (key, (te, ty))))
        env
        fdm_with_expected
    in
    let (env, fdm) =
      let convert_expr_and_type_to_shape_field_type env (key, (_, ty)) =
        (* An expression evaluation always corresponds to a shape_field_type
             with sft_optional = false. *)
        (env, (key, { sft_optional = false; sft_ty = ty }))
      in
      List.map_env ~f:convert_expr_and_type_to_shape_field_type env tfdm
    in
    let fdm =
      List.fold_left
        ~f:(fun acc (k, v) -> ShapeMap.add k v acc)
        ~init:ShapeMap.empty
        fdm
    in
    let env = check_shape_keys_validity env p (ShapeMap.keys fdm) in
    (* Fields are fully known, because this shape is constructed
     * using shape keyword and we know exactly what fields are set. *)
    make_result
      env
      p
      (Aast.Shape (List.map ~f:(fun (k, (te, _)) -> (k, te)) tfdm))
      (Reason.Rwitness p, Tshape (Closed_shape, fdm))
  | PU_atom s ->
    make_result env p (Aast.PU_atom s) (Reason.Rwitness p, Tprim (Tatom s))
  | PU_identifier (_, (_, enum), (_, atom)) ->
    (* TODO(T36532263): Pocket Universes *)
    let s = enum ^ ":@" ^ atom in
    Errors.pu_typing p "identifier" s;

    expr_error env (Reason.Rwitness p) outer

(* let ty = (Reason.Rwitness cst_pos, Typing_utils.terr env) in *)
and class_const ?(incl_tc = false) env p ((cpos, cid), mid) =
  let (env, _tal, ce, cty) =
    static_class_id ~check_constraints:true cpos env [] cid
  in
  let (env, (const_ty, _tal)) =
    class_get
      ~is_method:false
      ~is_const:true
      ~incl_tc
      ~coerce_from_ty:None
      env
      cty
      mid
      cid
  in
  make_result env p (Aast.Class_const (ce, mid)) const_ty
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
  let (env, te, valty) = expr env valexpr in
  (* Build the typed attribute node *)
  let typed_attr = Aast.Xhp_spread te in
  let (env, attr_ptys) =
    match c_onto with
    | None -> (env, [])
    | Some class_info -> Typing_xhp.get_spread_attributes env p class_info valty
  in
  (env, typed_attr, attr_ptys)

(**
 * Simple XHP attributes (attr={expr} form) are simply interpreted as a member
 * variable prefixed with a colon, the types of which will be validated later
 *)
and xhp_simple_attribute env id valexpr =
  let (p, _) = valexpr in
  let (env, te, valty) = expr env valexpr in
  (* This converts the attribute name to a member name. *)
  let name = ":" ^ snd id in
  let attr_pty = ((fst id, name), (p, valty)) in
  let typed_attr = Aast.Xhp_simple (id, te) in
  (env, typed_attr, [attr_pty])

(**
 * Typecheck the attribute expressions - this just checks that the expressions are
 * valid, not that they match the declared type for the attribute and,
 * in case of spreads, makes sure they are XHP.
 *)
and xhp_attribute_exprs env cid attrl =
  let handle_attr (env, typed_attrl, attr_ptyl) attr =
    let (env, typed_attr, attr_ptys) =
      match attr with
      | Xhp_simple (id, valexpr) -> xhp_simple_attribute env id valexpr
      | Xhp_spread valexpr -> xhp_spread_attribute env cid valexpr
    in
    (env, typed_attr :: typed_attrl, attr_ptys @ attr_ptyl)
  in
  let (env, typed_attrl, attr_ptyl) =
    List.fold_left ~f:handle_attr ~init:(env, [], []) attrl
  in
  (env, List.rev typed_attrl, List.rev attr_ptyl)

(*****************************************************************************)
(* Anonymous functions. *)
(*****************************************************************************)
and anon_bind_param params (env, t_params) ty : env * Tast.fun_param list =
  match !params with
  | [] ->
    (* This code cannot be executed normally, because the arity is wrong
     * and it will error later. Bind as many parameters as we can and carry
     * on. *)
    (env, t_params)
  | param :: paraml ->
    params := paraml;
    (match hint_of_type_hint param.param_type_hint with
    | Some h ->
      let h = Decl_hint.hint env.decl_env h in
      (* When creating a closure, the 'this' type will mean the
       * late bound type of the current enclosing class
       *)
      let ety_env =
        { (Phase.env_with_self env) with from_class = Some CIstatic }
      in
      let (env, h) = Phase.localize ~ety_env env h in
      let pos = Reason.to_pos (fst ty) in
      let env =
        Typing_coercion.coerce_type
          pos
          Reason.URparam
          env
          ty
          (MakeType.unenforced h)
          Errors.unify_error
      in
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
      let (env, t_param) = bind_param env (h, param) in
      (env, t_params @ [t_param])
    | None ->
      let ty = (Reason.Rlambda_param (param.param_pos, fst ty), snd ty) in
      let (env, t_param) = bind_param env (ty, param) in
      (env, t_params @ [t_param]))

and anon_bind_variadic env vparam variadic_ty =
  let (env, ty, pos) =
    match hint_of_type_hint vparam.param_type_hint with
    | None ->
      (* if the hint is missing, use the type we expect *)
      (env, variadic_ty, Reason.to_pos (fst variadic_ty))
    | Some hint ->
      let h = Decl_hint.hint env.decl_env hint in
      let ety_env =
        { (Phase.env_with_self env) with from_class = Some CIstatic }
      in
      let (env, h) = Phase.localize ~ety_env env h in
      let pos = Reason.to_pos (fst variadic_ty) in
      let env =
        Typing_coercion.coerce_type
          pos
          Reason.URparam
          env
          variadic_ty
          (MakeType.unenforced h)
          Errors.unify_error
      in
      (env, h, vparam.param_pos)
  in
  let r = Reason.Rvar_param pos in
  let arr_values = (r, snd ty) in
  let ty = (r, Tarraykind (AKvarray arr_values)) in
  let (env, t_variadic) = bind_param env (ty, vparam) in
  (env, t_variadic)

and anon_bind_opt_param env param : env =
  match param.param_expr with
  | None ->
    let ty = (Reason.Rwitness param.param_pos, Typing_utils.tany env) in
    let (env, _) = bind_param env (ty, param) in
    env
  | Some default ->
    let (env, _te, ty) = expr env default in
    Typing_sequencing.sequence_check_expr default;
    let (env, _) = bind_param env (ty, param) in
    env

and anon_check_param env param =
  match hint_of_type_hint param.param_type_hint with
  | None -> env
  | Some hty ->
    let (env, hty) = Phase.localize_hint_with_self env hty in
    let paramty = Env.get_local env (Local_id.make_unscoped param.param_name) in
    let hint_pos = Reason.to_pos (fst hty) in
    let env =
      Typing_coercion.coerce_type
        hint_pos
        Reason.URhint
        env
        paramty
        (MakeType.unenforced hty)
        Errors.unify_error
    in
    env

and stash_conts_for_anon env p is_anon captured f =
  let captured =
    if Env.is_local_defined env this then
      (Pos.none, this) :: captured
    else
      captured
  in
  let init =
    Option.map (Env.next_cont_opt env) ~f:(fun next_cont ->
        let initial_locals =
          if is_anon then
            Env.get_locals env captured
          else
            next_cont.Typing_per_cont_env.local_types
        in
        let initial_fakes =
          Fake.forget (Env.get_fake_members env) (Fake.Blame_lambda p)
        in
        let tpenv = Env.get_tpenv env in
        (initial_locals, initial_fakes, tpenv))
  in
  let (env, (tfun, result)) =
    Typing_lenv.stash_and_do env (Env.all_continuations env) (fun env ->
        let env =
          match init with
          | None -> env
          | Some (initial_locals, initial_fakes, tpenv) ->
            let env = Env.reinitialize_locals env in
            let env = Env.set_locals env initial_locals in
            let env = Env.set_fake_members env initial_fakes in
            let env = Env.env_with_tpenv env tpenv in
            env
        in
        let (env, tfun, result) = f env in
        (env, (tfun, result)))
  in
  (env, tfun, result)

(* Make a type-checking function for an anonymous function. *)
and anon_make tenv p f ft idl is_anon outer =
  let anon_lenv = tenv.lenv in
  let is_typing_self = ref false in
  let nb = Nast.assert_named_body f.f_body in
  let is_coroutine = Ast_defs.(equal_fun_kind f.f_fun_kind FCoroutine) in
  ( is_coroutine,
    ref ([], []),
    p,
    (* Here ret_ty should include Awaitable wrapper *)
    fun ?el ?ret_ty env supplied_params supplied_arity ->
      if !is_typing_self then (
        Errors.anonymous_recursive p;
        expr_error env (Reason.Rwitness p) outer
      ) else (
        is_typing_self := true;
        Env.anon anon_lenv env (fun env ->
            stash_conts_for_anon env p is_anon idl (fun env ->
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
                    let remaining_params =
                      List.drop supplied_params (List.length f.f_params)
                    in
                    List.map
                      ~f:(fun param -> param.fp_type.et_type)
                      remaining_params
                  in
                  let r = Reason.Rvar_param varg.param_pos in
                  let union = Tunion (tyl @ remaining_types) in
                  let (env, t_param) = anon_bind_variadic env varg (r, union) in
                  (env, Aast.FVvariadicArg t_param)
                in
                let (env, t_variadic) =
                  match (f.f_variadic, supplied_arity) with
                  | (FVvariadicArg arg, Fvariadic (_, variadic)) ->
                    make_variadic_arg env arg [variadic.fp_type.et_type]
                  | (FVvariadicArg arg, Fstandard _) ->
                    make_variadic_arg env arg []
                  | (FVellipsis pos, _) -> (env, Aast.FVellipsis pos)
                  | (_, _) -> (env, Aast.FVnonVariadic)
                in
                let params = ref f.f_params in
                let (env, t_params) =
                  List.fold_left
                    ~f:(anon_bind_param params)
                    ~init:(env, [])
                    (List.map supplied_params (fun x -> x.fp_type.et_type))
                in
                let env =
                  List.fold_left ~f:anon_bind_opt_param ~init:env !params
                in
                let env =
                  List.fold_left ~f:anon_check_param ~init:env f.f_params
                in
                let env =
                  match el with
                  | None ->
                    (*iter2_shortest
                      Unify.unify_param_modes
                      ft.ft_params
                      supplied_params; *)
                    env
                  | Some x ->
                    let var_param =
                      match f.f_variadic with
                      | FVellipsis pos ->
                        let param =
                          TUtils.default_fun_param
                            ~pos
                            (Reason.Rvar_param pos, Typing_defs.make_tany ())
                        in
                        Some param
                      | _ -> None
                    in
                    let rec iter l1 l2 =
                      match (l1, l2, var_param) with
                      | (_, [], _) -> ()
                      | ([], _, None) -> ()
                      | ([], x2 :: rl2, Some def1) ->
                        param_modes ~is_variadic:true def1 x2;
                        iter [] rl2
                      | (x1 :: rl1, x2 :: rl2, _) ->
                        param_modes x1 x2;
                        iter rl1 rl2
                    in
                    iter ft.ft_params x;
                    wfold_left2 inout_write_back env ft.ft_params x
                in
                let env = Env.set_fn_kind env f.f_fun_kind in
                let decl_ty =
                  Option.map
                    ~f:(Decl_hint.hint env.decl_env)
                    (hint_of_type_hint f.f_ret)
                in
                let (env, hret) =
                  match decl_ty with
                  | None ->
                    (* Do we have a contextual return type? *)
                    begin
                      match ret_ty with
                      | None ->
                        let (env, ret_ty) = Env.fresh_type env p in
                        (env, Typing_return.wrap_awaitable env p ret_ty)
                      | Some ret_ty ->
                        (* We might need to force it to be Awaitable if it is a type variable *)
                        Typing_return.force_awaitable env p ret_ty
                    end
                  | Some ret ->
                    (* If a 'this' type appears it needs to be compatible with the
                     * late static type
                     *)
                    let ety_env =
                      {
                        (Phase.env_with_self env) with
                        from_class = Some CIstatic;
                      }
                    in
                    Typing_return.make_return_type
                      (Phase.localize ~ety_env)
                      env
                      ret
                in
                let env =
                  Env.set_return
                    env
                    (Typing_return.make_info
                       f.f_fun_kind
                       []
                       env
                       ~is_explicit:(Option.is_some ret_ty)
                       hret
                       decl_ty)
                in
                let local_tpenv = Env.get_tpenv env in
                (* Outer pipe variables aren't available in closures. Note that
                 * locals are restored by Env.anon after processing the closure
                 *)
                let env =
                  Env.unset_local
                    env
                    (Local_id.make_unscoped SN.SpecialIdents.dollardollar)
                in
                let (env, tb) = block env nb.fb_ast in
                let implicit_return = LEnv.has_next env in
                let env =
                  if (not implicit_return) || Nast.named_body_is_unsafe nb then
                    env
                  else
                    fun_implicit_return env p hret f.f_fun_kind
                in
                is_typing_self := false;
                let annotation =
                  if Nast.named_body_is_unsafe nb then
                    Tast.HasUnsafeBlocks
                  else
                    Tast.NoUnsafeBlocks
                in
                let (env, tparams) = List.map_env env f.f_tparams type_param in
                let (env, user_attributes) =
                  List.map_env env f.f_user_attributes user_attribute
                in
                let tfun_ =
                  {
                    Aast.f_annotation = Env.save local_tpenv env;
                    Aast.f_span = f.f_span;
                    Aast.f_mode = f.f_mode;
                    Aast.f_ret = (hret, hint_of_type_hint f.f_ret);
                    Aast.f_name = f.f_name;
                    Aast.f_tparams = tparams;
                    Aast.f_where_constraints = f.f_where_constraints;
                    Aast.f_fun_kind = f.f_fun_kind;
                    Aast.f_file_attributes = [];
                    Aast.f_user_attributes = user_attributes;
                    Aast.f_body =
                      { Aast.fb_ast = tb; fb_annotation = annotation };
                    Aast.f_params = t_params;
                    Aast.f_variadic = t_variadic;
                    (* TODO TAST: Variadic efuns *)
                    Aast.f_external = f.f_external;
                    Aast.f_namespace = f.f_namespace;
                    Aast.f_doc_comment = f.f_doc_comment;
                    Aast.f_static = f.f_static;
                  }
                in
                let ty = (Reason.Rwitness p, Tfun ft) in
                let te =
                  if is_anon then
                    Tast.make_typed_expr p ty (Aast.Efun (tfun_, idl))
                  else
                    Tast.make_typed_expr p ty (Aast.Lfun (tfun_, idl))
                in
                let env = Env.set_tyvar_variance env ty in
                (env, te, hret)))
        (* stash_conts_for_anon *)
        (* Env.anon *)
      ) )

(*****************************************************************************)
(* End of anonymous functions. *)
(*****************************************************************************)
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
and expand_expected env (expected : ExpectedTy.t option) =
  match expected with
  | None -> (env, None)
  | Some ExpectedTy.{ pos = p; reason = ur; ty = { et_type = ty; _ }; _ } ->
    let (env, ty) = Env.expand_type env ty in
    (match ty with
    | (_, Tunion [ty]) -> (env, Some (p, ur, ty))
    | (_, Toption ty) -> (env, Some (p, ur, ty))
    | _ -> (env, Some (p, ur, ty)))

(* Do a subtype check of inferred type against expected type *)
and check_expected_ty message env inferred_ty (expected : ExpectedTy.t option) =
  match expected with
  | None -> env
  | Some ExpectedTy.{ pos = p; reason = ur; ty } ->
    Typing_log.(
      log_with_level env "typing" 1 (fun () ->
          log_types
            p
            env
            [
              Log_head
                ( Printf.sprintf
                    "Typing.check_expected_ty %s enforced=%b"
                    message
                    ty.et_enforced,
                  [
                    Log_type ("inferred_ty", inferred_ty);
                    Log_type ("expected_ty", ty.et_type);
                  ] );
            ]));
    Typing_coercion.coerce_type p ur env inferred_ty ty Errors.unify_error

and new_object
    ~(expected : ExpectedTy.t option)
    ~check_parent
    ~check_not_abstract
    ~is_using_clause
    p
    env
    cid
    explicit_targs
    el
    unpacked_element =
  (* Obtain class info from the cid expression. We get multiple
   * results with a CIexpr that has a union type *)
  let (env, tal, tcid, classes) =
    instantiable_cid ~exact:Exact p env cid explicit_targs
  in
  let allow_abstract_bound_generic =
    match tcid with
    | ((_, (_, Tgeneric tt)), Aast.CI (_, tn)) -> String.equal tt tn
    | _ -> false
  in
  let finish env tcid tel typed_unpack_element ty ctor_fty =
    let (env, new_ty) =
      let ((_, cid_ty), _) = tcid in
      let (env, cid_ty) = Env.expand_type env cid_ty in
      match cid_ty with
      | (_, Tgeneric _) -> (env, cid_ty)
      | _ ->
        if check_parent then
          (env, ty)
        else
          ExprDepTy.make env cid ty
    in
    (env, tcid, tal, tel, typed_unpack_element, new_ty, ctor_fty)
  in
  let rec gather env tel typed_unpack_element res classes =
    match classes with
    | [] ->
      begin
        match res with
        | [] ->
          let (env, tel, _) = exprs env el in
          let (env, typed_unpack_element, _) =
            match unpacked_element with
            | None -> (env, None, MakeType.nothing Reason.Rnone)
            | Some unpacked_element ->
              let (env, e, ty) = expr env unpacked_element in
              (env, Some e, ty)
          in
          let r = Reason.Runknown_class p in
          finish
            env
            tcid
            tel
            typed_unpack_element
            (r, Tobject)
            (r, TUtils.terr env)
        | [(ty, ctor_fty)] ->
          finish env tcid tel typed_unpack_element ty ctor_fty
        | l ->
          let (tyl, ctyl) = List.unzip l in
          let r = Reason.Rwitness p in
          finish
            env
            tcid
            tel
            typed_unpack_element
            (r, Tunion tyl)
            (r, Tunion ctyl)
      end
    | (cname, class_info, c_ty) :: classes ->
      if
        check_not_abstract
        && Cls.abstract class_info
        && (not (requires_consistent_construct cid))
        && not allow_abstract_bound_generic
      then
        uninstantiable_error
          env
          p
          cid
          (Cls.pos class_info)
          (Cls.name class_info)
          p
          c_ty;
      let (env, obj_ty_, params) =
        let (env, c_ty) = Env.expand_type env c_ty in
        match (cid, tal, snd c_ty) with
        (* Explicit type arguments *)
        | (CI _, _ :: _, Tclass (_, _, tyl)) -> (env, snd c_ty, tyl)
        | _ ->
          let (env, params) =
            List.map_env env (Cls.tparams class_info) (fun env tparam ->
                let (env, tvar) =
                  Env.fresh_type_reason
                    env
                    (Reason.Rtype_variable_generics
                       (p, snd tparam.tp_name, strip_ns (snd cname)))
                in
                Typing_log.log_new_tvar_for_new_object env p tvar cname tparam;
                (env, tvar))
          in
          begin
            match snd c_ty with
            | Tclass (_, Exact, _) ->
              (env, Tclass (cname, Exact, params), params)
            | _ -> (env, Tclass (cname, Nonexact, params), params)
          end
      in
      if
        (not check_parent)
        && (not is_using_clause)
        && Cls.is_disposable class_info
      then
        Errors.invalid_new_disposable p;
      let r_witness = Reason.Rwitness p in
      let obj_ty = (r_witness, obj_ty_) in
      let c_ty =
        match cid with
        | CIstatic -> (r_witness, TUtils.this_of obj_ty)
        | CIexpr _ -> (r_witness, snd c_ty)
        | _ -> obj_ty
      in
      let (env, new_ty) =
        let ((_, cid_ty), _) = tcid in
        let (env, cid_ty) = Env.expand_type env cid_ty in
        match cid_ty with
        | (_, Tgeneric _) -> (env, cid_ty)
        | _ ->
          if check_parent then
            (env, c_ty)
          else
            ExprDepTy.make env cid c_ty
      in
      (* Set variance according to type of `new` expression now. Lambda arguments
       * to the constructor might depend on it, and `call_construct` only uses
       * `ctor_fty` to set the variance which has void return type *)
      let env = Env.set_tyvar_variance env new_ty in
      let (env, _tcid, tel, typed_unpack_element, ctor_fty) =
        let env = check_expected_ty "New" env new_ty expected in
        call_construct p env class_info params el unpacked_element cid
      in
      ( if equal_consistent_kind (snd (Cls.construct class_info)) Inconsistent
      then
        match cid with
        | CIstatic -> Errors.new_inconsistent_construct p cname `static
        | CIexpr _ -> Errors.new_inconsistent_construct p cname `classname
        | _ -> () );
      (match cid with
      | CIparent ->
        let (env, ctor_fty) =
          match fst (Cls.construct class_info) with
          | Some { ce_type = (lazy ty); ce_abstract; _ } ->
            let ety_env =
              {
                type_expansions = [];
                substs = Subst.make_locl (Cls.tparams class_info) params;
                this_ty = obj_ty;
                from_class = None;
                quiet = false;
                on_error = Errors.unify_error_at p;
              }
            in
            if ce_abstract then
              Errors.parent_abstract_call
                SN.Members.__construct
                p
                (Reason.to_pos (fst ctor_fty));
            let (env, ctor_fty) = Phase.localize ~ety_env env ty in
            (env, ctor_fty)
          | None -> (env, ctor_fty)
        in
        gather env tel typed_unpack_element ((obj_ty, ctor_fty) :: res) classes
      | CIstatic
      | CI _
      | CIself ->
        gather env tel typed_unpack_element ((c_ty, ctor_fty) :: res) classes
      | CIexpr _ ->
        (* When constructing from a (classname) variable, the variable
         * dictates what the constructed object is going to be. This allows
         * for generic and dependent types to be correctly carried
         * through the 'new $foo()' iff the constructed obj_ty is a
         * supertype of the variable-dictated c_ty *)
        let env =
          Typing_ops.sub_type p Reason.URnone env c_ty obj_ty Errors.unify_error
        in
        gather env tel typed_unpack_element ((c_ty, ctor_fty) :: res) classes)
  in
  gather env [] None [] classes

(* FIXME: we need to separate our instantiability into two parts. Currently,
 * all this function is doing is checking if a given type is inhabited --
 * that is, whether there are runtime values of type Aast. However,
 * instantiability should be the stricter notion that T has a runtime
 * constructor; that is, `new T()` should be valid. In particular, interfaces
 * are inhabited, but not instantiable.
 * To make this work with classname, we likely need to add something like
 * concrete_classname<T>, where T cannot be an interface.
 * *)
and instantiable_cid ?(exact = Nonexact) p env cid explicit_targs =
  let (env, tal, te, classes) =
    class_id_for_new ~exact p env cid explicit_targs
  in
  List.iter classes (fun ((pos, name), class_info, c_ty) ->
      if
        Ast_defs.(equal_class_kind (Cls.kind class_info) Ctrait)
        || Ast_defs.(equal_class_kind (Cls.kind class_info) Cenum)
      then
        match cid with
        | CIexpr _
        | CI _ ->
          uninstantiable_error env p cid (Cls.pos class_info) name pos c_ty
        | CIstatic
        | CIparent
        | CIself ->
          ()
      else if
        Ast_defs.(equal_class_kind (Cls.kind class_info) Cabstract)
        && Cls.final class_info
      then
        uninstantiable_error env p cid (Cls.pos class_info) name pos c_ty
      else
        ());
  (env, tal, te, classes)

and uninstantiable_error env reason_pos cid c_tc_pos c_name c_usage_pos c_ty =
  let reason_msgl =
    match cid with
    | CIexpr _ ->
      let ty_str = "This would be " ^ Typing_print.error env c_ty in
      [(reason_pos, ty_str)]
    | _ -> []
  in
  Errors.uninstantiable_class c_usage_pos c_tc_pos c_name reason_msgl

and exception_ty pos env ty =
  let exn_ty = MakeType.throwable (Reason.Rthrow pos) in
  Typing_coercion.coerce_type
    pos
    Reason.URthrow
    env
    ty
    { et_type = exn_ty; et_enforced = false }
    Errors.unify_error

and shape_field_pos = function
  | Ast_defs.SFlit_int (p, _)
  | Ast_defs.SFlit_str (p, _) ->
    p
  | Ast_defs.SFclass_const ((cls_pos, _), (mem_pos, _)) ->
    Pos.btw cls_pos mem_pos

and check_shape_keys_validity env pos keys =
  (* If the key is a class constant, get its class name and type. *)
  let get_field_info env key =
    let key_pos = shape_field_pos key in
    (* Empty strings or literals that start with numbers are not
         permitted as shape field names. *)
    match key with
    | Ast_defs.SFlit_int _ -> (env, key_pos, None)
    | Ast_defs.SFlit_str (_, key_name) ->
      if Int.equal 0 (String.length key_name) then
        Errors.invalid_shape_field_name_empty key_pos;
      (env, key_pos, None)
    | Ast_defs.SFclass_const (((p, cls) as x), y) ->
      let (env, _te, ty) = class_const env pos ((p, CI x), y) in
      let env =
        Typing_enum.check_valid_array_key_type
          Errors.invalid_shape_field_type
          ~allow_any:false
          env
          key_pos
          ty
      in
      (env, key_pos, Some (cls, ty))
  in
  let check_field witness_pos witness_info env key =
    let (env, key_pos, key_info) = get_field_info env key in
    match (witness_info, key_info) with
    | (Some _, None) ->
      Errors.invalid_shape_field_literal key_pos witness_pos;
      env
    | (None, Some _) ->
      Errors.invalid_shape_field_const key_pos witness_pos;
      env
    | (None, None) -> env
    | (Some (cls1, ty1), Some (cls2, ty2)) ->
      if String.( <> ) cls1 cls2 then
        Errors.shape_field_class_mismatch
          key_pos
          witness_pos
          (strip_ns cls2)
          (strip_ns cls1);
      if
        not
          ( Typing_solver.is_sub_type env ty1 ty2
          && Typing_solver.is_sub_type env ty2 ty1 )
      then
        Errors.shape_field_type_mismatch
          key_pos
          witness_pos
          (Typing_print.error env ty2)
          (Typing_print.error env ty1);
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
    let (env, pos, info) = get_field_info env witness in
    List.fold_left ~f:(check_field pos info) ~init:env rest_keys

and set_valid_rvalue p env x ty =
  let env = set_local env (p, x) ty in
  (* We are assigning a new value to the local variable, so we need to
   * generate a new expression id
   *)
  Env.set_local_expr_id env x (Ident.tmp ())

(* Produce an error if assignment is used in the scope of the |> operator, i.e.
 * if $$ is in scope *)
and error_if_assign_in_pipe p env =
  let dd_var = Local_id.make_unscoped SN.SpecialIdents.dollardollar in
  let dd_defined = Env.is_local_defined env dd_var in
  if dd_defined then
    Errors.unimplemented_feature p "Assignment within pipe expressions"

(* Deal with assignment of a value of type ty2 to lvalue e1 *)
and assign p env e1 ty2 : _ * Tast.expr * Tast.ty =
  error_if_assign_in_pipe p env;
  assign_ p Reason.URassign env e1 ty2

and is_hack_collection env ty =
  Typing_solver.is_sub_type
    env
    ty
    (MakeType.const_collection Reason.Rnone (MakeType.mixed Reason.Rnone))

and assign_ p ur env e1 ty2 =
  match e1 with
  | (_, Lvar ((_, x) as id)) ->
    let env = set_valid_rvalue p env x ty2 in
    make_result env (fst e1) (Aast.Lvar id) ty2
  | (_, Lplaceholder id) ->
    let placeholder_ty = MakeType.void (Reason.Rplaceholder p) in
    make_result env (fst e1) (Aast.Lplaceholder id) placeholder_ty
  | (_, List el) ->
    let (env, tyl) =
      List.map_env env el ~f:(fun env _ ->
          Env.fresh_type env (Reason.to_pos (fst ty2)))
    in
    let destructure_ty =
      ConstraintType
        (Reason.Rdestructure (fst e1, List.length tyl), Tdestructure tyl)
    in
    let lty2 = LoclType ty2 in
    let env = Type.sub_type_i p ur env lty2 destructure_ty Errors.unify_error in
    let env = Env.set_tyvar_variance_i env destructure_ty in
    let (env, reversed_tel) =
      List.fold2_exn el tyl ~init:(env, []) ~f:(fun (env, tel) lvalue ty2 ->
          let (env, te, _) = assign p env lvalue ty2 in
          (env, te :: tel))
    in
    make_result env (fst e1) (Aast.List (List.rev reversed_tel)) ty2
  | (pobj, Obj_get (obj, (pm, Id ((_, member_name) as m)), nullflavor)) ->
    let lenv = env.lenv in
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
    let nullsafe =
      match nullflavor with
      | OG_nullthrows -> None
      | OG_nullsafe -> Some pobj
    in
    let (env, tobj, obj_ty) = expr ~accept_using_var:true no_fakes obj in
    let env = might_throw env in
    let (env, (result, _tal)) =
      TOG.obj_get
        ~obj_pos:(fst obj)
        ~is_method:false
        ~nullsafe
        ~coerce_from_ty:(Some (p, ur, ty2))
        env
        obj_ty
        (CIexpr e1)
        m
        Errors.unify_error
    in
    let te1 =
      Tast.make_typed_expr
        pobj
        result
        (Aast.Obj_get
           (tobj, Tast.make_typed_expr pm result (Aast.Id m), nullflavor))
    in
    let env = { env with lenv } in
    begin
      match obj with
      | (_, This) ->
        let (env, local) = Env.FakeMembers.make env obj member_name in
        let env = set_valid_rvalue p env local ty2 in
        (env, te1, ty2)
      | (_, Lvar _) ->
        let (env, local) = Env.FakeMembers.make env obj member_name in
        let env = set_valid_rvalue p env local ty2 in
        (env, te1, ty2)
      | _ -> (env, te1, ty2)
    end
  | (_, Obj_get _) ->
    let lenv = env.lenv in
    let no_fakes = LEnv.env_with_empty_fakes env in
    let (env, te1, real_type) = lvalue no_fakes e1 in
    let (env, exp_real_type) = Env.expand_type env real_type in
    let env = { env with lenv } in
    let env =
      Typing_coercion.coerce_type
        p
        ur
        env
        ty2
        (MakeType.unenforced exp_real_type)
        Errors.unify_error
    in
    (env, te1, ty2)
  | (_, Class_get (_, CGexpr _)) ->
    failwith "AST should not have any CGexprs after naming"
  | (_, Class_get ((pos_classid, x), CGstring (pos_member, y))) ->
    let lenv = env.lenv in
    let no_fakes = LEnv.env_with_empty_fakes env in
    let (env, te1, _) = lvalue no_fakes e1 in
    let env = { env with lenv } in
    let (env, ety2) = Env.expand_type env ty2 in
    (* This defers the coercion check to class_get, which looks up the appropriate target type *)
    let (env, _tal, _, cty) =
      static_class_id ~check_constraints:false pos_classid env [] x
    in
    let env = might_throw env in
    let (env, _) =
      class_get
        ~is_method:false
        ~is_const:false
        ~coerce_from_ty:(Some (p, ur, ety2))
        env
        cty
        (pos_member, y)
        x
    in
    let (env, local) = Env.FakeMembers.make_static env x y in
    let env = set_valid_rvalue p env local ty2 in
    (env, te1, ty2)
  | (pos, Array_get (e1, None)) ->
    let (env, te1, ty1) = update_array_type pos env e1 None `lvalue in
    let (env, ty1') =
      Typing_array_access.assign_array_append
        ~array_pos:(fst e1)
        ~expr_pos:p
        ur
        env
        ty1
        ty2
    in
    let (env, te1) =
      if is_hack_collection env ty1 then
        (env, te1)
      else
        let (env, te1, _) = assign_ p ur env e1 ty1' in
        (env, te1)
    in
    make_result env pos (Aast.Array_get (te1, None)) ty2
  | (pos, Array_get (e1, Some e)) ->
    let (env, te1, ty1) = update_array_type pos env e1 (Some e) `lvalue in
    let (env, te, ty) = expr env e in
    let (env, ty1') =
      Typing_array_access.assign_array_get
        ~array_pos:(fst e1)
        ~expr_pos:p
        ur
        env
        ty1
        e
        ty
        ty2
    in
    let (env, te1) =
      if is_hack_collection env ty1 then
        (env, te1)
      else
        let (env, te1, _) = assign_ p ur env e1 ty1' in
        (env, te1)
    in
    (env, ((pos, ty2), Aast.Array_get (te1, Some te)), ty2)
  | _ -> assign_simple p ur env e1 ty2

and assign_simple pos ur env e1 ty2 =
  let (env, te1, ty1) = lvalue env e1 in
  let env =
    Typing_coercion.coerce_type
      pos
      ur
      env
      ty2
      (MakeType.unenforced ty1)
      Errors.unify_error
  in
  (env, te1, ty2)

and array_field env = function
  | AFvalue ve ->
    let (env, tve, tv) = expr env ve in
    (env, (Aast.AFvalue tve, None, tv))
  | AFkvalue (ke, ve) ->
    let (env, tke, tk) = expr env ke in
    let (env, tve, tv) = expr env ve in
    (env, (Aast.AFkvalue (tke, tve), Some tk, tv))

and array_value ~(expected : ExpectedTy.t option) env x =
  let (env, te, ty) = expr ?expected env x in
  (env, (te, ty))

and array_field_value ~(expected : ExpectedTy.t option) env = function
  | AFvalue x
  | AFkvalue (_, x) ->
    array_value ~expected env x

and arraykey_value
    p class_name ~(expected : ExpectedTy.t option) env ((pos, _) as x) =
  let (env, (te, ty)) = array_value ~expected env x in
  let ty_arraykey = MakeType.arraykey (Reason.Ridx_dict pos) in
  let env =
    Typing_coercion.coerce_type
      p
      (Reason.index_class class_name)
      env
      ty
      { et_type = ty_arraykey; et_enforced = true }
      Errors.unify_error
  in
  (env, (te, ty))

and array_field_key ~(expected : ExpectedTy.t option) env = function
  (* This shouldn't happen *)
  | AFvalue ((p, _) as e) ->
    let ty = MakeType.int (Reason.Rwitness p) in
    (env, (with_type ty Tast.dummy_saved_env e, ty))
  | AFkvalue (x, _) -> array_value ~expected env x

and check_parent_construct pos env el unpacked_element env_parent =
  let check_not_abstract = false in
  let (env, env_parent) = Phase.localize_with_self env env_parent in
  let (env, _tcid, _tal, tel, typed_unpack_element, parent, fty) =
    new_object
      ~expected:None
      ~check_parent:true
      ~check_not_abstract
      ~is_using_clause:false
      pos
      env
      CIparent
      []
      el
      unpacked_element
  in
  (* Not sure why we need to equate these types *)
  let env =
    Type.sub_type pos Reason.URnone env env_parent parent Errors.unify_error
  in
  let env =
    Type.sub_type pos Reason.URnone env parent env_parent Errors.unify_error
  in
  ( env,
    tel,
    typed_unpack_element,
    MakeType.void (Reason.Rwitness pos),
    parent,
    fty )

and check_class_get env p def_pos cid mid ce e =
  match e with
  | CIself when ce.ce_abstract ->
    begin
      match Env.get_self env with
      | (_, Tclass ((_, self), _, _)) ->
        (* at runtime, self:: in a trait is a call to whatever
         * self:: is in the context of the non-trait "use"-ing
         * the trait's code *)
        begin
          match Env.get_class env self with
          | Some cls when Ast_defs.(equal_class_kind (Cls.kind cls) Ctrait) ->
            (* Ban self::some_abstract_method() in a trait, if the
             * method is also defined in a trait.
             *
             * Abstract methods from interfaces are fine: we'll check
             * in the child class that we actually have an
             * implementation. *)
            (match Decl_provider.get_class (Env.get_ctx env) ce.ce_origin with
            | Some meth_cls
              when Ast_defs.(equal_class_kind (Cls.kind meth_cls) Ctrait) ->
              Errors.self_abstract_call mid p def_pos
            | _ -> ())
          | _ ->
            (* Ban self::some_abstract_method() in a class. This will
             *  always error. *)
            Errors.self_abstract_call mid p def_pos
        end
      | _ -> ()
    end
  | CIparent when ce.ce_abstract -> Errors.parent_abstract_call mid p def_pos
  | CI _ when ce.ce_abstract -> Errors.classname_abstract_call cid mid p def_pos
  | CI (_, classname) when ce.ce_synthesized ->
    Errors.static_synthetic_method classname mid p def_pos
  | _ -> ()

and call_parent_construct pos env el unpacked_element =
  match Env.get_parent_ty env with
  | Some parent -> check_parent_construct pos env el unpacked_element parent
  | None ->
    (* continue here *)
    let ty = (Reason.Rwitness pos, Typing_utils.tany env) in
    let default = (env, [], None, ty, ty, ty) in
    (match Env.get_self env with
    | (_, Tclass ((_, self), _, _)) ->
      (match Env.get_class env self with
      | Some trait when Ast_defs.(equal_class_kind (Cls.kind trait) Ctrait) ->
        (match trait_most_concrete_req_class trait env with
        | None ->
          Errors.parent_in_trait pos;
          default
        | Some (_, parent_ty) ->
          check_parent_construct pos env el unpacked_element parent_ty)
      | Some self_tc ->
        if not (Cls.members_fully_known self_tc) then
          ()
        (* Don't know the hierarchy, assume it's correct *)
        else
          Errors.undefined_parent pos;
        default
      | None -> assert false)
    | ( _,
        ( Terr | Tany _ | Tnonnull | Tarraykind _ | Toption _ | Tprim _ | Tfun _
        | Ttuple _ | Tshape _ | Tvar _ | Tdynamic | Tgeneric _ | Tnewtype _
        | Tdependent (_, _)
        | Tanon (_, _)
        | Tunion _ | Tintersection _ | Tobject | Tpu _ | Tpu_type_access _ ) )
      ->
      Errors.parent_outside_class pos;
      let ty = (Reason.Rwitness pos, Typing_utils.terr env) in
      (env, [], None, ty, ty, ty))

(* Depending on the kind of expression we are dealing with
 * The typing of call is different.
 *)
and dispatch_call
    ~(expected : ExpectedTy.t option)
    ~is_using_clause
    p
    env
    call_type
    ((fpos, fun_expr) as e)
    explicit_targs
    el
    unpacked_element
    ~in_suspend =
  let make_call env te tal tel typed_unpack_element ty =
    make_result
      env
      p
      (Aast.Call (call_type, te, tal, tel, typed_unpack_element))
      ty
  in
  (* TODO: Avoid Tany annotations in TAST by eliminating `make_call_special` *)
  let make_call_special env id tel ty =
    make_call
      env
      (Tast.make_typed_expr fpos (Reason.Rnone, TUtils.tany env) (Aast.Id id))
      []
      tel
      None
      ty
  in
  (* For special functions and pseudofunctions with a definition in hhi. *)
  let make_call_special_from_def env id tel ty_ =
    let (env, fty, tal) = fun_type_of_id env id explicit_targs el in
    let ty =
      match fty with
      | (_, Tfun ft) -> ft.ft_ret.et_type
      | _ -> (Reason.Rwitness p, ty_)
    in
    make_call env (Tast.make_typed_expr fpos fty (Aast.Id id)) tal tel None ty
  in
  let overload_function = overload_function make_call fpos in
  let check_coroutine_call env fty =
    let () =
      if is_return_disposable_fun_type env fty && not is_using_clause then
        Errors.invalid_new_disposable p
      else
        ()
    in
    (* returns
       - Some true if type is definitely a coroutine
       - Some false if type is definitely not a coroutine
       - None if type is Tunion that contains
         both coroutine and non-coroutine constituents *)
    (* TODO: replace the case analysis here with a subtyping check;
     * see T37483866 and the linked diff for discussion.
     *)
    let rec is_coroutine env ty =
      let (env, ety) =
        Typing_solver.expand_type_and_solve
          env
          (Reason.to_pos (fst ty))
          ty
          Errors.unify_error
          ~description_of_expected:"a function value"
      in
      match snd ety with
      | Tfun { ft_is_coroutine = true; _ } -> (env, Some true)
      | Tanon (_, id) ->
        ( env,
          Some
            (Option.value_map
               (Env.get_anonymous env id)
               ~default:false
               ~f:(fun ty_ -> ty_.is_coroutine)) )
      | Tunion ts
      | Tintersection ts ->
        are_coroutines env ts
      | _ -> (env, Some false)
    and are_coroutines env ts =
      let (env, ts_are_coroutines) = List.map_env env ts ~f:is_coroutine in
      let ts_are_coroutines =
        match ts_are_coroutines with
        | None :: _ -> None
        | Some x :: xs ->
          (*if rest of the list has the same value as the first element
            return value of the first element or None otherwise*)
          if
            List.for_all
              xs
              ~f:(Option.value_map ~default:false ~f:(Bool.equal x))
          then
            Some x
          else
            None
        | _ -> Some false
      in
      (env, ts_are_coroutines)
    in
    let (env, fty_is_coroutine) = is_coroutine env fty in
    let () =
      match (in_suspend, fty_is_coroutine) with
      | (true, Some true)
      | (false, Some false) ->
        ()
      | (true, _) ->
        (* non-coroutine call in suspend *)
        Errors.non_coroutine_call_in_suspend
          fpos
          (Reason.to_string ("This is " ^ Typing_print.error env fty) (fst fty))
      | (false, _) ->
        (*coroutine call outside of suspend *)
        Errors.coroutine_call_outside_of_suspend p
    in
    env
  in
  let check_function_in_suspend name =
    if in_suspend then Errors.function_is_not_coroutine fpos name
  in
  let check_class_function_in_suspend class_name function_name =
    check_function_in_suspend (class_name ^ "::" ^ function_name)
  in
  match fun_expr with
  (* Special function `echo` *)
  | Id ((p, pseudo_func) as id)
    when String.equal pseudo_func SN.SpecialFunctions.echo ->
    check_function_in_suspend SN.SpecialFunctions.echo;
    let (env, tel, _) = exprs ~accept_using_var:true env el in
    make_call_special env id tel (MakeType.void (Reason.Rwitness p))
  (* Special function `isset` *)
  | Id ((_, pseudo_func) as id)
    when String.equal pseudo_func SN.PseudoFunctions.isset ->
    check_function_in_suspend SN.PseudoFunctions.isset;
    let (env, tel, _) =
      exprs ~accept_using_var:true ~check_defined:false env el
    in
    if Option.is_some unpacked_element then
      Errors.unpacking_disallowed_builtin_function p pseudo_func;
    make_call_special_from_def env id tel (Tprim Tbool)
  (* Special function `unset` *)
  | Id ((_, pseudo_func) as id)
    when String.equal pseudo_func SN.PseudoFunctions.unset ->
    check_function_in_suspend SN.PseudoFunctions.unset;
    let (env, tel, _) = exprs env el in
    if Option.is_some unpacked_element then
      Errors.unpacking_disallowed_builtin_function p pseudo_func;
    let checked_unset_error =
      if Partial.should_check_error (Env.get_mode env) 4135 then
        Errors.unset_nonidx_in_strict
      else
        fun _ _ ->
      ()
    in
    let env =
      match (el, unpacked_element) with
      | ([(_, Array_get ((_, Class_const _), Some _))], None)
        when Partial.should_check_error (Env.get_mode env) 4011 ->
        Errors.const_mutation p Pos.none "";
        env
      | ([(_, Array_get (ea, Some _))], None) ->
        let (env, _te, ty) = expr env ea in
        let r = Reason.Rwitness p in
        let tmixed = MakeType.mixed r in
        let super =
          ( Reason.Rnone,
            Tunion
              [
                MakeType.dynamic r;
                MakeType.dict r tmixed tmixed;
                MakeType.keyset r tmixed;
                (r, Tarraykind (AKdarray (tmixed, tmixed)));
              ] )
        in
        SubType.sub_type_or_fail env ty super (fun () ->
            checked_unset_error
              p
              (Reason.to_string
                 ("This is " ^ Typing_print.error ~ignore_dynamic:true env ty)
                 (fst ty)))
      | _ ->
        checked_unset_error p [];
        env
    in
    (match el with
    | [(p, Obj_get (_, _, OG_nullsafe))] ->
      Errors.nullsafe_property_write_context p;
      make_call_special_from_def env id tel (TUtils.terr env)
    | _ -> make_call_special_from_def env id tel (Tprim Tvoid))
  (* Special function `array_filter` *)
  | Id ((_, array_filter) as id)
    when String.equal array_filter SN.StdlibFunctions.array_filter
         && (not (List.is_empty el))
         && Option.is_none unpacked_element ->
    check_function_in_suspend SN.StdlibFunctions.array_filter;

    (* dispatch the call to typecheck the arguments *)
    let (env, fty, tal) = fun_type_of_id env id explicit_targs el in
    let (env, (tel, typed_unpack_element, res)) =
      call ~expected p env fty el unpacked_element
    in
    (* but ignore the result and overwrite it with custom return type *)
    let x = List.hd_exn el in
    let (env, _tx, ty) = expr env x in
    let explain_array_filter (r, t) = (Reason.Rarray_filter (p, r), t) in
    let get_value_type env tv =
      let (env, tv) =
        if List.length el > 1 then
          (env, tv)
        else
          Typing_solver.non_null env p tv
      in
      (env, explain_array_filter tv)
    in
    let rec get_array_filter_return_type env ty =
      let (env, ety) = Env.expand_type env ty in
      match ety with
      | (_, Tarraykind AKempty) as array_type -> (env, array_type)
      | (r, Tarraykind (AKvarray tv)) ->
        let (env, tv) = get_value_type env tv in
        (env, (r, Tarraykind (AKvarray tv)))
      | (r, Tunion tyl) ->
        let (env, tyl) = List.map_env env tyl get_array_filter_return_type in
        Typing_union.union_list env r tyl
      | (r, Tintersection tyl) ->
        let (env, tyl) = List.map_env env tyl get_array_filter_return_type in
        Inter.intersect_list env r tyl
      | (r, Tany _) -> (env, (r, Typing_utils.tany env))
      | (r, Terr) -> (env, (r, Typing_utils.terr env))
      | (r, _) ->
        let (env, tk) = Env.fresh_type env p in
        let (env, tv) = Env.fresh_type env p in
        Errors.try_
          (fun () ->
            let keyed_container_type =
              MakeType.keyed_container Reason.Rnone tk tv
            in
            let env =
              SubType.sub_type env ety keyed_container_type Errors.unify_error
            in
            let (env, tv) = get_value_type env tv in
            (env, (r, Tarraykind (AKdarray (explain_array_filter tk, tv)))))
          (fun _ ->
            Errors.try_
              (fun () ->
                let container_type = MakeType.container Reason.Rnone tv in
                let env =
                  SubType.sub_type env ety container_type Errors.unify_error
                in
                let (env, tv) = get_value_type env tv in
                ( env,
                  ( r,
                    Tarraykind
                      (AKdarray (explain_array_filter (MakeType.arraykey r), tv))
                  ) ))
              (fun _ -> (env, res)))
    in
    let (env, rty) = get_array_filter_return_type env ty in
    let fty =
      match fty with
      | (r, Tfun ft) -> (r, Tfun { ft with ft_ret = MakeType.unenforced rty })
      | _ -> fty
    in
    make_call
      env
      (Tast.make_typed_expr fpos fty (Aast.Id id))
      tal
      tel
      typed_unpack_element
      rty
  (* Special function `type_structure` *)
  | Id (p, type_structure)
    when String.equal type_structure SN.StdlibFunctions.type_structure
         && Int.equal (List.length el) 2
         && Option.is_none unpacked_element ->
    check_function_in_suspend SN.StdlibFunctions.type_structure;
    (match el with
    | [e1; e2] ->
      (match e2 with
      | (p, String cst) ->
        (* find the class constant implicitly defined by the typeconst *)
        let cid =
          match e1 with
          | (_, Class_const (cid, (_, x)))
          | (_, Class_get (cid, CGstring (_, x)))
            when String.equal x SN.Members.mClass ->
            cid
          | _ -> (fst e1, CIexpr e1)
        in
        class_const ~incl_tc:true env p (cid, (p, cst))
      | _ ->
        Errors.illegal_type_structure p "second argument is not a string";
        expr_error env (Reason.Rwitness p) e)
    | _ -> assert false)
  (* Special function `array_map` *)
  | Id ((_, array_map) as x)
    when String.equal array_map SN.StdlibFunctions.array_map
         && (not (List.is_empty el))
         && Option.is_none unpacked_element ->
    check_function_in_suspend SN.StdlibFunctions.array_map;

    (* This uses the arity to determine a signature for array_map. But there
     * is more: for two-argument use of array_map, we specialize the return
     * type to the collection that's passed in, below. *)
    let (env, fty, tal) = fun_type_of_id env x explicit_targs el in
    let (env, fty) = Env.expand_type env fty in
    let r_fty = fst fty in
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
    let rec build_output_container env (x : locl_ty) :
        env * (env -> locl_ty -> env * locl_ty) =
      let (env, x) = Env.expand_type env x in
      match x with
      | (_, Tarraykind AKempty) as array_type ->
        (env, (fun env _ -> (env, array_type)))
      | (r, Tarraykind (AKvarray _)) ->
        (env, (fun env tr -> (env, (r, Tarraykind (AKvarray tr)))))
      | (r, Tany _) -> (env, (fun env _ -> (env, (r, Typing_utils.tany env))))
      | (r, Terr) -> (env, (fun env _ -> (env, (r, Typing_utils.terr env))))
      | (r, Tunion tyl) ->
        let (env, builders) = List.map_env env tyl build_output_container in
        ( env,
          fun env tr ->
            let (env, tyl) =
              List.map_env env builders (fun env f -> f env tr)
            in
            Typing_union.union_list env r tyl )
      | (r, Tintersection tyl) ->
        let (env, builders) = List.map_env env tyl build_output_container in
        ( env,
          fun env tr ->
            let (env, tyl) =
              List.map_env env builders (fun env f -> f env tr)
            in
            Typing_intersection.intersect_list env r tyl )
      | (r, _) ->
        let (env, tk) = Env.fresh_type env p in
        let (env, tv) = Env.fresh_type env p in
        let try_vector env =
          let vector_type = MakeType.const_vector r_fty tv in
          let env = SubType.sub_type env x vector_type Errors.unify_error in
          (env, (fun env tr -> (env, (r, Tarraykind (AKvarray tr)))))
        in
        let try_keyed_container env =
          let keyed_container_type = MakeType.keyed_container r_fty tk tv in
          let env =
            SubType.sub_type env x keyed_container_type Errors.unify_error
          in
          (env, (fun env tr -> (env, (r, Tarraykind (AKdarray (tk, tr))))))
        in
        let try_container env =
          let container_type = MakeType.container r_fty tv in
          let env = SubType.sub_type env x container_type Errors.unify_error in
          ( env,
            fun env tr ->
              (env, (r, Tarraykind (AKdarray (MakeType.arraykey r, tr)))) )
        in
        let (env, tr) =
          Errors.try_
            (fun () -> try_vector env)
            (fun _ ->
              Errors.try_
                (fun () -> try_keyed_container env)
                (fun _ ->
                  Errors.try_
                    (fun () -> try_container env)
                    (fun _ ->
                      ( env,
                        fun env _ ->
                          (env, (Reason.Rwitness p, Typing_utils.tany env)) ))))
        in
        (env, tr)
    in
    let (env, fty) =
      match (fty, el) with
      | ((_, Tfun funty), [_; x]) ->
        let (env, _tx, x) = expr env x in
        let (env, output_container) = build_output_container env x in
        begin
          match get_akvarray_inst funty.ft_ret.et_type with
          | None -> (env, fty)
          | Some elem_ty ->
            let (env, elem_ty) = output_container env elem_ty in
            let ft_ret = MakeType.unenforced elem_ty in
            (env, (r_fty, Tfun { funty with ft_ret }))
        end
      | _ -> (env, fty)
    in
    let (env, (tel, typed_unpack_element, ty)) =
      call ~expected p env fty el None
    in
    make_call
      env
      (Tast.make_typed_expr fpos fty (Aast.Id x))
      tal
      tel
      typed_unpack_element
      ty
  (* Special function `Shapes::idx` *)
  | Class_const (((_, CI (_, shapes)) as class_id), ((_, idx) as method_id))
    when String.equal shapes SN.Shapes.cShapes && String.equal idx SN.Shapes.idx
    ->
    check_class_function_in_suspend SN.Shapes.cShapes SN.Shapes.idx;
    overload_function
      p
      env
      class_id
      method_id
      el
      unpacked_element
      (fun env fty res el ->
        match el with
        | [shape; field] ->
          let (env, _ts, shape_ty) = expr env shape in
          Typing_shapes.idx
            env
            shape_ty
            field
            None
            ~expr_pos:p
            ~fun_pos:(fst fty)
            ~shape_pos:(fst shape)
        | [shape; field; default] ->
          let (env, _ts, shape_ty) = expr env shape in
          let (env, _td, default_ty) = expr env default in
          Typing_shapes.idx
            env
            shape_ty
            field
            (Some (fst default, default_ty))
            ~expr_pos:p
            ~fun_pos:(fst fty)
            ~shape_pos:(fst shape)
        | _ -> (env, res))
  (* Special function `Shapes::at` *)
  | Class_const (((_, CI (_, shapes)) as class_id), ((_, at) as method_id))
    when String.equal shapes SN.Shapes.cShapes && String.equal at SN.Shapes.at
    ->
    check_class_function_in_suspend SN.Shapes.cShapes SN.Shapes.at;
    overload_function
      p
      env
      class_id
      method_id
      el
      unpacked_element
      (fun env _fty res el ->
        match el with
        | [shape; field] ->
          let (env, _te, shape_ty) = expr env shape in
          Typing_shapes.at env ~expr_pos:p ~shape_pos:(fst shape) shape_ty field
        | _ -> (env, res))
  (* Special function `Shapes::keyExists` *)
  | Class_const
      (((_, CI (_, shapes)) as class_id), ((_, key_exists) as method_id))
    when String.equal shapes SN.Shapes.cShapes
         && String.equal key_exists SN.Shapes.keyExists ->
    check_class_function_in_suspend SN.Shapes.cShapes SN.Shapes.keyExists;
    overload_function
      p
      env
      class_id
      method_id
      el
      unpacked_element
      (fun env fty res el ->
        match el with
        | [shape; field] ->
          let (env, _te, shape_ty) = expr env shape in
          (* try accessing the field, to verify existence, but ignore
           * the returned type and keep the one coming from function
           * return type hint *)
          let (env, _) =
            Typing_shapes.idx
              env
              shape_ty
              field
              None
              ~expr_pos:p
              ~fun_pos:(fst fty)
              ~shape_pos:(fst shape)
          in
          (env, res)
        | _ -> (env, res))
  (* Special function `Shapes::removeKey` *)
  | Class_const
      (((_, CI (_, shapes)) as class_id), ((_, remove_key) as method_id))
    when String.equal shapes SN.Shapes.cShapes
         && String.equal remove_key SN.Shapes.removeKey ->
    check_class_function_in_suspend SN.Shapes.cShapes SN.Shapes.removeKey;
    overload_function
      p
      env
      class_id
      method_id
      el
      unpacked_element
      (fun env _ res el ->
        match el with
        | [shape; field] ->
          begin
            match shape with
            | (_, Lvar (_, lvar))
            | (_, Callconv (Ast_defs.Pinout, (_, Lvar (_, lvar)))) ->
              let (env, _te, shape_ty) = expr env shape in
              let (env, shape_ty) =
                Typing_shapes.remove_key p env shape_ty field
              in
              let env = set_valid_rvalue p env lvar shape_ty in
              (env, res)
            | _ ->
              Errors.invalid_shape_remove_key (fst shape);
              (env, res)
          end
        | _ -> (env, res))
  (* Special function `Shapes::toArray` *)
  | Class_const (((_, CI (_, shapes)) as class_id), ((_, to_array) as method_id))
    when String.equal shapes SN.Shapes.cShapes
         && String.equal to_array SN.Shapes.toArray ->
    check_class_function_in_suspend SN.Shapes.cShapes SN.Shapes.toArray;
    overload_function
      p
      env
      class_id
      method_id
      el
      unpacked_element
      (fun env _ res el ->
        match el with
        | [shape] ->
          let (env, _te, shape_ty) = expr env shape in
          Typing_shapes.to_array env p shape_ty res
        | _ -> (env, res))
  (* Special function `Shapes::toDict` *)
  | Class_const (((_, CI (_, shapes)) as class_id), ((_, to_array) as method_id))
    when String.equal shapes SN.Shapes.cShapes
         && String.equal to_array SN.Shapes.toDict ->
    check_class_function_in_suspend SN.Shapes.cShapes SN.Shapes.toDict;
    overload_function
      p
      env
      class_id
      method_id
      el
      unpacked_element
      (fun env _ res el ->
        match el with
        | [shape] ->
          let (env, _te, shape_ty) = expr env shape in
          Typing_shapes.to_dict env p shape_ty res
        | _ -> (env, res))
  (* Special function `parent::__construct` *)
  | Class_const ((pos, CIparent), ((_, construct) as id))
    when String.equal construct SN.Members.__construct ->
    check_class_function_in_suspend "parent" SN.Members.__construct;
    let (env, tel, typed_unpack_element, ty, pty, ctor_fty) =
      call_parent_construct p env el unpacked_element
    in
    make_call
      env
      (Tast.make_typed_expr
         fpos
         ctor_fty
         (Aast.Class_const (((pos, pty), Aast.CIparent), id)))
      [] (* tal: no type arguments to constructor *)
      tel
      typed_unpack_element
      ty
  (* Calling parent / class method *)
  | Class_const ((pos, e1), m) ->
    let (env, _tal, tcid, ty1) =
      static_class_id
        ~check_constraints:(not (Nast.equal_class_id_ e1 CIparent))
        pos
        env
        []
        e1
    in
    let this_ty = (Reason.Rwitness fpos, TUtils.this_of (Env.get_self env)) in
    (* In static context, you can only call parent::foo() on static methods.
     * In instance context, you can call parent:foo() on static
     * methods as well as instance methods
     *)
    let is_static =
      (not (Nast.equal_class_id_ e1 CIparent))
      || Env.is_static env
      || class_contains_smethod env ty1 m
    in
    let (env, (fty, tal)) =
      if is_static then
        class_get
          ~coerce_from_ty:None
          ~is_method:true
          ~is_const:false
          ~explicit_targs
          env
          ty1
          m
          e1
      else
        (* parent::nonStaticFunc() is really weird. It's calling a method
         * defined on the parent class, but $this is still the child class.
         * We can deal with this by hijacking the continuation that
         * calculates the SN.Typehints.this type *)
        let k_lhs _ = this_ty in
        TOG.obj_get_
          ~inst_meth:false
          ~is_method:true
          ~nullsafe:None
          ~obj_pos:pos
          ~coerce_from_ty:None
          ~pos_params:(Some el)
          ~is_nonnull:false
          env
          ty1
          e1
          m
          k_lhs
          Errors.unify_error
    in
    let env = check_coroutine_call env fty in
    let ty =
      if Nast.equal_class_id_ e1 CIparent then
        this_ty
      else
        ty1
    in
    let (env, (tel, typed_unpack_element, ty)) =
      call
        ~expected
        ~method_call_info:
          (TR.make_call_info
             ~receiver_is_self:(Nast.equal_class_id_ e1 CIself)
             ~is_static
             ty
             (snd m))
        p
        env
        fty
        el
        unpacked_element
    in
    make_call
      env
      (Tast.make_typed_expr fpos fty (Aast.Class_const (tcid, m)))
      tal
      tel
      typed_unpack_element
      ty
  (* <<__PPL>>: sample, factor, observe, condition *)
  | Id (pos, id) when env.inside_ppl_class && SN.PPLFunctions.is_reserved id ->
    let m = (pos, String_utils.lstrip id "\\") in
    (* Mock these as type equivalent to \Infer -> sample... *)
    let infer_e = CI (p, "\\Infer") in
    let (env, _tal, _, ty1) =
      static_class_id ~check_constraints:true p env [] infer_e
    in
    let nullsafe = None in
    let (env, (tfty, tal)) =
      TOG.obj_get
        ~obj_pos:p
        ~is_method:true
        ~nullsafe
        ~pos_params:el
        ~coerce_from_ty:None
        ~explicit_targs
        env
        ty1
        infer_e
        m
        Errors.unify_error
    in
    let (env, (tel, typed_unpack_element, ty)) =
      call
        ~expected
        ~method_call_info:
          (TR.make_call_info
             ~receiver_is_self:false
             ~is_static:false
             ty1
             (snd m))
        p
        env
        tfty
        el
        unpacked_element
    in
    make_call
      env
      (Tast.make_typed_expr fpos tfty (Aast.Fun_id m))
      tal
      tel
      typed_unpack_element
      ty
  (* Call instance method *)
  | Obj_get (e1, (pos_id, Id m), nullflavor) ->
    let is_method = Aast_defs.equal_call_type call_type Cnormal in
    let (env, te1, ty1) = expr ~accept_using_var:true env e1 in
    let nullsafe =
      match nullflavor with
      | OG_nullthrows -> None
      | OG_nullsafe -> Some p
    in
    let (env, (tfty, tal)) =
      TOG.obj_get
        ~obj_pos:(fst e1)
        ~is_method
        ~nullsafe
        ~pos_params:el
        ~coerce_from_ty:None
        ~explicit_targs
        env
        ty1
        (CIexpr e1)
        m
        Errors.unify_error
    in
    let env = check_coroutine_call env tfty in
    let (env, (tel, typed_unpack_element, ty)) =
      call
        ~nullsafe
        ~expected
        ~method_call_info:
          (TR.make_call_info
             ~receiver_is_self:false
             ~is_static:false
             ty1
             (snd m))
        p
        env
        tfty
        el
        unpacked_element
    in
    make_call
      env
      (Tast.make_typed_expr
         fpos
         tfty
         (Aast.Obj_get
            (te1, Tast.make_typed_expr pos_id tfty (Aast.Id m), nullflavor)))
      tal
      tel
      typed_unpack_element
      ty
  (* Function invocation *)
  | Fun_id x ->
    let (env, fty, tal) = fun_type_of_id env x explicit_targs el in
    let env = check_coroutine_call env fty in
    let (env, (tel, typed_unpack_element, ty)) =
      call ~expected p env fty el unpacked_element
    in
    make_call
      env
      (Tast.make_typed_expr fpos fty (Aast.Fun_id x))
      tal
      tel
      typed_unpack_element
      ty
  | Id ((_, id) as x) ->
    let (env, fty, tal) = fun_type_of_id env x explicit_targs el in
    let env = check_coroutine_call env fty in
    let (env, (tel, typed_unpack_element, ty)) =
      call ~expected p env fty el unpacked_element
    in
    let is_mutable = String.equal id SN.Rx.mutable_ in
    let is_move = String.equal id SN.Rx.move in
    let is_freeze = String.equal id SN.Rx.freeze in
    (* error when rx builtins are used in non-reactive context *)
    if not (Env.env_local_reactive env) then
      if is_mutable then
        Errors.mutable_in_nonreactive_context p
      else if is_move then
        Errors.move_in_nonreactive_context p
      else if is_freeze then
        Errors.freeze_in_nonreactive_context p;

    (* ban unpacking when calling builtings *)
    if (is_mutable || is_move || is_freeze) && Option.is_some unpacked_element
    then
      Errors.unpacking_disallowed_builtin_function p id;

    (* adjust env for Rx\freeze or Rx\move calls *)
    let env =
      if is_freeze then
        Typing_mutability.freeze_local p env tel
      else if is_move then
        Typing_mutability.move_local p env tel
      else
        env
    in
    make_call
      env
      (Tast.make_typed_expr fpos fty (Aast.Id x))
      tal
      tel
      typed_unpack_element
      ty
  | PU_identifier ((cpos, cid), ((_, enum) as enum'), ((_, case) as case')) ->
    let (env, tal, te1, ty1) =
      static_class_id ~check_constraints:false cpos env [] cid
    in
    let (env, fty) =
      let (env, ety_env, et) =
        match class_get_pu ~from_class:cid env ty1 enum with
        | (_env, None) ->
          failwithf "TODO(T36532263): class_get_pu: %s not found" enum ()
        | (env, Some (ety_env, et)) -> (env, ety_env, et)
      in
      let make_fty params ft_ret =
        let len = List.length params in
        {
          ft_is_coroutine = false;
          ft_arity = Fstandard (len, len);
          ft_tparams = ([], FTKtparams);
          ft_where_constraints = [];
          ft_params =
            List.map params ~f:(fun et_type ->
                {
                  fp_pos = cpos;
                  fp_name = None;
                  fp_type = { et_enforced = false; et_type };
                  fp_kind = FPnormal;
                  fp_accept_disposable = true;
                  fp_mutability = None;
                  fp_rx_annotation = None;
                });
          ft_ret = { et_enforced = false; et_type = ft_ret };
          ft_reactive = Nonreactive;
          ft_return_disposable = false;
          (* mutability of the receiver *)
          ft_mutability = None;
          ft_returns_mutable = false;
          ft_returns_void_to_rx = false;
          ft_fun_kind = Ast_defs.FSync;
        }
      in
      let reason = Reason.Rwitness cpos in
      let (env, fty) =
        let pu_type = (reason, Tpu (ty1, enum')) in
        if String.equal SN.PocketUniverses.members case then
          ( env,
            make_fty
              []
              (reason, Tclass ((fst et.tpu_name, "\\vec"), Nonexact, [pu_type]))
          )
        else
          let case_ty =
            match SMap.find_opt case et.tpu_case_values with
            | Some (_, case) -> case
            | None ->
              failwithf
                "TODO(T36532263): PU_identifier: %s has no case value %s"
                enum
                case
                ()
          in
          (* Type variable to type the parameter of the Pu expression call.
             We use a variable in case there is some dependency *)
          (* let (env, fresh_ty) = Env.fresh_invariant_type_var env cpos in *)
          let (env, fresh_ty) = Env.fresh_type env cpos in
          (* It's original upper bound is the PU enum itself *)
          let env =
            SubType.sub_type
              env
              fresh_ty
              (reason, Tpu (ty1, enum'))
              Errors.pocket_universes_typing
          in
          let substs =
            let f (id, _reified) =
              (reason, Tpu_type_access (ty1, enum', fresh_ty, id))
            in
            SMap.map f et.tpu_case_types
          in
          let ety_env =
            let combine _ va vb =
              if Option.is_some vb then
                vb
              else
                va
            in
            let substs = SMap.merge combine ety_env.substs substs in
            { ety_env with substs }
          in
          let (env, case_ty) = Phase.localize ~ety_env env case_ty in
          (env, make_fty [fresh_ty] case_ty)
      in
      (env, (Reason.Rwitness p, Tfun fty))
    in
    let (env, (tel, tuel, ty)) = call ~expected p env fty el unpacked_element in
    let (a, b, c) =
      make_call
        env
        (Tast.make_typed_expr fpos fty (Aast.PU_identifier (te1, enum', case')))
        tal
        tel
        tuel
        ty
    in
    (a, b, c)
  | _ ->
    let (env, te, fty) = expr env e in
    let (env, fty) =
      Typing_solver.expand_type_and_solve
        ~description_of_expected:"a function value"
        env
        fpos
        fty
        Errors.unify_error
    in
    let env = check_coroutine_call env fty in
    let (env, (tel, typed_unpack_element, ty)) =
      call ~expected p env fty el unpacked_element
    in
    make_call
      env
      te
      (* tal: no type arguments to function values, as they are non-generic *)
      []
      tel
      typed_unpack_element
      ty

and fun_type_of_id env x tal el =
  match Env.get_fun env (snd x) with
  | None ->
    let (env, _, ty) = unbound_name env x (Pos.none, Aast.Null) in
    (env, ty, [])
  | Some { fe_type; fe_pos; fe_deprecated; _ } ->
    (match fe_type with
    | (r, Tfun ft) ->
      let ft =
        Typing_special_fun.transform_special_fun_ty ft x (List.length el)
      in
      let ety_env = Phase.env_with_self env in
      let (env, tal) =
        Phase.localize_targs
          ~is_method:true
          ~def_pos:fe_pos
          ~use_pos:(fst x)
          ~use_name:(strip_ns (snd x))
          env
          (fst ft.ft_tparams)
          (List.map ~f:snd tal)
      in
      let ft =
        Typing_enforceability.compute_enforced_and_pessimize_fun_type env ft
      in
      let use_pos = fst x in
      let (env, ft) =
        Phase.(
          localize_ft
            ~instantiation:
              { use_name = strip_ns (snd x); use_pos; explicit_targs = tal }
            ~def_pos:fe_pos
            ~ety_env
            env
            ft)
      in
      let fty = (fst fe_type, Tfun ft) in
      let def_pos = Reason.to_pos r in
      TVis.check_deprecated ~use_pos ~def_pos fe_deprecated;
      (env, fty, tal)
    | _ -> failwith "Expected function type")

(**
 * Checks if a class (given by cty) contains a given static method.
 *
 * We could refactor this + class_get
 *)
and class_contains_smethod env cty (_pos, mid) =
  let lookup_member ty =
    match ty with
    | (_, Tclass ((_, c), _, _)) ->
      (match Env.get_class env c with
      | None -> false
      | Some class_ ->
        Option.is_some @@ Env.get_static_member true env class_ mid)
    | _ -> false
  in
  let (_env, tyl) = TUtils.get_concrete_supertypes env cty in
  List.exists tyl ~f:lookup_member

and class_get
    ~is_method
    ~is_const
    ~coerce_from_ty
    ?(explicit_targs = [])
    ?(incl_tc = false)
    env
    cty
    (p, mid)
    cid =
  let (env, this_ty) =
    if is_method then
      this_for_method env cid cty
    else
      (env, cty)
  in
  class_get_
    ~is_method
    ~is_const
    ~this_ty
    ~explicit_targs
    ~incl_tc
    ~coerce_from_ty
    env
    cid
    cty
    (p, mid)

and class_get_
    ~is_method
    ~is_const
    ~this_ty
    ~coerce_from_ty
    ?(explicit_targs = [])
    ?(incl_tc = false)
    env
    cid
    cty
    (p, mid) =
  let (env, cty) = Env.expand_type env cty in
  match cty with
  | (r, Tany _) -> (env, ((r, Typing_utils.tany env), []))
  | (r, Terr) -> (env, (err_witness env (Reason.to_pos r), []))
  | (_, Tdynamic) -> (env, (cty, []))
  | (_, Tunion tyl) ->
    let (env, pairs) =
      List.map_env env tyl (fun env ty ->
          class_get
            ~is_method
            ~is_const
            ~explicit_targs
            ~incl_tc
            ~coerce_from_ty
            env
            ty
            (p, mid)
            cid)
    in
    let (env, ty) = Union.union_list env (fst cty) (List.map ~f:fst pairs) in
    (env, (ty, []))
  | (_, Tintersection tyl) ->
    let (env, pairs) =
      TUtils.run_on_intersection env tyl ~f:(fun env ty ->
          class_get
            ~is_method
            ~is_const
            ~explicit_targs
            ~incl_tc
            ~coerce_from_ty
            env
            ty
            (p, mid)
            cid)
    in
    let (env, ty) =
      Inter.intersect_list env (fst cty) (List.map ~f:fst pairs)
    in
    (env, (ty, []))
  | (_, Tnewtype (_, _, ty))
  | (_, Tdependent (_, ty)) ->
    class_get_
      ~is_method
      ~is_const
      ~this_ty
      ~explicit_targs
      ~incl_tc
      ~coerce_from_ty
      env
      cid
      ty
      (p, mid)
  | (_, Tgeneric _) ->
    let resl =
      TUtils.try_over_concrete_supertypes env cty (fun env ty ->
          class_get_
            ~is_method
            ~is_const
            ~this_ty
            ~explicit_targs
            ~incl_tc
            ~coerce_from_ty
            env
            cid
            ty
            (p, mid))
    in
    begin
      match resl with
      | [] ->
        Errors.non_class_member
          ~is_method
          mid
          p
          (Typing_print.error env cty)
          (Reason.to_pos (fst cty));
        (env, (err_witness env p, []))
      | ((_, (ty, _)) as res) :: rest ->
        if List.exists rest (fun (_, (ty', _)) -> not @@ ty_equal ty' ty) then (
          Errors.ambiguous_member
            ~is_method
            mid
            p
            (Typing_print.error env cty)
            (Reason.to_pos (fst cty));
          (env, (err_witness env p, []))
        ) else
          res
    end
  | (_, Tclass ((_, c), _, paraml)) ->
    let class_ = Env.get_class env c in
    (match class_ with
    | None -> (env, ((Reason.Rwitness p, Typing_utils.tany env), []))
    | Some class_ ->
      (* We need to instantiate generic parameters in the method signature *)
      let ety_env =
        {
          type_expansions = [];
          this_ty;
          substs = Subst.make_locl (Cls.tparams class_) paraml;
          from_class = Some cid;
          quiet = true;
          on_error = Errors.unify_error_at p;
        }
      in
      let get_smember_from_constraints env class_info =
        let upper_bounds =
          Sequence.to_list
            (Cls.upper_bounds_on_this_from_constraints class_info)
        in
        let (env, upper_bounds) =
          List.map_env env upper_bounds ~f:(fun env up ->
              Phase.localize ~ety_env env up)
        in
        let (env, inter_ty) =
          Inter.intersect_list env (Reason.Rwitness p) upper_bounds
        in
        class_get_
          ~is_method
          ~is_const
          ~this_ty
          ~explicit_targs
          ~incl_tc
          ~coerce_from_ty
          env
          cid
          inter_ty
          (p, mid)
      in
      let try_get_smember_from_constraints env class_info =
        Errors.try_with_result
          (fun () -> get_smember_from_constraints env class_info)
          (fun _ _ ->
            TOG.smember_not_found
              p
              ~is_const
              ~is_method
              class_info
              mid
              Errors.unify_error;
            (env, ((Reason.Rnone, Typing_utils.terr env), [])))
      in
      if is_const then (
        let const =
          if incl_tc then
            Env.get_const env class_ mid
          else
            match Env.get_typeconst env class_ mid with
            | Some _ ->
              Errors.illegal_typeconst_direct_access p;
              None
            | None -> Env.get_const env class_ mid
        in
        match const with
        | None when Cls.has_upper_bounds_on_this_from_constraints class_ ->
          try_get_smember_from_constraints env class_
        | None ->
          TOG.smember_not_found
            p
            ~is_const
            ~is_method
            class_
            mid
            Errors.unify_error;
          (env, ((Reason.Rnone, Typing_utils.terr env), []))
        | Some { cc_type; cc_abstract; cc_pos; _ } ->
          let (env, cc_locl_type) = Phase.localize ~ety_env env cc_type in
          ( if cc_abstract then
            match cid with
            | CIstatic
            | CIexpr _ ->
              ()
            | _ ->
              let cc_name = Cls.name class_ ^ "::" ^ mid in
              Errors.abstract_const_usage p cc_pos cc_name );
          (env, (cc_locl_type, []))
      ) else
        let static_member_opt =
          Env.get_static_member is_method env class_ mid
        in
        (match static_member_opt with
        | None when Cls.has_upper_bounds_on_this_from_constraints class_ ->
          try_get_smember_from_constraints env class_
        | None ->
          TOG.smember_not_found
            p
            ~is_const
            ~is_method
            class_
            mid
            Errors.unify_error;
          (env, ((Reason.Rnone, Typing_utils.terr env), []))
        | Some
            ( {
                ce_visibility = vis;
                ce_lsb = lsb;
                ce_type = (lazy member_decl_ty);
                ce_deprecated;
                _;
              } as ce ) ->
          let def_pos = Reason.to_pos (fst member_decl_ty) in
          TVis.check_class_access ~use_pos:p ~def_pos env (vis, lsb) cid class_;
          TVis.check_deprecated ~use_pos:p ~def_pos ce_deprecated;
          check_class_get env p def_pos c mid ce cid;
          let (env, member_ty, et_enforced, tal) =
            match member_decl_ty with
            (* We special case Tfun here to allow passing in explicit tparams to localize_ft. *)
            | (r, Tfun ft) when is_method ->
              let (env, explicit_targs) =
                Phase.localize_targs
                  ~is_method:true
                  ~def_pos
                  ~use_pos:p
                  ~use_name:(strip_ns mid)
                  env
                  (fst ft.ft_tparams)
                  (List.map ~f:snd explicit_targs)
              in
              let ft =
                Typing_enforceability.compute_enforced_and_pessimize_fun_type
                  env
                  ft
              in
              let (env, ft) =
                Phase.(
                  localize_ft
                    ~instantiation:
                      { use_name = strip_ns mid; use_pos = p; explicit_targs }
                    ~ety_env
                    ~def_pos
                    env
                    ft)
              in
              (env, (r, Tfun ft), false, explicit_targs)
            (* unused *)
            | _ ->
              let { et_type; et_enforced } =
                Typing_enforceability.compute_enforced_and_pessimize_ty
                  env
                  member_decl_ty
              in
              let (env, member_ty) = Phase.localize ~ety_env env et_type in
              (* TODO(T52753871) make function just return possibly_enforced_ty
               * after considering intersection case *)
              (env, member_ty, et_enforced, [])
          in
          let (env, member_ty) =
            if Cls.has_upper_bounds_on_this_from_constraints class_ then
              let ((env, (member_ty', _)), succeed) =
                Errors.try_with_result
                  (fun () -> (get_smember_from_constraints env class_, true))
                  (fun _ _ ->
                    (* No eligible functions found in constraints *)
                    ((env, (MakeType.mixed Reason.Rnone, [])), false))
              in
              if succeed then
                Inter.intersect env (Reason.Rwitness p) member_ty member_ty'
              else
                (env, member_ty)
            else
              (env, member_ty)
          in
          let env =
            match coerce_from_ty with
            | None -> env
            | Some (p, ur, ty) ->
              Typing_coercion.coerce_type
                p
                ur
                env
                ty
                { et_type = member_ty; et_enforced }
                Errors.unify_error
          in
          (env, (member_ty, tal))))
  | ( _,
      ( Tvar _ | Tnonnull | Tarraykind _ | Toption _ | Tprim _ | Tfun _
      | Ttuple _
      | Tanon (_, _)
      | Tobject | Tshape _ | Tpu _ | Tpu_type_access _ ) ) ->
    (* should never happen; static_class_id takes care of these *)
    (env, ((Reason.Rnone, Typing_utils.tany env), []))

and class_id_for_new ~exact p env cid explicit_targs =
  let (env, tal, te, cid_ty) =
    static_class_id ~exact ~check_constraints:false p env explicit_targs cid
  in
  (* Need to deal with union case *)
  let rec get_info res tyl =
    match tyl with
    | [] -> (env, tal, te, res)
    | ty :: tyl ->
      (match snd ty with
      | Tunion tyl'
      | Tintersection tyl' ->
        get_info res (tyl' @ tyl)
      | _ ->
        (* Instantiation on an abstract class (e.g. from classname<T>) is
         * via the base type (to check constructor args), but the actual
         * type `ty` must be preserved. *)
        (match TUtils.get_base_type env ty with
        | (_, Tclass (sid, _, _)) ->
          let class_ = Env.get_class env (snd sid) in
          (match class_ with
          | None -> get_info res tyl
          | Some class_info ->
            (match (te, cid_ty) with
            (* When computing the classes for a new T() where T is a generic,
             * the class must be consistent (final, final constructor, or
             * <<__ConsistentConstruct>>) for its constructor to be considered *)
            | ((_, Aast.CI (_, c)), (_, Tgeneric cg)) when String.equal c cg ->
              (* Only have this choosing behavior for new T(), not all generic types
               * i.e. new classname<T>, TODO: T41190512 *)
              if Tast_utils.valid_newable_class class_info then
                get_info ((sid, class_info, ty) :: res) tyl
              else
                get_info res tyl
            | _ -> get_info ((sid, class_info, ty) :: res) tyl))
        | ( _,
            ( Tany _ | Terr | Tnonnull | Tarraykind _ | Toption _ | Tprim _
            | Tvar _ | Tfun _ | Tgeneric _ | Tnewtype _
            | Tdependent (_, _)
            | Ttuple _
            | Tanon (_, _)
            | Tunion _ | Tintersection _ | Tobject | Tshape _ | Tdynamic | Tpu _
            | Tpu_type_access _ ) ) ->
          get_info res tyl))
  in
  get_info [] [cid_ty]

(* To be a valid trait declaration, all of its 'require extends' must
 * match; since there's no multiple inheritance, it follows that all of
 * the 'require extends' must belong to the same inheritance hierarchy
 * and one of them should be the child of all the others *)
and trait_most_concrete_req_class trait env =
  Sequence.fold
    (Cls.all_ancestor_reqs trait)
    ~f:
      begin
        fun acc (_p, ty) ->
        let (_r, (_p, name), _paraml) = TUtils.unwrap_class_type ty in
        let keep =
          match acc with
          | Some (c, _ty) -> Cls.has_ancestor c name
          | None -> false
        in
        if keep then
          acc
        else
          let class_ = Env.get_class env name in
          match class_ with
          | None -> acc
          | Some c when Ast_defs.(equal_class_kind (Cls.kind c) Cinterface) ->
            acc
          | Some c when Ast_defs.(equal_class_kind (Cls.kind c) Ctrait) ->
            (* this is an error case for which the nastCheck spits out
             * an error, but does *not* currently remove the offending
             * 'require extends' or 'require implements' *)
            acc
          | Some c -> Some (c, ty)
      end
    ~init:None

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
and this_for_method env cid default_ty =
  match cid with
  | CIparent
  | CIself
  | CIstatic ->
    let p = Reason.to_pos (fst default_ty) in
    let (env, _tal, _te, ty) =
      static_class_id ~check_constraints:false p env [] CIstatic
    in
    ExprDepTy.make env CIstatic ty
  | _ -> (env, default_ty)

and static_class_id ?(exact = Nonexact) ~check_constraints p env tal =
  let make_result env tal te ty = (env, tal, ((p, ty), te), ty) in
  function
  | CIparent ->
    (match Env.get_self env with
    | (_, Tclass ((_, self), _, _)) ->
      (match Env.get_class env self with
      | Some trait when Ast_defs.(equal_class_kind (Cls.kind trait) Ctrait) ->
        (match trait_most_concrete_req_class trait env with
        | None ->
          Errors.parent_in_trait p;
          make_result
            env
            []
            Aast.CIparent
            (Reason.Rwitness p, Typing_utils.terr env)
        | Some (_, parent_ty) ->
          (* inside a trait, parent is SN.Typehints.this, but with the
           * type of the most concrete class that the trait has
           * "require extend"-ed *)
          let r = Reason.Rwitness p in
          let (env, parent_ty) = Phase.localize_with_self env parent_ty in
          make_result env [] Aast.CIparent (r, TUtils.this_of parent_ty))
      | _ ->
        let parent =
          match Env.get_parent_ty env with
          | None ->
            Errors.parent_undefined p;
            (Reason.none, Typing_defs.make_tany ())
          | Some parent -> parent
        in
        let r = Reason.Rwitness p in
        let (env, parent) = Phase.localize_with_self env parent in
        (* parent is still technically the same object. *)
        make_result env [] Aast.CIparent (r, TUtils.this_of (r, snd parent)))
    | ( _,
        ( Terr | Tany _ | Tnonnull | Tarraykind _ | Toption _ | Tprim _ | Tfun _
        | Ttuple _ | Tshape _ | Tvar _ | Tdynamic
        | Tanon (_, _)
        | Tunion _ | Tintersection _ | Tgeneric _ | Tnewtype _
        | Tdependent (_, _)
        | Tobject | Tpu _ | Tpu_type_access _ ) ) ->
      let parent =
        match Env.get_parent_ty env with
        | None ->
          Errors.parent_undefined p;
          (Reason.none, Typing_defs.make_tany ())
        | Some parent -> parent
      in
      let r = Reason.Rwitness p in
      let (env, parent) = Phase.localize_with_self env parent in
      (* parent is still technically the same object. *)
      make_result env [] Aast.CIparent (r, TUtils.this_of (r, snd parent)))
  | CIstatic ->
    let this = (Reason.Rwitness p, TUtils.this_of (Env.get_self env)) in
    make_result env [] Aast.CIstatic this
  | CIself ->
    let self =
      match snd (Env.get_self env) with
      | Tclass (c, _, tyl) -> Tclass (c, exact, tyl)
      | self -> self
    in
    make_result env [] Aast.CIself (Reason.Rwitness p, self)
  | CI ((p, id) as c) as e1 ->
    if Env.is_generic_parameter env id then
      let (env, tal) =
        Phase.localize_targs
          ~is_method:true
          ~def_pos:p
          ~use_pos:p
          ~use_name:(strip_ns (snd c))
          env
          []
          (List.map ~f:snd tal)
      in
      let r = Reason.Rhint p in
      let tgeneric = (r, Tgeneric id) in
      make_result env tal (Aast.CI c) tgeneric
    else
      let class_ = Env.get_class env (snd c) in
      (match class_ with
      | None ->
        make_result env [] (Aast.CI c) (Reason.Rwitness p, Typing_utils.tany env)
      | Some class_ ->
        let (env, ty, tal) =
          List.map ~f:snd tal
          |> Phase.resolve_type_arguments_and_check_constraints
               ~exact
               ~check_constraints
               ~def_pos:(Cls.pos class_)
               ~use_pos:p
               env
               c
               e1
               (Cls.tparams class_)
        in
        make_result env tal (Aast.CI c) ty)
  | CIexpr ((p, _) as e) ->
    let (env, te, ty) = expr env e in
    let rec resolve_ety env ty =
      let (env, ty) =
        Typing_solver.expand_type_and_solve
          ~description_of_expected:"an object"
          env
          p
          ty
          Errors.unify_error
      in
      match TUtils.get_base_type env ty with
      | (_, Tnewtype (classname, [the_cls], _))
        when String.equal classname SN.Classes.cClassname ->
        resolve_ety env the_cls
      | (_, Tgeneric _)
      | (_, Tclass _) ->
        (env, ty)
      | (r, Tunion tyl) ->
        let (env, tyl) = List.map_env env tyl resolve_ety in
        (env, (r, Tunion tyl))
      | (r, Tintersection tyl) ->
        let (env, tyl) = TUtils.run_on_intersection env tyl ~f:resolve_ety in
        Inter.intersect_list env r tyl
      | (_, Tdynamic) as ty -> (env, ty)
      | (_, (Tany _ | Tprim Tstring | Tobject)) when not (Env.is_strict env) ->
        (env, (Reason.Rwitness p, Typing_utils.tany env))
      | (_, Terr) -> (env, (Reason.Rwitness p, Typing_utils.terr env))
      | (r, Tvar _) ->
        Errors.unknown_type "an object" p (Reason.to_string "It is unknown" r);
        (env, (Reason.Rwitness p, Typing_utils.terr env))
      | ( _,
          ( Tany _ | Tnonnull | Tarraykind _ | Toption _ | Tprim _ | Tfun _
          | Ttuple _ | Tnewtype _ | Tdependent _
          | Tanon (_, _)
          | Tobject | Tshape _ | Tpu _ | Tpu_type_access _ ) ) as ty ->
        Errors.expected_class
          ~suffix:(", but got " ^ Typing_print.error env ty)
          p;
        (env, (Reason.Rwitness p, Typing_utils.terr env))
    in
    let (env, result_ty) = resolve_ety env ty in
    make_result env [] (Aast.CIexpr te) result_ty

and call_construct p env class_ params el unpacked_element cid =
  let cid =
    if Nast.equal_class_id_ cid CIparent then
      CIstatic
    else
      cid
  in
  let (env, _tal, tcid, cid_ty) =
    static_class_id ~check_constraints:false p env [] cid
  in
  let ety_env =
    {
      type_expansions = [];
      this_ty = cid_ty;
      substs = Subst.make_locl (Cls.tparams class_) params;
      from_class = Some cid;
      quiet = true;
      on_error = Errors.unify_error_at p;
    }
  in
  let env =
    Phase.check_tparams_constraints ~use_pos:p ~ety_env env (Cls.tparams class_)
  in
  let env =
    Phase.check_where_constraints
      ~in_class:true
      ~use_pos:p
      ~definition_pos:(Cls.pos class_)
      ~ety_env
      env
      (Cls.where_constraints class_)
  in
  if Cls.is_xhp class_ then
    (env, tcid, [], None, (Reason.Rnone, TUtils.tany env))
  else
    let cstr = Env.get_construct env class_ in
    let mode = Env.get_mode env in
    match fst cstr with
    | None ->
      if
        (not (List.is_empty el))
        && (FileInfo.is_strict mode || FileInfo.(equal_mode mode Mpartial))
        && Cls.members_fully_known class_
      then
        Errors.constructor_no_args p;
      let (env, tel, _tyl) = exprs env el in
      (env, tcid, tel, None, (Reason.Rnone, TUtils.terr env))
    | Some { ce_visibility = vis; ce_type = (lazy m); ce_deprecated; _ } ->
      let def_pos = Reason.to_pos (fst m) in
      TVis.check_obj_access ~use_pos:p ~def_pos env vis;
      TVis.check_deprecated ~use_pos:p ~def_pos ce_deprecated;
      let m =
        match m with
        | (r, Tfun ft) ->
          ( r,
            Tfun
              (Typing_enforceability.compute_enforced_and_pessimize_fun_type
                 env
                 ft) )
        | _ -> m
      in
      let (env, m) = Phase.localize ~ety_env env m in
      let (env, (tel, typed_unpack_element, _ty)) =
        call ~expected:None p env m el unpacked_element
      in
      (env, tcid, tel, typed_unpack_element, m)

and check_arity ?(did_unpack = false) pos pos_def (arity : int) exp_arity =
  let exp_min = Typing_defs.arity_min exp_arity in
  if arity < exp_min then Errors.typing_too_few_args exp_min arity pos pos_def;
  match exp_arity with
  | Fstandard (_, exp_max) ->
    let arity =
      if did_unpack then
        arity + 1
      else
        arity
    in
    if arity > exp_max then
      Errors.typing_too_many_args exp_max arity pos pos_def
  | Fvariadic _
  | Fellipsis _ ->
    ()

and check_lambda_arity lambda_pos def_pos lambda_arity expected_arity =
  let expected_min = Typing_defs.arity_min expected_arity in
  match (lambda_arity, expected_arity) with
  | (Fstandard (lambda_min, _), Fstandard _) ->
    if lambda_min < expected_min then
      Errors.typing_too_few_args expected_min lambda_min lambda_pos def_pos;
    if lambda_min > expected_min then
      Errors.typing_too_many_args expected_min lambda_min lambda_pos def_pos
  | (_, _) -> ()

(* The variadic capture argument is an array listing the passed
 * variable arguments for the purposes of the function body; callsites
 * should not unify with it *)
and variadic_param env ft =
  match ft.ft_arity with
  | Fvariadic (_, param) -> (env, Some param)
  | Fellipsis (_, pos) ->
    ( env,
      Some
        (TUtils.default_fun_param
           ~pos
           (Reason.Rvar_param pos, Typing_defs.make_tany ())) )
  | Fstandard _ -> (env, None)

and param_modes ?(is_variadic = false) { fp_pos; fp_kind; _ } (pos, e) =
  match (fp_kind, e) with
  | (FPnormal, Callconv _) ->
    Errors.inout_annotation_unexpected pos fp_pos is_variadic
  | (FPnormal, _) -> ()
  | (FPref, Callconv (kind, _)) ->
    (match kind with
    (* HHVM supports pass-by-ref for arguments annotated as 'inout'. *)
    | Ast_defs.Pinout -> ())
  | (FPref, _) -> Errors.pass_by_ref_annotation_missing pos fp_pos
  (* HHVM also allows '&' on arguments to inout parameters via interop layer. *)
  | (FPinout, Callconv (Ast_defs.Pinout, _)) -> ()
  | (FPinout, _) -> Errors.inout_annotation_missing pos fp_pos

and inout_write_back env { fp_type; _ } (_, e) =
  match e with
  | Callconv (Ast_defs.Pinout, e1) ->
    (* Translate the write-back semantics of inout parameters.
     *
     * This matters because we want to:
     * (1) make sure we can write to the original argument
     *     (modifiable lvalue check)
     * (2) allow for growing of locals / Tunions (type side effect)
     *     but otherwise unify the argument type with the parameter hint
     *)
    let (env, _te, _ty) =
      assign_ (fst e1) Reason.URparam_inout env e1 fp_type.et_type
    in
    env
  | _ -> env

and is_empty_type ty =
  match ty with
  | (_, Tunion []) -> true
  | _ -> false

(** Typechecks a call.
Returns in this order the typed expressions for the arguments, for the variadic arguments, and the return type. *)
and call
    ~(expected : ExpectedTy.t option)
    ?method_call_info
    ?nullsafe
    pos
    env
    fty
    el
    unpacked_element =
  (* Special case needed for invoking a method on the empty type *)
  if is_empty_type fty then
    (env, ([], None, fty))
  else
    let make_unpacked_traversable_ty pos ty =
      MakeType.traversable (Reason.Runpack_param pos) ty
    in
    let resl =
      TUtils.try_over_concrete_supertypes env fty (fun env fty ->
          let (env, efty) =
            Typing_solver.expand_type_and_solve
              ~description_of_expected:"a function value"
              env
              pos
              fty
              Errors.unify_error
          in
          match efty with
          (* We allow nullsafe calls on a "null" function type, in order to type check nullsafe
           * method invocation *)
          | (_, Tprim Tnull) when Option.is_some nullsafe ->
            let el =
              Option.value_map
                ~f:(fun u -> el @ [u])
                ~default:el
                unpacked_element
            in
            let (env, tel) =
              List.map_env env el (fun env elt ->
                  let expected =
                    ExpectedTy.make
                      pos
                      Reason.URparam
                      (Reason.Rnone, Typing_utils.tany env)
                  in
                  let (env, te, _) = expr ~expected env elt in
                  (env, te))
            in
            let env = call_untyped_unpack env unpacked_element in
            (env, (tel, None, efty))
          | (_, (Terr | Tany _ | Tunion [] | Tdynamic)) ->
            let el =
              Option.value_map
                ~f:(fun u -> el @ [u])
                ~default:el
                unpacked_element
            in
            let (env, tel) =
              List.map_env env el (fun env elt ->
                  let (env, te, ty) =
                    let expected =
                      ExpectedTy.make
                        pos
                        Reason.URparam
                        (Reason.Rnone, Typing_utils.tany env)
                    in
                    expr ~expected env elt
                  in
                  let env =
                    if IM.is_on @@ TCO.infer_missing (Env.get_tcopt env) then
                      match efty with
                      | (_, (Terr | Tany _ | Tdynamic)) ->
                        Typing_coercion.coerce_type
                          pos
                          Reason.URparam
                          env
                          ty
                          (MakeType.unenforced efty)
                          Errors.unify_error
                      | _ -> env
                    else
                      env
                  in
                  let env =
                    match elt with
                    | (_, Callconv (Ast_defs.Pinout, e1)) ->
                      let (env, _te, _ty) =
                        assign_ (fst e1) Reason.URparam_inout env e1 efty
                      in
                      env
                    | _ -> env
                  in
                  (env, te))
            in
            let env = call_untyped_unpack env unpacked_element in
            let ty =
              if equal_locl_ty_ (snd efty) Tdynamic then
                MakeType.dynamic (Reason.Rdynamic_call pos)
              else
                (Reason.Rnone, Typing_utils.tany env)
            in
            (env, (tel, None, ty))
          | (_, Tunion [ty]) ->
            call ~expected ?nullsafe pos env ty el unpacked_element
          | (r, Tunion tyl) ->
            let (env, resl) =
              List.map_env env tyl (fun env ty ->
                  call ~expected ?nullsafe pos env ty el unpacked_element)
            in
            let retl = List.map resl ~f:(fun (_, _, x) -> x) in
            let (env, ty) = Union.union_list env r retl in
            (* We shouldn't be picking arbitrarily for the TAST here, as TAST checks
             * depend on the types inferred. Here's we're preserving legacy behaviour
             * by picking the last one.
             * TODO: don't do this, instead use subtyping to push unions
             * through function types
             *)
            let (tel, typed_unpack_element, _) = List.hd_exn (List.rev resl) in
            (env, (tel, typed_unpack_element, ty))
          | (r, Tintersection tyl) ->
            let (env, resl) =
              TUtils.run_on_intersection env tyl ~f:(fun env ty ->
                  call ~expected ?nullsafe pos env ty el unpacked_element)
            in
            let retl = List.map resl ~f:(fun (_, _, x) -> x) in
            let (env, ty) = Inter.intersect_list env r retl in
            (* We shouldn't be picking arbitrarily for the TAST here, as TAST checks
             * depend on the types inferred. Here we're preserving legacy behaviour
             * by picking the last one.
             * TODO: don't do this, instead use subtyping to push intersections
             * through function types
             *)
            let (tel, typed_unpack_element, _) = List.hd_exn (List.rev resl) in
            (env, (tel, typed_unpack_element, ty))
          | (r2, Tfun ft) ->
            (* Typing of format string functions. It is dependent on the arguments (el)
             * so it cannot be done earlier.
             *)
            let pos_def = Reason.to_pos r2 in
            let (env, ft) = Typing_exts.retype_magic_func env ft el in
            let (env, var_param) = variadic_param env ft in
            (* Force subtype with expected result *)
            let env =
              check_expected_ty "Call result" env ft.ft_ret.et_type expected
            in
            let env = Env.set_tyvar_variance env ft.ft_ret.et_type in
            let is_lambda e =
              match snd e with
              | Efun _
              | Lfun _ ->
                true
              | _ -> false
            in
            let get_next_param_info paraml =
              match paraml with
              | param :: paraml -> (false, Some param, paraml)
              | [] -> (true, var_param, paraml)
            in
            let check_arg env ((pos, _) as e) opt_param ~is_variadic =
              match opt_param with
              | Some param ->
                let (env, te, ty) =
                  let expected =
                    ExpectedTy.make_and_allow_coercion
                      pos
                      Reason.URparam
                      param.fp_type
                  in
                  expr
                    ~accept_using_var:param.fp_accept_disposable
                    ~expected
                    env
                    e
                in
                let env = call_param env param (e, ty) ~is_variadic in
                (env, Some (te, ty))
              | None ->
                let expected =
                  ExpectedTy.make
                    pos
                    Reason.URparam
                    (Reason.Rnone, Typing_utils.tany env)
                in
                let (env, te, ty) = expr ~expected env e in
                (env, Some (te, ty))
            in
            let set_tyvar_variance_from_lambda_param env opt_param =
              match opt_param with
              | Some param ->
                let rec set_params_variance env ty =
                  let (env, ty) = Env.expand_type env ty in
                  match ty with
                  | (_, Tunion [ty]) -> set_params_variance env ty
                  | (_, Toption ty) -> set_params_variance env ty
                  | (_, Tfun { ft_params; ft_ret; _ }) ->
                    let env =
                      List.fold
                        ~init:env
                        ~f:(fun env param ->
                          Env.set_tyvar_variance env param.fp_type.et_type)
                        ft_params
                    in
                    Env.set_tyvar_variance env ft_ret.et_type ~flip:true
                  | _ -> env
                in
                set_params_variance env param.fp_type.et_type
              | None -> env
            in
            (* Given an expected function type ft, check types for the non-unpacked
             * arguments. Don't check lambda expressions if check_lambdas=false *)
            let rec check_args check_lambdas env el paraml =
              match el with
              (* We've got an argument *)
              | (e, opt_result) :: el ->
                (* Pick up next parameter type info *)
                let (is_variadic, opt_param, paraml) =
                  get_next_param_info paraml
                in
                let (env, one_result) =
                  match (check_lambdas, is_lambda e) with
                  | (false, false)
                  | (true, true) ->
                    check_arg env e opt_param ~is_variadic
                  | (false, true) ->
                    let env =
                      set_tyvar_variance_from_lambda_param env opt_param
                    in
                    (env, opt_result)
                  | (true, false) -> (env, opt_result)
                in
                let (env, rl, paraml) =
                  check_args check_lambdas env el paraml
                in
                (env, (e, one_result) :: rl, paraml)
              | [] -> (env, [], paraml)
            in
            (* First check the non-lambda arguments. For generic functions, this
             * is likely to resolve type variables to concrete types *)
            let rl = List.map el (fun e -> (e, None)) in
            let (env, rl, _) = check_args false env rl ft.ft_params in
            (* Now check the lambda arguments, hopefully with type variables resolved *)
            let (env, rl, paraml) = check_args true env rl ft.ft_params in
            (* We expect to see results for all arguments after this second pass *)
            let get_param opt =
              match opt with
              | Some x -> x
              | None -> failwith "missing parameter in check_args"
            in
            let (tel, tys) =
              let l = List.map rl (fun (_, opt) -> get_param opt) in
              List.unzip l
            in
            let env = TR.check_call env method_call_info pos r2 ft tys in
            let (env, typed_unpack_element, arity, did_unpack) =
              match unpacked_element with
              | None -> (env, None, List.length el, false)
              | Some e ->
                (* Enforces that e is unpackable. If e is a tuple, check types against
                 * parameter types *)
                let (env, te, ty) = expr env e in
                let (env, ety) =
                  Typing_solver.expand_type_and_solve
                    ~description_of_expected:"an unpackable value"
                    env
                    (fst e)
                    ty
                    Errors.unify_error
                in
                (match ety with
                | (_, Ttuple tyl) ->
                  let rec check_elements env tyl paraml =
                    match tyl with
                    | [] -> env
                    | ty :: tyl ->
                      let (is_variadic, opt_param, paraml) =
                        get_next_param_info paraml
                      in
                      (match opt_param with
                      | None -> env
                      | Some param ->
                        let env = call_param env param (e, ty) ~is_variadic in
                        check_elements env tyl paraml)
                  in
                  let env = check_elements env tyl paraml in
                  (env, Some te, List.length el + List.length tyl, false)
                | _ ->
                  let param_tyl =
                    List.map paraml (fun param -> param.fp_type)
                  in
                  let add_variadic_param_ty param_tyl =
                    match var_param with
                    | Some param -> param.fp_type :: param_tyl
                    | None -> param_tyl
                  in
                  let param_tyl = add_variadic_param_ty param_tyl in
                  let pos = fst e in
                  let env =
                    List.fold_right param_tyl ~init:env ~f:(fun param_ty env ->
                        let traversable_ty =
                          make_unpacked_traversable_ty pos param_ty.et_type
                        in
                        Typing_coercion.coerce_type
                          pos
                          Reason.URparam
                          env
                          ety
                          { et_type = traversable_ty; et_enforced = false }
                          Errors.unify_error)
                  in
                  (env, Some te, List.length el, true))
            in
            (* If we unpacked an array, we don't check arity exactly. Since each
             * unpacked array consumes 1 or many parameters, it is nonsensical to say
             * that not enough args were passed in (so we don't do the min check).
             *)
            let () = check_arity ~did_unpack pos pos_def arity ft.ft_arity in
            (* Variadic params cannot be inout so we can stop early *)
            let env = wfold_left2 inout_write_back env ft.ft_params el in
            let (env, ret_ty) =
              TR.get_adjusted_return_type env method_call_info ft.ft_ret.et_type
            in
            (env, (tel, typed_unpack_element, ret_ty))
          | (r2, Tanon (arity, id)) ->
            let (env, tel, tyl) = exprs env el in
            let expr_for_unpacked_expr_list env = function
              | None -> (env, None, None, Pos.none)
              | Some ((pos, _) as e) ->
                let (env, te, ety) = expr env e in
                (env, Some te, Some ety, pos)
            in
            let append_tuple_types tyl = function
              | Some (_, Ttuple tuple_tyl) -> tyl @ tuple_tyl
              | _ -> tyl
            in
            let determine_arity env min_arity pos = function
              | None
              | Some (_, Ttuple _) ->
                (env, Fstandard (min_arity, min_arity))
              | Some ety ->
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
                let (env, ty) = Env.fresh_type env pos in
                let traversable_ty = make_unpacked_traversable_ty pos ty in
                let env =
                  Typing_coercion.coerce_type
                    pos
                    Reason.URparam
                    env
                    ety
                    { et_type = traversable_ty; et_enforced = false }
                    Errors.unify_error
                in
                let param =
                  {
                    fp_pos = pos;
                    fp_name = None;
                    fp_type = MakeType.unenforced ty;
                    fp_kind = FPnormal;
                    fp_accept_disposable = false;
                    fp_mutability = None;
                    fp_rx_annotation = None;
                  }
                in
                (env, Fvariadic (min_arity, param))
            in
            let (env, typed_unpack_element, uety_opt, uepos) =
              expr_for_unpacked_expr_list env unpacked_element
            in
            let tyl = append_tuple_types tyl uety_opt in
            let (env, call_arity) =
              determine_arity env (List.length tyl) uepos uety_opt
            in
            let anon = Env.get_anonymous env id in
            let fpos = Reason.to_pos r2 in
            (match anon with
            | None ->
              Errors.anonymous_recursive_call pos;
              (env, (tel, typed_unpack_element, err_witness env pos))
            | Some
                {
                  rx = reactivity;
                  is_coroutine;
                  counter = ftys;
                  typecheck = anon;
                  _;
                } ->
              let () =
                check_arity pos fpos (Typing_defs.arity_min call_arity) arity
              in
              let tyl = List.map tyl TUtils.default_fun_param in
              let (env, _, ty) = anon ~el env tyl call_arity in
              let fty =
                ( Reason.Rlambda_use pos,
                  Tfun
                    {
                      ft_is_coroutine = is_coroutine;
                      ft_arity = arity;
                      ft_tparams = ([], FTKtparams);
                      ft_where_constraints = [];
                      ft_params = tyl;
                      ft_ret = MakeType.unenforced ty;
                      ft_reactive = reactivity;
                      (* TODO: record proper async lambda information *)
                      ft_fun_kind = Ast_defs.FSync;
                      ft_return_disposable = false;
                      ft_mutability = None;
                      ft_returns_mutable = false;
                      ft_returns_void_to_rx = false;
                    } )
              in
              ftys := TUtils.add_function_type env fty !ftys;
              (env, (tel, typed_unpack_element, ty)))
          | ty ->
            bad_call env pos ty;
            let env = call_untyped_unpack env unpacked_element in
            (env, ([], None, err_witness env pos)))
    in
    match resl with
    | [res] -> res
    | _ ->
      bad_call env pos fty;
      let env = call_untyped_unpack env unpacked_element in
      (env, ([], None, err_witness env pos))

and call_param env param (((pos, _) as e), arg_ty) ~is_variadic =
  param_modes ~is_variadic param e;

  (* When checking params, the type 'x' may be expression dependent. Since
   * we store the expression id in the local env for Lvar, we want to apply
   * it in this case.
   *)
  let (env, dep_ty) =
    match snd e with
    | Lvar _ -> ExprDepTy.make env (CIexpr e) arg_ty
    | _ -> (env, arg_ty)
  in
  Typing_coercion.coerce_type
    pos
    Reason.URparam
    env
    dep_ty
    param.fp_type
    Errors.unify_error

and call_untyped_unpack env unpacked_element =
  match unpacked_element with
  (* In the event that we don't have a known function call type, we can still
   * verify that any unpacked arguments (`...$args`) are something that can
   * be actually unpacked. *)
  | None -> env
  | Some e ->
    let (env, _, ety) = expr env e in
    (match ety with
    | (_, Ttuple _) -> env (* tuples are always fine *)
    | _ ->
      let pos = fst e in
      let (env, ty) = Env.fresh_type env pos in
      let unpack_r = Reason.Runpack_param pos in
      let unpack_ty = MakeType.traversable unpack_r ty in
      Typing_coercion.coerce_type
        pos
        Reason.URparam
        env
        ety
        (MakeType.unenforced unpack_ty)
        Errors.unify_error)

and bad_call env p ty = Errors.bad_call p (Typing_print.error env ty)

and make_a_local_of env e =
  match e with
  | (p, Class_get ((_, cname), CGstring (_, member_name))) ->
    let (env, local) = Env.FakeMembers.make_static env cname member_name in
    (env, Some (p, local))
  | ( p,
      Obj_get ((((_, This) | (_, Lvar _)) as obj), (_, Id (_, member_name)), _)
    ) ->
    let (env, local) = Env.FakeMembers.make env obj member_name in
    (env, Some (p, local))
  | (_, Lvar x)
  | (_, Dollardollar x) ->
    (env, Some x)
  | _ -> (env, None)

(* This function captures the common bits of logic behind refinement
 * of the type of a local variable or a class member variable as a
 * result of a dynamic check (e.g., nullity check, simple type check
 * using functions like is_int, is_string, is_array etc.).  The
 * argument refine is a function that takes the type of the variable
 * and returns a refined type (making necessary changes to the
 * environment, which is threaded through).
 *)
and refine_lvalue_type env (((_p, ty), _) as te) ~refine =
  let (env, ty) = refine env ty in
  let e = Tast.to_nast_expr te in
  let (env, localopt) = make_a_local_of env e in
  (* TODO TAST: generate an assignment to the fake local in the TAST *)
  match localopt with
  | Some local -> set_local env local ty
  | None -> env

and condition_nullity ~nonnull (env : env) te =
  match te with
  (* assignment: both the rhs and lhs of the '=' must be made null/non-null *)
  | (_, Aast.Binop (Ast_defs.Eq None, var, te)) ->
    let env = condition_nullity ~nonnull env te in
    condition_nullity ~nonnull env var
  (* case where `Shapes::idx(...)` must be made null/non-null *)
  | ( _,
      Aast.Call
        ( _,
          (_, Aast.Class_const ((_, Aast.CI (_, shapes)), (_, idx))),
          _,
          [shape; field],
          _ ) )
    when String.equal shapes SN.Shapes.cShapes && String.equal idx SN.Shapes.idx
    ->
    let field = Tast.to_nast_expr field in
    let refine env shape_ty =
      if nonnull then
        Typing_shapes.shapes_idx_not_null env shape_ty field
      else
        (env, shape_ty)
    in
    refine_lvalue_type env shape ~refine
  | ((p, _), _) ->
    let refine env ty =
      if nonnull then
        Typing_solver.non_null env p ty
      else
        let r = Reason.Rwitness (Reason.to_pos (fst ty)) in
        Inter.intersect env r ty (MakeType.null r)
    in
    refine_lvalue_type env te ~refine

and condition_isset env = function
  | (_, Aast.Array_get (x, _)) -> condition_isset env x
  | v -> condition_nullity ~nonnull:true env v

(**
 * Build an environment for the true or false branch of
 * conditional statements.
 *)
and condition
    ?lhs_of_null_coalesce env tparamet ((((p, ty) as pty), e) as te : Tast.expr)
    =
  let condition = condition ?lhs_of_null_coalesce in
  match e with
  | Aast.True
  | Aast.Expr_list []
    when not tparamet ->
    LEnv.drop_cont env C.Next
  | Aast.False when tparamet -> LEnv.drop_cont env C.Next
  | Aast.Expr_list [] -> env
  | Aast.Expr_list [x] -> condition env tparamet x
  | Aast.Expr_list (_ :: xs) -> condition env tparamet (pty, Aast.Expr_list xs)
  | Aast.Call (Cnormal, (_, Aast.Id (_, func)), _, [param], None)
    when String.equal SN.PseudoFunctions.isset func
         && tparamet
         && not (Env.is_strict env) ->
    condition_isset env param
  | Aast.Call (Cnormal, (_, Aast.Id (_, func)), _, [te], None)
    when String.equal SN.StdlibFunctions.is_null func ->
    condition_nullity ~nonnull:(not tparamet) env te
  | Aast.Binop ((Ast_defs.Eqeq | Ast_defs.Eqeqeq), (_, Aast.Null), e)
  | Aast.Binop ((Ast_defs.Eqeq | Ast_defs.Eqeqeq), e, (_, Aast.Null)) ->
    condition_nullity ~nonnull:(not tparamet) env e
  | Aast.Lvar _
  | Aast.Obj_get _
  | Aast.Class_get _
  | Aast.Binop (Ast_defs.Eq None, _, _) ->
    let (env, ety) = Env.expand_type env ty in
    (match ety with
    | (_, Tarraykind AKempty)
    | (_, Tprim Tbool) ->
      env
    | ( _,
        ( Terr | Tany _ | Tnonnull | Tarraykind _ | Toption _ | Tdynamic
        | Tprim _ | Tvar _ | Tfun _ | Tgeneric _ | Tnewtype _ | Tdependent _
        | Tclass _ | Ttuple _
        | Tanon (_, _)
        | Tunion _ | Tintersection _ | Tobject | Tshape _ | Tpu _
        | Tpu_type_access _ ) ) ->
      condition_nullity ~nonnull:tparamet env te)
  | Aast.Binop (((Ast_defs.Diff | Ast_defs.Diff2) as op), e1, e2) ->
    let op =
      if Ast_defs.(equal_bop op Diff) then
        Ast_defs.Eqeq
      else
        Ast_defs.Eqeqeq
    in
    condition env (not tparamet) (pty, Aast.Binop (op, e1, e2))
  | Aast.Id (_, s) when String.equal s SN.Rx.is_enabled ->
    (* when Rx\IS_ENABLED is false - switch env to non-reactive *)
    if not tparamet then
      Env.set_env_reactive env Nonreactive
    else
      env
  (* Conjunction of conditions. Matches the two following forms:
      if (cond1 && cond2)
      if (!(cond1 || cond2))
  *)
  | Aast.Binop (((Ast_defs.Ampamp | Ast_defs.Barbar) as bop), e1, e2)
    when Bool.equal tparamet Ast_defs.(equal_bop bop Ampamp) ->
    let env = condition env tparamet e1 in
    (* This is necessary in case there is an assignment in e2
     * We essentially redo what has been undone in the
     * `Binop (Ampamp|Barbar)` case of `expr` *)
    let (env, _, _) = expr env (Tast.to_nast_expr e2) in
    let env = condition env tparamet e2 in
    env
  (* Disjunction of conditions. Matches the two following forms:
      if (cond1 || cond2)
      if (!(cond1 && cond2))
  *)
  | Aast.Binop (((Ast_defs.Ampamp | Ast_defs.Barbar) as bop), e1, e2)
    when Bool.equal tparamet Ast_defs.(equal_bop bop Barbar) ->
    let (env, (), ()) =
      branch
        env
        (fun env ->
          (* Either cond1 is true and we don't know anything about cond2... *)
          let env = condition env tparamet e1 in
          (env, ()))
        (fun env ->
          (* ... Or cond1 is false and therefore cond2 must be true *)
          let env = condition env (not tparamet) e1 in
          (* Similarly to the conjunction case, there might be an assignment in
          cond2 which we must account for. Again we redo what has been undone in
          the `Binop (Ampamp|Barbar)` case of `expr` *)
          let (env, _, _) = expr env (Tast.to_nast_expr e2) in
          let env = condition env tparamet e2 in
          (env, ()))
    in
    env
  | Aast.Call (Cnormal, ((p, _), Aast.Id (_, f)), _, [lv], None)
    when tparamet && String.equal f SN.StdlibFunctions.is_array ->
    is_array env `PHPArray p f lv
  | Aast.Call
      ( Cnormal,
        (_, Aast.Class_const ((_, Aast.CI (_, class_name)), (_, method_name))),
        _,
        [shape; field],
        None )
    when tparamet
         && String.equal class_name SN.Shapes.cShapes
         && String.equal method_name SN.Shapes.keyExists ->
    key_exists env p shape field
  | Aast.Unop (Ast_defs.Unot, e) -> condition env (not tparamet) e
  | Aast.Is (ivar, h) when is_instance_var (Tast.to_nast_expr ivar) ->
    let ety_env =
      { (Phase.env_with_self env) with from_class = Some CIstatic }
    in
    let (env, hint_ty) = Phase.localize_hint ~ety_env env h in
    let (env, hint_ty) = Env.expand_type env hint_ty in
    let reason = Reason.Ris (fst h) in
    let refine_type env hint_ty =
      let (ivar_pos, ivar_ty) = fst ivar in
      let (env, ivar) = get_instance_var env (Tast.to_nast_expr ivar) in
      let (env, hint_ty) =
        class_for_refinement env p reason ivar_pos ivar_ty hint_ty
      in
      let (env, refined_ty) = Inter.intersect env reason ivar_ty hint_ty in
      set_local env ivar refined_ty
    in
    let (env, hint_ty) =
      if not tparamet then
        Inter.non env reason hint_ty ~approx:TUtils.ApproxUp
      else
        (env, hint_ty)
    in
    refine_type env hint_ty
  | _ -> env

(** Transform a hint like `A<_>` to a localized type like `A<T#1>` *)
and class_for_refinement env p reason ivar_pos ivar_ty hint_ty =
  match (snd ivar_ty, snd hint_ty) with
  | (_, Tclass (((_, cid) as _c), _, tyl)) ->
    begin
      match Env.get_class env cid with
      | Some class_info ->
        let (env, tparams_with_new_names, tyl_fresh) =
          generate_fresh_tparams env class_info reason tyl
        in
        safely_refine_class_type
          env
          p
          _c
          class_info
          ivar_ty
          hint_ty
          reason
          tparams_with_new_names
          tyl_fresh
      | None -> (env, (Reason.Rwitness ivar_pos, Tobject))
    end
  | (Ttuple ivar_tyl, Ttuple hint_tyl)
    when Int.equal (List.length ivar_tyl) (List.length hint_tyl) ->
    let (env, tyl) =
      List.map2_env env ivar_tyl hint_tyl (fun env ivar_ty hint_ty ->
          class_for_refinement env p reason ivar_pos ivar_ty hint_ty)
    in
    (env, (reason, Ttuple tyl))
  | ( _,
      ( Tany _ | Tprim _ | Toption _ | Ttuple _ | Tnonnull | Tshape _ | Tvar _
      | Tgeneric _ | Tnewtype _ | Tdependent _ | Tarraykind _ | Tanon _
      | Tunion _ | Tintersection _ | Tobject | Terr | Tfun _ | Tdynamic | Tpu _
      | Tpu_type_access _ ) ) ->
    (env, hint_ty)

(** If we are dealing with a refinement like
      $x is MyClass<A, B>
    then class_info is the class info of MyClass and hint_tyl corresponds
    to A, B. *)
and generate_fresh_tparams env class_info reason hint_tyl =
  let tparams_len = List.length (Cls.tparams class_info) in
  let hint_tyl = List.take hint_tyl tparams_len in
  let pad_len = tparams_len - List.length hint_tyl in
  let hint_tyl =
    List.map hint_tyl (fun x -> Some x) @ List.init pad_len (fun _ -> None)
  in
  let replace_wildcard env hint_ty tp =
    let {
      tp_name = (_, tparam_name);
      tp_reified = reified;
      tp_user_attributes;
      _;
    } =
      tp
    in
    let enforceable =
      Naming_attributes.mem SN.UserAttributes.uaEnforceable tp_user_attributes
    in
    let newable =
      Naming_attributes.mem SN.UserAttributes.uaNewable tp_user_attributes
    in
    match hint_ty with
    | Some (_, Tgeneric name) when Env.is_fresh_generic_parameter name ->
      (env, (Some (tp, name), (reason, Tgeneric name)))
    | Some ty -> (env, (None, ty))
    | None ->
      let (env, new_name) =
        Env.add_fresh_generic_parameter
          env
          tparam_name
          ~reified
          ~enforceable
          ~newable
      in
      (env, (Some (tp, new_name), (reason, Tgeneric new_name)))
  in
  let (env, tparams_and_tyl) =
    List.map2_env env hint_tyl (Cls.tparams class_info) ~f:replace_wildcard
  in
  let (tparams_with_new_names, tyl_fresh) = List.unzip tparams_and_tyl in
  (env, tparams_with_new_names, tyl_fresh)

and safely_refine_class_type
    env
    p
    class_name
    class_info
    ivar_ty
    obj_ty
    reason
    (tparams_with_new_names : (decl_tparam * string) option list)
    tyl_fresh =
  (* Type of variable in block will be class name
   * with fresh type parameters *)
  let obj_ty = (fst obj_ty, Tclass (class_name, Nonexact, tyl_fresh)) in
  let tparams = Cls.tparams class_info in
  (* Add in constraints as assumptions on those type parameters *)
  let ety_env =
    {
      type_expansions = [];
      substs = Subst.make_locl tparams tyl_fresh;
      this_ty = obj_ty;
      (* In case `this` appears in constraints *)
      from_class = None;
      quiet = true;
      on_error = Errors.unify_error_at p;
    }
  in
  let add_bounds env (t, ty_fresh) =
    List.fold_left t.tp_constraints ~init:env ~f:(fun env (ck, ty) ->
        (* Substitute fresh type parameters for
         * original formals in constraint *)
        let (env, ty) = Phase.localize ~ety_env env ty in
        SubType.add_constraint p env ck ty_fresh ty)
  in
  let env =
    List.fold_left (List.zip_exn tparams tyl_fresh) ~f:add_bounds ~init:env
  in
  (* Finally, if we have a class-test on something with static classish type,
   * then we can chase the hierarchy and decompose the types to deduce
   * further assumptions on type parameters. For example, we might have
   *   class B<Tb> { ... }
   *   class C extends B<int>
   * and have obj_ty = C and x_ty = B<T> for a generic parameter Aast.
   * Then SubType.add_constraint will deduce that T=int and add int as
   * both lower and upper bound on T in env.lenv.tpenv
   *)
  let (env, supertypes) = TUtils.get_concrete_supertypes env ivar_ty in
  let env =
    List.fold_left supertypes ~init:env ~f:(fun env ty ->
        SubType.add_constraint p env Ast_defs.Constraint_as obj_ty ty)
  in
  (* It's often the case that the fresh name isn't necessary. For
   * example, if C<T> extends B<T>, and we have $x:B<t> for some type t
   * then $x is C should refine to $x:C<t>.
   * We take a simple approach:
   *    For a fresh type parameter T#1, if
   *      - There is an eqality constraint T#1 = t,
   *      - T#1 is covariant, and T# has upper bound t (or mixed if absent)
   *      - T#1 is contravariant, and T#1 has lower bound t (or nothing if absent)
   *    then replace T#1 with t.
   * This is done in Type_parameter_env_ops.simplify_tpenv
   *)
  let (env, tparam_substs) =
    Type_parameter_env_ops.simplify_tpenv
      env
      (List.zip_exn tparams_with_new_names tyl_fresh)
      reason
  in
  let tyl_fresh =
    List.map2_exn tyl_fresh tparams_with_new_names ~f:(fun orig_ty tparam_opt ->
        match tparam_opt with
        | None -> orig_ty
        | Some (_tp, name) -> SMap.find name tparam_substs)
  in
  let obj_ty_simplified =
    (fst obj_ty, Tclass (class_name, Nonexact, tyl_fresh))
  in
  (env, obj_ty_simplified)

and is_instance_var = function
  | (_, (Lvar _ | This | Dollardollar _)) -> true
  | (_, Obj_get ((_, This), (_, Id _), _)) -> true
  | (_, Obj_get ((_, Lvar _), (_, Id _), _)) -> true
  | (_, Class_get (_, _)) -> true
  | _ -> false

and get_instance_var env = function
  | (p, Class_get ((_, cname), CGstring (_, member_name))) ->
    let (env, local) = Env.FakeMembers.make_static env cname member_name in
    (env, (p, local))
  | ( p,
      Obj_get ((((_, This) | (_, Lvar _)) as obj), (_, Id (_, member_name)), _)
    ) ->
    let (env, local) = Env.FakeMembers.make env obj member_name in
    (env, (p, local))
  | (_, Dollardollar (p, x))
  | (_, Lvar (p, x)) ->
    (env, (p, x))
  | (p, This) -> (env, (p, this))
  | _ -> failwith "Should only be called when is_instance_var is true"

(* Refine type for is_array, is_vec, is_keyset and is_dict tests
 * `pred_name` is the function name itself (e.g. 'is_vec')
 * `p` is position of the function name in the source
 * `arg_expr` is the argument to the function
 *)
and is_array env ty p pred_name arg_expr =
  refine_lvalue_type env arg_expr ~refine:(fun env arg_ty ->
      let r = Reason.Rpredicated (p, pred_name) in
      let (env, tarrkey_name) =
        Env.add_fresh_generic_parameter
          env
          "Tk"
          ~reified:Erased
          ~enforceable:false
          ~newable:false
      in
      let tarrkey = (r, Tgeneric tarrkey_name) in
      let env =
        SubType.add_constraint
          p
          env
          Ast_defs.Constraint_as
          tarrkey
          (MakeType.arraykey r)
      in
      let (env, tfresh_name) =
        Env.add_fresh_generic_parameter
          env
          "T"
          ~reified:Erased
          ~enforceable:false
          ~newable:false
      in
      let tfresh = (r, Tgeneric tfresh_name) in
      (* This is the refined type of e inside the branch *)
      let refined_ty =
        match ty with
        | `HackDict -> MakeType.dict r tarrkey tfresh
        | `HackVec -> MakeType.vec r tfresh
        | `HackKeyset -> MakeType.keyset r tarrkey
        | `PHPArray ->
          let safe_isarray_enabled =
            TypecheckerOptions.experimental_feature_enabled
              (Env.get_tcopt env)
              TypecheckerOptions.experimental_isarray
          in
          let tk =
            MakeType.arraykey Reason.(Rvarray_or_darray_key (to_pos r))
          in
          let tv =
            if safe_isarray_enabled then
              tfresh
            else
              (r, TUtils.tany env)
          in
          (r, Tarraykind (AKvarray_or_darray (tk, tv)))
      in
      (* Add constraints on generic parameters that must
       * hold for refined_ty <:arg_ty. For example, if arg_ty is Traversable<T>
       * and refined_ty is keyset<T#1> then we know T#1 <: T *)
      let env =
        SubType.add_constraint p env Ast_defs.Constraint_as refined_ty arg_ty
      in
      (env, refined_ty))

and key_exists env pos shape field =
  let field = Tast.to_nast_expr field in
  refine_lvalue_type env shape ~refine:(fun env shape_ty ->
      match TUtils.shape_field_name env field with
      | None -> (env, shape_ty)
      | Some field_name ->
        Typing_shapes.refine_shape field_name pos env shape_ty)

and string2 env idl =
  let (env, tel) =
    List.fold_left idl ~init:(env, []) ~f:(fun (env, tel) x ->
        let (env, te, ty) = expr env x in
        let p = fst x in
        let env = Typing_substring.sub_string p env ty in
        (env, te :: tel))
  in
  (env, List.rev tel)

(* If the current class inherits from classes that take type arguments, we need
 * to check that the arguments provided are consistent with the constraints on
 * the type parameters. *)
and check_implements_tparaml (env : env) ht =
  let (_r, (_, c), paraml) = TUtils.unwrap_class_type ht in
  let class_ = Env.get_class_dep env c in
  match class_ with
  | None ->
    (* The class lives in PHP land *)
    ()
  | Some class_ ->
    let subst = Inst.make_subst (Cls.tparams class_) paraml in
    iter2_shortest
      begin
        fun t ty ->
        let ty_pos = Reason.to_pos (fst ty) in
        List.iter t.tp_constraints (fun (ck, cstr) ->
            (* Constraint might contain uses of generic type parameters *)
            let cstr = Inst.instantiate subst cstr in
            match ck with
            | Ast_defs.Constraint_as ->
              Type.sub_type_decl ty_pos Reason.URnone env ty cstr
            | Ast_defs.Constraint_eq ->
              (* This code could well be unreachable, because we don't allow
               * equality constraints on class generics. *)
              Type.sub_type_decl ty_pos Reason.URnone env ty cstr;
              Type.sub_type_decl ty_pos Reason.URnone env cstr ty
            | Ast_defs.Constraint_super ->
              Type.sub_type_decl ty_pos Reason.URnone env cstr ty)
      end
      (Cls.tparams class_)
      paraml

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

and check_const_trait_members pos env use_list =
  let (_, trait, _) = Decl_utils.unwrap_class_hint use_list in
  match Env.get_class env trait with
  | Some c when Ast_defs.(equal_class_kind (Cls.kind c) Ctrait) ->
    Sequence.iter (Cls.props c) (fun (x, ce) ->
        if not ce.ce_const then Errors.trait_prop_const_class pos x)
  | _ -> ()

and shallow_decl_enabled () =
  TCO.shallow_class_decl (Global_naming_options.get ())

and class_def tcopt c =
  let env = EnvFromDef.class_env tcopt c in
  let tc = Env.get_class env (snd c.c_name) in
  let env = Env.set_env_pessimize env in
  add_decl_errors Option.(map tc (fun tc -> value_exn (Cls.decl_errors tc)));
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
    if shallow_decl_enabled () then Typing_inheritance.check_class env tc;
    Some (class_def_ env c tc)

(* The two following functions enable us to retrieve the function (or class)
  header from the shared mem. Note that they only return a non None value if
  global inference is on *)
and get_decl_method_header tcopt cls method_id =
  let is_global_inference_on = IM.global_inference @@ TCO.infer_missing tcopt in
  if is_global_inference_on then
    match Cls.get_method cls method_id with
    | Some { ce_type = (lazy (_, Tfun fun_type)); _ } -> Some fun_type
    | _ -> None
  else
    None

and get_decl_function_header env function_id =
  let is_global_inference_on =
    IM.global_inference @@ TCO.infer_missing (Env.get_tcopt env)
  in
  if is_global_inference_on then
    match Decl_provider.get_fun (Env.get_ctx env) function_id with
    | Some { fe_type = (_, Tfun fun_type); _ } -> Some fun_type
    | _ -> None
  else
    None

and class_def_ env c tc =
  let env =
    let kind =
      match c.c_kind with
      | Ast_defs.Cenum -> SN.AttributeKinds.enum
      | _ -> SN.AttributeKinds.cls
    in
    Typing_attributes.check_def env new_object kind c.c_user_attributes
  in
  if
    Ast_defs.(equal_class_kind c.c_kind Cnormal)
    && not (shallow_decl_enabled ())
  then (
    (* This check is only for eager mode. The same check is performed
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
      (* `ce_override` is set in Decl when we determine that an
       * override_per_trait error needs emit. This emission is deferred
       * until Typing to ensure that this method has been added to
       * Decl_heap *)
      if ce.ce_override then
        let pos = method_pos ~is_static ce.ce_origin id in
        Errors.override_per_trait c.c_name id pos
    in
    Sequence.iter (Cls.methods tc) (check_override ~is_static:false);
    Sequence.iter (Cls.smethods tc) (check_override ~is_static:true)
  );
  let env =
    {
      env with
      inside_ppl_class =
        Naming_attributes.mem
          SN.UserAttributes.uaProbabilisticModel
          c.c_user_attributes;
    }
  in
  let (pc, _) = c.c_name in
  let impl =
    List.map
      (c.c_extends @ c.c_implements @ c.c_uses)
      (Decl_hint.hint env.decl_env)
  in
  let c_tparam_list : decl_tparam list =
    List.map
      c.c_tparams.c_tparam_list
      ~f:(Decl_hint.aast_tparam_to_decl_tparam env.decl_env)
  in
  let (env, constraints) =
    Phase.localize_generic_parameters_with_bounds
      env
      c_tparam_list
      ~ety_env:(Phase.env_with_self env)
  in
  let env = SubType.add_constraints (fst c.c_name) env constraints in
  let env =
    Phase.localize_where_constraints
      ~ety_env:(Phase.env_with_self env)
      env
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
  List.iter impl (check_implements_tparaml env);
  let check_where_constraints env ht =
    let (_, (p, _), _) = TUtils.unwrap_class_type ht in
    let (env, locl_ty) = Phase.localize_with_self env ht in
    match TUtils.get_base_type env locl_ty with
    | (_, Tclass (cls, _, tyl)) ->
      (match Env.get_class env (snd cls) with
      | Some cls when not (List.is_empty (Cls.where_constraints cls)) ->
        let tc_tparams = Cls.tparams cls in
        let ety_env =
          {
            (Phase.env_with_self env) with
            substs = Subst.make_locl tc_tparams tyl;
          }
        in
        ignore
          (Phase.check_where_constraints
             ~in_class:true
             ~use_pos:pc
             ~definition_pos:p
             ~ety_env
             env
             (Cls.where_constraints cls))
      | _ -> ())
    | _ -> ()
  in
  List.iter impl (check_where_constraints env);
  check_parent env c tc;
  check_parents_sealed env c tc;

  let is_final = Cls.final tc in
  if
    (Ast_defs.(equal_class_kind (Cls.kind tc) Cnormal) || is_final)
    && Cls.members_fully_known tc
  then (
    check_extend_abstract_meth ~is_final pc (Cls.methods tc);
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

  (* get a map of method names to list of traits from which they were removed *)
  let alist =
    List.map c.c_method_redeclarations ~f:(fun m ->
        let (_, name) = m.mt_method in
        let (_, trait, _) = Decl_utils.unwrap_class_hint m.mt_trait in
        (name, trait))
  in
  let removals =
    String.Map.of_alist_fold alist ~init:[] ~f:(Fn.flip List.cons)
  in
  let env =
    List.fold ~init:env impl ~f:(fun env ->
        class_implements_type env c removals)
  in
  let env =
    List.fold
      c.c_method_redeclarations
      ~init:env
      ~f:(supertype_redeclared_method tc)
  in
  if Cls.is_disposable tc then
    List.iter
      (c.c_extends @ c.c_uses)
      (Typing_disposable.enforce_is_disposable env);
  let (env, typed_vars) =
    List.map_env env vars (class_var_def ~is_static:false)
  in
  let typed_method_redeclarations = [] in
  let (typed_methods, typed_methods_global_tvenv) =
    List.filter_map methods (method_def env tc) |> List.unzip
  in
  let (env, typed_typeconsts) = List.map_env env c.c_typeconsts typeconst_def in
  let (env, consts) = List.map_env env c.c_consts class_const_def in
  let (typed_consts, const_types) = List.unzip consts in
  let env = Typing_enum.enum_class_check env tc c.c_consts const_types in
  let typed_constructor = class_constr_def env tc constructor in
  let env = Env.set_static env in
  let (env, typed_static_vars) =
    List.map_env env static_vars (class_var_def ~is_static:true)
  in
  let (typed_static_methods, typed_static_methods_global_tvenv) =
    List.filter_map static_methods (method_def env tc) |> List.unzip
  in
  let (env, file_attrs) = file_attributes env c.c_file_attributes in
  let (methods, constr_global_tvenv) =
    match typed_constructor with
    | None -> (typed_static_methods @ typed_methods, [])
    | Some (m, global_tvenv) ->
      ((m :: typed_static_methods) @ typed_methods, [global_tvenv])
  in
  let pu_enums =
    try List.map c.c_pu_enums ~f:(pu_enum_def env (snd c.c_name))
    with InvalidPocketUniverse -> []
  in
  let (env, tparams) = class_type_param env c.c_tparams in
  let (env, user_attributes) =
    List.map_env env c.c_user_attributes user_attribute
  in
  let env =
    Typing_solver.solve_all_unsolved_tyvars env Errors.bad_class_typevar
  in
  let env = Typing_solver.expand_bounds_of_global_tyvars env in
  ( {
      Aast.c_span = c.c_span;
      Aast.c_annotation = Env.save (Env.get_tpenv env) env;
      Aast.c_mode = c.c_mode;
      Aast.c_final = c.c_final;
      Aast.c_is_xhp = c.c_is_xhp;
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
      Aast.c_method_redeclarations = typed_method_redeclarations;
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
    },
    typed_methods_global_tvenv
    @ typed_static_methods_global_tvenv
    @ constr_global_tvenv )

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
    let (lazy (static_element_reason, _)) = static_element.ce_type in
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
    let (lazy (dyn_element_reason, _)) = dyn_element.ce_type in
    Errors.dynamic_redeclared_as_static
      static_pos
      (Reason.to_pos dyn_element_reason)
      element_name
      ~elt_type

and check_extend_abstract_meth ~is_final p seq =
  Sequence.iter seq (fun (x, ce) ->
      match ce.ce_type with
      | (lazy (r, Tfun _)) when ce.ce_abstract ->
        Errors.implement_abstract ~is_final p (Reason.to_pos r) "method" x
      | _ -> ())

and check_extend_abstract_prop ~is_final p seq =
  Sequence.iter seq (fun (x, ce) ->
      if ce.ce_abstract then
        let ce_pos = Lazy.force ce.ce_type |> fst |> Reason.to_pos in
        Errors.implement_abstract ~is_final p ce_pos "property" x)

(* Type constants must be bound to a concrete type for non-abstract classes.
 *)
and check_extend_abstract_typeconst ~is_final p seq =
  Sequence.iter seq (fun (x, tc) ->
      if Option.is_none tc.ttc_type then
        Errors.implement_abstract
          ~is_final
          p
          (fst tc.ttc_name)
          "type constant"
          x)

and check_extend_abstract_const ~is_final p seq =
  Sequence.iter seq (fun (x, cc) ->
      if cc.cc_abstract && not cc.cc_synthesized then
        let cc_pos = Reason.to_pos (fst cc.cc_type) in
        Errors.implement_abstract ~is_final p cc_pos "constant" x)

and typeconst_def
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
  let (env, cstr) = opt Phase.localize_hint_with_self env c_tconst_constraint in
  let (env, ty) = opt Phase.localize_hint_with_self env hint in
  let check t c =
    Type.sub_type pos Reason.URtypeconst_cstr env t c Errors.unify_error
  in
  ignore (Option.map2 ty cstr ~f:check);
  let env =
    match hint with
    | Some (pos, Hshape { nsi_field_map; _ }) ->
      let get_name sfi = sfi.sfi_name in
      check_shape_keys_validity env pos (List.map ~f:get_name nsi_field_map)
    | _ -> env
  in
  let env =
    Typing_attributes.check_def
      env
      new_object
      SN.AttributeKinds.typeconst
      c_tconst_user_attributes
  in
  let (env, user_attributes) =
    List.map_env env c_tconst_user_attributes user_attribute
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
    { pu_name; pu_is_final; pu_case_types; pu_case_values; pu_members; _ } =
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

    Note: Structural correctness (mostly uniqueness and exhaustivity)
    is checked during Nast check.
    If the Nast check fails, this function might raise InvalidPocketUniverse,
    that we silently ignore not to spam with duplicated errors.

    Here we check that all definitions are well-typed.
 *)
  let pos = fst pu_name in
  let cls = Decl_provider.get_class (Env.get_ctx env) c_name in
  let pu_enum =
    Option.bind cls ~f:(fun cls -> Cls.get_pu_enum cls (snd pu_name))
  in
  let make_ty_tparam (sid, reified) =
    {
      tp_variance = Ast_defs.Invariant;
      tp_name = sid;
      tp_constraints = [];
      tp_reified = reified;
      tp_user_attributes = [];
    }
  in
  let make_aast_tparam (sid, hint) =
    let hint_ty = Decl_hint.hint env.decl_env hint in
    {
      tp_variance = Ast_defs.Invariant;
      tp_name = sid;
      tp_constraints = [(Ast_defs.Constraint_eq, hint_ty)];
      tp_reified = Aast.Erased;
      tp_user_attributes = [];
    }
  in
  let (env, constraints) =
    let (env, constraints) =
      Phase.localize_generic_parameters_with_bounds
        env
        ~ety_env:(Phase.env_with_self env)
        (List.map ~f:make_ty_tparam pu_case_types)
    in
    List.iter pu_case_values ~f:(fun (_sid, hint) ->
        ignore (Phase.localize_hint_with_self env hint : _ * _));
    (env, constraints)
  in
  let env = SubType.add_constraints pos env constraints in
  let members =
    let process_member pum =
      let (env, cstrs) =
        let pum_types = List.map ~f:make_aast_tparam pum.pum_types in
        Phase.localize_generic_parameters_with_bounds
          env
          ~ety_env:(Phase.env_with_self env)
          pum_types
      in
      let env = SubType.add_constraints (fst pum.pum_atom) env cstrs in
      let process_mapping (sid, map_expr) =
        let (env, ty, expected) =
          let equal ((_, s1) : sid) ((_, s2) : sid) = String.equal s1 s2 in
          let (env, ty) =
            match List.Assoc.find ~equal pu_case_values sid with
            | None ->
              ((* Check in parent hierarchy *)
              match pu_enum with
              | None -> raise InvalidPocketUniverse
              | Some pu_enum ->
                (match SMap.find_opt (snd sid) pu_enum.tpu_case_values with
                | None -> raise InvalidPocketUniverse
                | Some (_, decl_ty) -> Phase.localize_with_self env decl_ty))
            | Some hint -> Phase.localize_hint_with_self env hint
          in
          (env, ty, Some (ExpectedTy.make (fst sid) Reason.URhint ty))
        in
        let (_env, expr, ty') = expr ?expected env map_expr in
        ignore
          (Typing_ops.sub_type
             (fst sid)
             Reason.URhint
             env
             ty'
             ty
             Errors.pocket_universes_typing);
        (sid, expr)
      in
      {
        Aast.pum_atom = pum.pum_atom;
        Aast.pum_types = pum.pum_types;
        Aast.pum_exprs = List.map ~f:process_mapping pum.pum_exprs;
      }
    in
    List.map ~f:process_member pu_members
  in
  let local_tpenv = Env.get_tpenv env in
  {
    Aast.pu_annotation = Env.save local_tpenv env;
    Aast.pu_name;
    Aast.pu_is_final;
    Aast.pu_case_types;
    Aast.pu_case_values;
    Aast.pu_members = members;
  }

and class_const_def env cc =
  let { cc_type = h; cc_id = id; cc_expr = e; _ } = cc in
  let (env, ty, opt_expected) =
    match h with
    | None ->
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
      let (env, te, ty') = expr ?expected:opt_expected env e in
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

and class_implements_type env c1 removals ctype2 =
  let params =
    List.map c1.c_tparams.c_tparam_list (fun { tp_name = (p, s); _ } ->
        (Reason.Rwitness p, Tgeneric s))
  in
  let r = Reason.Rwitness (fst c1.c_name) in
  let ctype1 = (r, Tapply (c1.c_name, params)) in
  Typing_extends.check_implements env removals ctype2 ctype1

(* Type-check a property declaration, with optional initializer *)
and class_var_def ~is_static env cv =
  (* First pick up and localize the hint if it exists *)
  let (env, expected) =
    match cv.cv_type with
    | None -> (env, None)
    | Some ((p, _) as cty) ->
      let decl_cty = Decl_hint.hint env.decl_env cty in
      let decl_cty = Typing_enforceability.compute_enforced_ty env decl_cty in
      let (env, cty) =
        Phase.localize_possibly_enforced_with_self env decl_cty
      in
      (env, Some (ExpectedTy.make_and_allow_coercion p Reason.URhint cty))
  in
  (* Next check the expression, passing in expected type if present *)
  let (env, typed_cv_expr) =
    match cv.cv_expr with
    | None -> (env, None)
    | Some e ->
      let (env, te, ty) = expr ?expected env e in
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
      Typing_attributes.check_def
        env
        new_object
        SN.AttributeKinds.staticProperty
        cv.cv_user_attributes
    else
      Typing_attributes.check_def
        env
        new_object
        SN.AttributeKinds.instProperty
        cv.cv_user_attributes
  in
  let (env, user_attributes) =
    List.map_env env cv.cv_user_attributes user_attribute
  in
  if
    Option.is_none cv.cv_type
    && Partial.should_check_error (Env.get_mode env) 2001
  then
    Errors.prop_without_typehint
      (string_of_visibility cv.cv_visibility)
      cv.cv_id;
  ( env,
    {
      Aast.cv_final = cv.cv_final;
      Aast.cv_xhp_attr = cv.cv_xhp_attr;
      Aast.cv_abstract = cv.cv_abstract;
      Aast.cv_visibility = cv.cv_visibility;
      Aast.cv_type = cv.cv_type;
      Aast.cv_id = cv.cv_id;
      Aast.cv_expr = typed_cv_expr;
      Aast.cv_user_attributes = user_attributes;
      Aast.cv_is_promoted_variadic = cv.cv_is_promoted_variadic;
      Aast.cv_doc_comment = cv.cv_doc_comment;
      (* Can make None to save space *)
      Aast.cv_is_static = is_static;
      Aast.cv_span = cv.cv_span;
    } )

and supertype_redeclared_method tc env m =
  let (pos, name) = m.mt_name in
  let get_method =
    if m.mt_static then
      Env.get_static_member
    else
      Env.get_member
  in
  let class_member_opt = get_method true env tc name in
  let (_, trait, _) = Decl_utils.unwrap_class_hint m.mt_trait in
  let (_, trait_method) = m.mt_method in
  Option.(
    let trait_member_opt =
      Env.get_class env trait >>= fun trait_tc ->
      get_method true env trait_tc trait_method
    in
    map2 trait_member_opt class_member_opt ~f:(fun trait_member class_member ->
        match (trait_member.ce_type, class_member.ce_type) with
        | ((lazy (r_child, Tfun ft_child)), (lazy (r_parent, Tfun ft_parent)))
          ->
          let ety_env = Phase.env_with_self env ~quiet:true in
          let (env, ft_child) =
            Phase.localize_ft
              ~ety_env
              ~def_pos:(Reason.to_pos r_child)
              env
              ft_child
          in
          let (env, ft_parent) =
            Phase.localize_ft
              ~ety_env
              ~def_pos:(Reason.to_pos r_parent)
              env
              ft_parent
          in
          Typing_subtype.(
            subtype_method
              ~check_return:true
              ~extra_info:
                { method_info = None; class_ty = None; parent_class_ty = None }
              env
              r_child
              ft_child
              r_parent
              ft_parent)
            (fun ?code:_ errorl ->
              Errors.bad_method_override pos name errorl Errors.unify_error)
        | _ -> env)
    |> Option.value ~default:env)

and file_attributes env file_attrs =
  let uas = List.concat_map ~f:(fun fa -> fa.fa_user_attributes) file_attrs in
  let env =
    Typing_attributes.check_def env new_object SN.AttributeKinds.file uas
  in
  List.map_env env file_attrs (fun env fa ->
      let (env, user_attributes) =
        List.map_env env fa.fa_user_attributes user_attribute
      in
      ( env,
        {
          Aast.fa_user_attributes = user_attributes;
          Aast.fa_namespace = fa.fa_namespace;
        } ))

and user_attribute env ua =
  let (env, typed_ua_params) =
    List.map_env env ua.ua_params (fun env e ->
        let (env, te, _) = expr env e in
        (env, te))
  in
  (env, { Aast.ua_name = ua.ua_name; Aast.ua_params = typed_ua_params })

and reify_kind = function
  | Erased -> Aast.Erased
  | SoftReified -> Aast.SoftReified
  | Reified -> Aast.Reified

and type_param env t =
  let env =
    Typing_attributes.check_def
      env
      new_object
      SN.AttributeKinds.typeparam
      t.tp_user_attributes
  in
  let (env, user_attributes) =
    List.map_env env t.tp_user_attributes user_attribute
  in
  ( env,
    {
      Aast.tp_variance = t.tp_variance;
      Aast.tp_name = t.tp_name;
      Aast.tp_constraints = t.tp_constraints;
      Aast.tp_reified = reify_kind t.tp_reified;
      Aast.tp_user_attributes = user_attributes;
    } )

and class_type_param env ct =
  let (env, tparam_list) = List.map_env env ct.c_tparam_list type_param in
  ( env,
    {
      Aast.c_tparam_list = tparam_list;
      Aast.c_tparam_constraints =
        SMap.map (Tuple.T2.map_fst ~f:reify_kind) ct.c_tparam_constraints;
    } )

(* If the localized types of the return type is a tyvar we force it to be covariant.
  The same goes for parameter types, but this time we force them to be contravariant
*)
and set_tyvars_variance_in_callable env return_ty param_tys =
  let env =
    match return_ty with
    | (_, Tvar v) -> Env.set_tyvar_appears_covariantly env v
    | _ -> env
  in
  List.fold
    ~init:env
    ~f:(fun env -> function
      | (_, Tvar v) -> Env.set_tyvar_appears_contravariantly env v
      | _ -> env)
    param_tys

(* During the decl phase we can, for global inference, add "improved type hints".
   That is we can say that some missing type hints are in fact global tyvars.
   In that case to get the real type hint we must merge the type hint present
   in the ast with the one we created during the decl phase. This function does
   exactly this for the return type, the parameters and the variadic parameters.
   *)
and merge_decl_header_with_hints ~params ~ret ~variadic decl_header env =
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

and method_def env cls m =
  with_timeout env m.m_name ~do_:(fun env ->
      (* reset the expression dependent display ids for each method body *)
      Reason.expr_display_id_map := IMap.empty;
      let decl_header =
        get_decl_method_header (Env.get_tcopt env) cls (snd m.m_name)
      in
      let pos = fst m.m_name in
      let env = Env.open_tyvars env (fst m.m_name) in
      let env = Env.reinitialize_locals env in
      let env = Env.set_env_function_pos env pos in
      let env =
        Typing_attributes.check_def
          env
          new_object
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
      let ety_env =
        { (Phase.env_with_self env) with from_class = Some CIstatic }
      in
      let m_tparams : decl_tparam list =
        List.map
          m.m_tparams
          ~f:(Decl_hint.aast_tparam_to_decl_tparam env.decl_env)
      in
      let (env, constraints) =
        Phase.localize_generic_parameters_with_bounds env m_tparams ~ety_env
      in
      let env = SubType.add_constraints pos env constraints in
      let env =
        Phase.localize_where_constraints ~ety_env env m.m_where_constraints
      in
      let env =
        if Env.is_static env then
          env
        else
          Env.set_local env this (Env.get_self env)
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
      let (decl_ty, params_decl_ty, variadicity_decl_ty) =
        merge_decl_header_with_hints
          ~params:m.m_params
          ~ret:m.m_ret
          ~variadic:m.m_variadic
          decl_header
          env
      in
      let env = Env.set_fn_kind env m.m_fun_kind in
      let (env, locl_ty) =
        match decl_ty with
        | None ->
          Typing_return.make_default_return
            ~is_method:true
            ~is_infer_missing_on:
              (IM.can_infer_return @@ TCO.infer_missing (Env.get_tcopt env))
            env
            m.m_name
        | Some ret ->
          (* If a 'this' type appears it needs to be compatible with the
           * late static type
           *)
          let ety_env =
            { (Phase.env_with_self env) with from_class = Some CIstatic }
          in
          Typing_return.make_return_type (Phase.localize ~ety_env) env ret
      in
      let return =
        Typing_return.make_info
          m.m_fun_kind
          m.m_user_attributes
          env
          ~is_explicit:(Option.is_some (hint_of_type_hint m.m_ret))
          locl_ty
          decl_ty
      in
      let (env, param_tys) =
        List.zip_exn m.m_params params_decl_ty
        |> List.map_env env ~f:(fun env (param, hint) ->
               make_param_local_ty env hint param)
      in
      let partial_callback = Partial.should_check_error (Env.get_mode env) in
      let param_fn p t = check_param env p t partial_callback in
      List.iter2_exn ~f:param_fn m.m_params param_tys;
      Typing_memoize.check_method env m;
      let (env, typed_params) =
        List.map_env env (List.zip_exn param_tys m.m_params) bind_param
      in
      let (env, t_variadic) =
        get_callable_variadicity
          ~partial_callback
          ~pos
          env
          variadicity_decl_ty
          m.m_variadic
      in
      let env = set_tyvars_variance_in_callable env locl_ty param_tys in
      let nb = Nast.assert_named_body m.m_body in
      let local_tpenv = Env.get_tpenv env in
      let disable =
        Naming_attributes.mem
          SN.UserAttributes.uaDisableTypecheckerInternal
          m.m_user_attributes
      in
      let (env, tb) =
        fun_ ~abstract:m.m_abstract ~disable env return pos nb m.m_fun_kind
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
      let annotation =
        if Nast.named_body_is_unsafe nb then
          Tast.HasUnsafeBlocks
        else
          Tast.NoUnsafeBlocks
      in
      let (env, tparams) = List.map_env env m.m_tparams type_param in
      let (env, user_attributes) =
        List.map_env env m.m_user_attributes user_attribute
      in
      let env =
        Typing_solver.close_tyvars_and_solve env Errors.bad_method_typevar
      in
      let env =
        Typing_solver.solve_all_unsolved_tyvars env Errors.bad_method_typevar
      in
      let env = Typing_solver.expand_bounds_of_global_tyvars env in
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
          Aast.m_fun_kind = m.m_fun_kind;
          Aast.m_user_attributes = user_attributes;
          Aast.m_ret = (locl_ty, hint_of_type_hint m.m_ret);
          Aast.m_body = { Aast.fb_ast = tb; fb_annotation = annotation };
          Aast.m_external = m.m_external;
          Aast.m_doc_comment = m.m_doc_comment;
        }
      in
      ( Typing_lambda_ambiguous.suggest_method_def env method_def,
        (pos, env.global_tvenv) ))

and typedef_def tcopt typedef =
  let env = EnvFromDef.typedef_env tcopt typedef in
  let tdecl = Env.get_typedef env (snd typedef.t_name) in
  add_decl_errors
    Option.(map tdecl (fun tdecl -> value_exn tdecl.td_decl_errors));
  let t_tparams : decl_tparam list =
    List.map
      typedef.t_tparams
      ~f:(Decl_hint.aast_tparam_to_decl_tparam env.decl_env)
  in
  let (env, constraints) =
    Phase.localize_generic_parameters_with_bounds
      env
      t_tparams
      ~ety_env:(Phase.env_with_self env)
  in
  let env = SubType.add_constraints (fst typedef.t_name) env constraints in
  NastCheck.typedef env typedef;
  let {
    t_annotation = ();
    t_name = (t_pos, _);
    t_tparams = _;
    t_constraint = tcstr;
    t_kind = hint;
    t_user_attributes = _;
    t_vis = _;
    t_mode = _;
    t_namespace = _;
  } =
    typedef
  in
  let ty = Decl_hint.hint env.decl_env hint in
  let (env, ty) = Phase.localize_with_self ~pos:t_pos env ty in
  let env =
    match tcstr with
    | Some tcstr ->
      let cstr = Decl_hint.hint env.decl_env tcstr in
      let (env, cstr) = Phase.localize_with_self ~pos:t_pos env cstr in
      Typing_ops.sub_type
        t_pos
        Reason.URnewtype_cstr
        env
        ty
        cstr
        Errors.newtype_alias_must_satisfy_constraint
    | _ -> env
  in
  let env =
    match hint with
    | (pos, Hshape { nsi_allows_unknown_fields = _; nsi_field_map }) ->
      let get_name sfi = sfi.sfi_name in
      check_shape_keys_validity env pos (List.map ~f:get_name nsi_field_map)
    | _ -> env
  in
  let env =
    Typing_attributes.check_def
      env
      new_object
      SN.AttributeKinds.typealias
      typedef.t_user_attributes
  in
  let (env, tparams) = List.map_env env typedef.t_tparams type_param in
  let (env, user_attributes) =
    List.map_env env typedef.t_user_attributes user_attribute
  in
  {
    Aast.t_annotation = Env.save (Env.get_tpenv env) env;
    Aast.t_name = typedef.t_name;
    Aast.t_mode = typedef.t_mode;
    Aast.t_vis = typedef.t_vis;
    Aast.t_user_attributes = user_attributes;
    Aast.t_constraint = typedef.t_constraint;
    Aast.t_kind = typedef.t_kind;
    Aast.t_tparams = tparams;
    Aast.t_namespace = typedef.t_namespace;
  }

and gconst_def tcopt cst =
  let env = EnvFromDef.gconst_env tcopt cst in
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
        expr ~expected env value
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
      let (env, te, _value_type) = expr env value in
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

(* Calls the method of a class, but allows the f callback to override the
 * return value type *)
and overload_function
    make_call fpos p env (cpos, class_id) method_id el unpacked_element f =
  let (env, _tal, tcid, ty) =
    static_class_id ~check_constraints:false cpos env [] class_id
  in
  let (env, _tel, _) = exprs env el in
  let (env, (fty, tal)) =
    class_get
      ~is_method:true
      ~is_const:false
      ~coerce_from_ty:None
      env
      ty
      method_id
      class_id
  in
  let (env, (tel, typed_unpack_element, res)) =
    call ~expected:None p env fty el unpacked_element
  in
  let (env, ty) = f env fty res el in
  let (env, fty) = Env.expand_type env fty in
  let fty =
    match fty with
    | (r, Tfun ft) -> (r, Tfun { ft with ft_ret = MakeType.unenforced ty })
    | _ -> fty
  in
  let te = Tast.make_typed_expr fpos fty (Aast.Class_const (tcid, method_id)) in
  make_call env te tal tel typed_unpack_element ty

and update_array_type ?lhs_of_null_coalesce p env e1 e2 valkind =
  let type_mapper =
    Typing_arrays.update_array_type p ~is_map:(Option.is_some e2)
  in
  match valkind with
  | `lvalue
  | `lvalue_subexpr ->
    let (env, te1, ty1) =
      raw_expr ~valkind:`lvalue_subexpr ~check_defined:true env e1
    in
    let (env, ty1) = type_mapper env ty1 in
    begin
      match e1 with
      | (_, Lvar (_, x)) ->
        (* type_mapper has updated the type in ty1 typevars, but we
             need to update the local variable type too *)
        let env = set_valid_rvalue p env x ty1 in
        (env, te1, ty1)
      | _ -> (env, te1, ty1)
    end
  | _ -> raw_expr ?lhs_of_null_coalesce env e1

and class_get_pu ?from_class env ty name =
  match class_get_pu_ env ty name with
  | (env, None) -> (env, None)
  | (env, Some (this_ty, substs, et)) ->
    let ety_env =
      {
        type_expansions = [];
        this_ty;
        substs;
        from_class;
        quiet = false;
        on_error = Errors.unify_error;
      }
    in
    (env, Some (ety_env, et))

and class_get_pu_ env cty name =
  let (env, cty) = Env.expand_type env cty in
  match snd cty with
  | Tany _
  | Terr
  | Tdynamic
  | Tunion _
  | Tgeneric _ ->
    (env, None)
  | Tvar _
  | Tnonnull
  | Tarraykind _
  | Toption _
  | Tprim _
  | Tfun _
  | Ttuple _
  | Tanon (_, _)
  | Tobject
  | Tshape _ ->
    (env, None)
  | Tintersection _ -> (env, None)
  | Tpu_type_access _
  | Tpu _ ->
    (env, None)
  | Tnewtype (_, _, ty)
  | Tdependent (_, ty) ->
    class_get_pu_ env ty name
  | Tclass ((_, c), _, paraml) ->
    let class_ = Env.get_class env c in
    begin
      match class_ with
      | None -> (env, None)
      | Some class_ ->
        (match Env.get_pu_enum env class_ name with
        | Some et ->
          (env, Some (cty, Subst.make_locl (Cls.tparams class_) paraml, et))
        | None -> (env, None))
    end

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
    let (env, te, ty) = expr ~expected env e in
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
      let inherited_fields = all_record_fields env parent_rd in
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

let record_def_def tcopt rd =
  let env = EnvFromDef.record_def_env tcopt rd in
  (match rd.rd_extends with
  | Some parent -> record_def_parent env rd parent
  | None -> ());

  check_record_inheritance_cycle env rd.rd_name;

  let (env, attributes) =
    List.map_env env rd.rd_user_attributes user_attribute
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

let nast_to_tast ~(do_tast_checks : bool) opts nast =
  let convert_def = function
    | Fun f ->
      begin
        match fun_def opts f with
        | Some (f, _) -> Aast.Fun f
        | None ->
          failwith
          @@ Printf.sprintf
               "Error when typechecking function: %s"
               (snd f.f_name)
      end
    | Constant gc -> Aast.Constant (gconst_def opts gc)
    | Typedef td -> Aast.Typedef (typedef_def opts td)
    | Class c ->
      begin
        match class_def opts c with
        | Some (c, _) -> Aast.Class c
        | None ->
          failwith
          @@ Printf.sprintf "Error in declaration of class: %s" (snd c.c_name)
      end
    | RecordDef rd -> Aast.RecordDef (record_def_def opts rd)
    (* We don't typecheck top level statements:
     * https://docs.hhvm.com/hack/unsupported/top-level
     * so just create the minimal env for us to construct a Stmt.
     *)
    | Stmt s ->
      let env = Env.empty opts Relative_path.default None in
      Aast.Stmt (snd (stmt env s))
    | Namespace _
    | NamespaceUse _
    | SetNamespaceEnv _
    | FileAttributes _ ->
      failwith
        "Invalid nodes in NAST. These nodes should be removed during naming."
  in
  Nast_check.program nast;
  let tast = List.map nast convert_def in
  if do_tast_checks then Tast_check.program opts tast;
  tast

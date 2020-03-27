(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs

let conditionally_reactive_attribute_to_hint env { ua_params = l; _ } =
  match l with
  (* convert class const expression to non-generic type hint *)
  | [(p, Class_const ((_, CI cls), (_, name)))]
    when String.equal name SN.Members.mClass ->
    (* set Extends dependency for between class that contains
         method and condition type *)
    Decl_env.add_extends_dependency env (snd cls);
    Decl_hint.hint env (p, Happly (cls, []))
  | _ ->
    (* error for invalid argument list was already reported during the
       naming step, do nothing *)
    mk (Reason.none, Typing_defs.make_tany ())

let condition_type_from_attributes env user_attributes =
  Naming_attributes.find SN.UserAttributes.uaOnlyRxIfImpl user_attributes
  |> Option.map ~f:(conditionally_reactive_attribute_to_hint env)

let fun_reactivity_opt env user_attributes =
  let has attr = Naming_attributes.mem attr user_attributes in
  let module UA = SN.UserAttributes in
  let rx_condition = condition_type_from_attributes env user_attributes in
  if has UA.uaReactive then
    Some (Reactive rx_condition)
  else if has UA.uaShallowReactive then
    Some (Shallow rx_condition)
  else if has UA.uaLocalReactive then
    Some (Local rx_condition)
  else if has UA.uaNonRx then
    Some Nonreactive
  else
    None

let fun_reactivity env user_attributes =
  fun_reactivity_opt env user_attributes |> Option.value ~default:Nonreactive

let has_accept_disposable_attribute user_attributes =
  Naming_attributes.mem SN.UserAttributes.uaAcceptDisposable user_attributes

let has_return_disposable_attribute user_attributes =
  Naming_attributes.mem SN.UserAttributes.uaReturnDisposable user_attributes

let fun_returns_mutable user_attributes =
  Naming_attributes.mem SN.UserAttributes.uaMutableReturn user_attributes

let fun_returns_void_to_rx user_attributes =
  Naming_attributes.mem SN.UserAttributes.uaReturnsVoidToRx user_attributes

let get_param_mutability user_attributes =
  if Naming_attributes.mem SN.UserAttributes.uaOwnedMutable user_attributes then
    Some Param_owned_mutable
  else if Naming_attributes.mem SN.UserAttributes.uaMutable user_attributes then
    Some Param_borrowed_mutable
  else if Naming_attributes.mem SN.UserAttributes.uaMaybeMutable user_attributes
  then
    Some Param_maybe_mutable
  else
    None

exception Gi_reinfer_type_not_supported

let cut_namespace id =
  let ids = String.split id ~on:'\\' in
  List.last_exn ids

let rec reinfer_type_to_string_exn ty =
  match ty with
  | Tmixed -> "mixed"
  | Tnonnull -> "nonnull"
  | Tdynamic -> "dynamic"
  | Tunion [] -> "nothing"
  | Tthis -> "this"
  | Tprim prim ->
    (match prim with
    | Tnull -> "null"
    | Tvoid -> "void"
    | Tint -> "int"
    | Tnum -> "num"
    | Tfloat -> "float"
    | Tstring -> "string"
    | Tarraykey -> "arraykey"
    | Tresource -> "resource"
    | Tnoreturn -> "noreturn"
    | Tbool -> "bool"
    | Tatom _id -> "atom")
  | Tapply ((_p, id), _tyl) -> cut_namespace id
  | Taccess (ty, ids) ->
    let s = reinfer_type_to_string_exn (get_node ty) in
    List.fold ids ~init:s ~f:(fun s (_p, id) -> Printf.sprintf "%s::%s" s id)
  | _ -> raise Gi_reinfer_type_not_supported

let reinfer_type_to_string_opt ty =
  try Some (reinfer_type_to_string_exn ty)
  with Gi_reinfer_type_not_supported -> None

let must_reinfer_type tcopt (ty : decl_phase ty_) =
  let reinfer_types = GlobalOptions.tco_gi_reinfer_types tcopt in
  match reinfer_type_to_string_opt ty with
  | None -> false
  | Some ty_str -> List.mem reinfer_types ty_str ~equal:String.equal

let hint_to_type_opt ~is_lambda env reason hint =
  let ty = Option.map hint ~f:(Decl_hint.hint env) in
  let tcopt = Provider_context.get_tcopt env.Decl_env.ctx in
  let tco_global_inference = GlobalOptions.tco_global_inference tcopt in
  let tvar = mk (reason, Tvar 0) in
  if tco_global_inference && not is_lambda then
    let ty =
      match ty with
      | None -> tvar
      | Some ty ->
        let rec create_vars_for_reinfer_types ty =
          match deref ty with
          | (r, Tapply (id, [ty']))
            when String.equal (snd id) SN.Classes.cAwaitable ->
            let ty' = create_vars_for_reinfer_types ty' in
            mk (r, Tapply (id, [ty']))
          | (r, Toption ty') ->
            let ty' = create_vars_for_reinfer_types ty' in
            mk (r, Toption ty')
          | (r, Tapply ((_p, id), []))
            when String.equal (cut_namespace id) "PHPism_FIXME_Array" ->
            if must_reinfer_type tcopt (get_node ty) then
              let tvar = mk (r, Tvar 0) in
              mk (r, Tvarray_or_darray (Some tvar, tvar))
            else
              ty
          | (_r, ty_) ->
            if must_reinfer_type tcopt ty_ then
              tvar
            else
              ty
        in
        create_vars_for_reinfer_types ty
    in
    Some ty
  else
    ty

let hint_to_type ~is_lambda ~default env reason hint =
  Option.value (hint_to_type_opt ~is_lambda env reason hint) ~default

let make_param_ty env ~is_lambda param =
  let ty =
    let r = Reason.Rwitness param.param_pos in
    hint_to_type
      ~is_lambda
      ~default:(mk (r, Typing_defs.make_tany ()))
      env
      (Reason.Rglobal_fun_param param.param_pos)
      (hint_of_type_hint param.param_type_hint)
  in
  let ty =
    match get_node ty with
    | t when param.param_is_variadic ->
      (* When checking a call f($a, $b) to a function f(C ...$args),
       * both $a and $b must be of type C *)
      mk (Reason.Rvar_param param.param_pos, t)
    | _ -> ty
  in
  let module UA = SN.UserAttributes in
  let has_at_most_rx_as_func =
    Naming_attributes.mem UA.uaAtMostRxAsFunc param.param_user_attributes
  in
  let ty =
    if has_at_most_rx_as_func then
      make_function_type_rxvar ty
    else
      ty
  in
  let mode = get_param_mode param.param_callconv in
  let rx_annotation =
    if has_at_most_rx_as_func then
      Some Param_rx_var
    else
      Naming_attributes.find UA.uaOnlyRxIfImpl param.param_user_attributes
      |> Option.map ~f:(fun v ->
             Param_rx_if_impl (conditionally_reactive_attribute_to_hint env v))
  in
  {
    fp_pos = param.param_pos;
    fp_name = Some param.param_name;
    fp_type = { et_type = ty; et_enforced = false };
    fp_kind = mode;
    fp_mutability = get_param_mutability param.param_user_attributes;
    fp_accept_disposable =
      has_accept_disposable_attribute param.param_user_attributes;
    fp_rx_annotation = rx_annotation;
  }

let ret_from_fun_kind ?(is_constructor = false) ~is_lambda env pos kind hint =
  let default = mk (Reason.Rwitness pos, Typing_defs.make_tany ()) in
  let ret_ty () =
    if is_constructor then
      mk (Reason.Rwitness pos, Tprim Tvoid)
    else
      hint_to_type ~is_lambda ~default env (Reason.Rglobal_fun_ret pos) hint
  in
  match hint with
  | None ->
    (match kind with
    | Ast_defs.FGenerator ->
      let r = Reason.Rret_fun_kind (pos, kind) in
      mk
        ( r,
          Tapply
            ((pos, SN.Classes.cGenerator), [ret_ty (); ret_ty (); ret_ty ()]) )
    | Ast_defs.FAsyncGenerator ->
      let r = Reason.Rret_fun_kind (pos, kind) in
      mk
        ( r,
          Tapply
            ( (pos, SN.Classes.cAsyncGenerator),
              [ret_ty (); ret_ty (); ret_ty ()] ) )
    | Ast_defs.FAsync ->
      let r = Reason.Rret_fun_kind (pos, kind) in
      mk (r, Tapply ((pos, SN.Classes.cAwaitable), [ret_ty ()]))
    | Ast_defs.FSync
    | Ast_defs.FCoroutine ->
      ret_ty ())
  | Some _ -> ret_ty ()

let type_param env (t : Nast.tparam) =
  {
    Typing_defs.tp_variance = t.tp_variance;
    tp_name = t.tp_name;
    tp_constraints =
      List.map t.tp_constraints (fun (ck, h) -> (ck, Decl_hint.hint env h));
    tp_reified = t.tp_reified;
    tp_user_attributes = t.tp_user_attributes;
  }

let where_constraint env (ty1, ck, ty2) =
  (Decl_hint.hint env ty1, ck, Decl_hint.hint env ty2)

(* Functions building the types for the parameters of a function *)
(* It's not completely trivial because of optional arguments  *)

let minimum_arity paraml =
  (* We're looking for the minimum number of arguments that must be specified
  in a call to this method. Variadic "..." parameters need not be specified,
  parameters with default values need not be specified, so this method counts
  non-default-value, non-variadic parameters. *)
  let f param =
    (not param.param_is_variadic) && Option.is_none param.param_expr
  in
  List.count paraml f

let check_params env paraml =
  (* We wish to give an error on the first non-default parameter
  after a default parameter. That is:
  function foo(int $x, ?int $y = null, int $z)
  is an error on $z. *)
  (* TODO: This check doesn't need to be done at type checking time; it is
  entirely syntactic. When we switch over to the FFP, remove this code. *)
  let rec loop seen_default paraml =
    match paraml with
    | [] -> ()
    | param :: rl ->
      if param.param_is_variadic then
        ()
      (* Assume that a variadic parameter is the last one we need
            to check. We've already given a parse error if the variadic
            parameter is not last. *)
      else if seen_default && Option.is_none param.param_expr then
        Errors.previous_default param.param_pos
      (* We've seen at least one required parameter, and there's an
          optional parameter after it.  Given an error, and then stop looking
          for more errors in this parameter list. *)
      else
        loop (Option.is_some param.param_expr) rl
  in
  (* PHP allows non-default valued parameters after default valued parameters. *)
  if not FileInfo.(equal_mode env.Decl_env.mode Mphp) then loop false paraml

let make_params env ~is_lambda paraml =
  List.map paraml ~f:(make_param_ty env ~is_lambda)

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
module SN = Naming_special_names

let get_classname_or_literal_attribute_param = function
  | [(_, String s)] -> Some s
  | [(_, Class_const ((_, CI (_, s)), (_, name)))]
    when String.equal name SN.Members.mClass ->
    Some s
  | _ -> None

let find_policied_attribute user_attributes : ifc_fun_decl =
  match Naming_attributes.find SN.UserAttributes.uaPolicied user_attributes with
  | Some { ua_params; _ } ->
    (match get_classname_or_literal_attribute_param ua_params with
    | Some s -> FDPolicied (Some s)
    | None -> FDPolicied None)
  | None
    when Naming_attributes.mem SN.UserAttributes.uaInferFlows user_attributes ->
    FDInferFlows
  | None -> default_ifc_fun_decl

let has_accept_disposable_attribute user_attributes =
  Naming_attributes.mem SN.UserAttributes.uaAcceptDisposable user_attributes

let has_external_attribute user_attributes =
  Naming_attributes.mem SN.UserAttributes.uaExternal user_attributes

let has_can_call_attribute user_attributes =
  Naming_attributes.mem SN.UserAttributes.uaCanCall user_attributes

let has_via_label_attribute user_attributes =
  Naming_attributes.mem SN.UserAttributes.uaViaLabel user_attributes

let has_return_disposable_attribute user_attributes =
  Naming_attributes.mem SN.UserAttributes.uaReturnDisposable user_attributes

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
    | Tbool -> "bool")
  | Tapply ((_p, id), _tyl) -> cut_namespace id
  | Taccess (ty, id) ->
    let s = reinfer_type_to_string_exn (get_node ty) in
    Printf.sprintf "%s::%s" s (snd id)
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
              mk (r, Tvarray_or_darray (tvar, tvar))
            else
              ty
          | (r, Tvarray ty) ->
            let ty = create_vars_for_reinfer_types ty in
            mk (r, Tvarray ty)
          | (r, Tdarray (tyk, tyv)) ->
            let tyk = create_vars_for_reinfer_types tyk in
            let tyv = create_vars_for_reinfer_types tyv in
            mk (r, Tdarray (tyk, tyv))
          | (r, Tvarray_or_darray (tyk, tyv)) ->
            let tyk = create_vars_for_reinfer_types tyk in
            let tyv = create_vars_for_reinfer_types tyv in
            mk (r, Tvarray_or_darray (tyk, tyv))
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
  let param_pos = Decl_env.make_decl_pos env param.param_pos in
  let ty =
    let r = Reason.Rwitness_from_decl param_pos in
    hint_to_type
      ~is_lambda
      ~default:(mk (r, Typing_defs.make_tany ()))
      env
      (Reason.Rglobal_fun_param param_pos)
      (hint_of_type_hint param.param_type_hint)
  in
  let ty =
    match get_node ty with
    | t when param.param_is_variadic ->
      (* When checking a call f($a, $b) to a function f(C ...$args),
       * both $a and $b must be of type C *)
      mk (Reason.Rvar_param_from_decl param_pos, t)
    | _ -> ty
  in
  let module UA = SN.UserAttributes in
  let mode = get_param_mode param.param_callconv in
  {
    fp_pos = param_pos;
    fp_name = Some param.param_name;
    fp_type = { et_type = ty; et_enforced = Unenforced };
    fp_flags =
      make_fp_flags
        ~mode
        ~accept_disposable:
          (has_accept_disposable_attribute param.param_user_attributes)
        ~has_default:(Option.is_some param.param_expr)
        ~ifc_external:(has_external_attribute param.param_user_attributes)
        ~ifc_can_call:(has_can_call_attribute param.param_user_attributes)
        ~via_label:(has_via_label_attribute param.param_user_attributes)
        ~readonly:(Option.is_some param.param_readonly);
  }

(** Make FunParam for the partial-mode ellipsis parameter (unnamed, and untyped) *)
let make_ellipsis_param_ty :
    Decl_env.env -> Pos.t -> 'phase ty Typing_defs_core.fun_param =
 fun env pos ->
  let pos = Decl_env.make_decl_pos env pos in
  let r = Reason.Rwitness_from_decl pos in
  let ty = mk (r, Typing_defs.make_tany ()) in
  {
    fp_pos = pos;
    fp_name = None;
    fp_type = { et_type = ty; et_enforced = Unenforced };
    fp_flags =
      make_fp_flags
        ~mode:FPnormal
        ~accept_disposable:false
        ~has_default:false
        ~ifc_external:false
        ~ifc_can_call:false
        ~via_label:false
        ~readonly:false;
  }

let ret_from_fun_kind
    ?(is_constructor = false) ~is_lambda env (pos : pos) kind hint =
  let pos = Decl_env.make_decl_pos env pos in
  let default = mk (Reason.Rwitness_from_decl pos, Typing_defs.make_tany ()) in
  let ret_ty () =
    if is_constructor then
      mk (Reason.Rwitness_from_decl pos, Tprim Tvoid)
    else
      hint_to_type ~is_lambda ~default env (Reason.Rglobal_fun_ret pos) hint
  in
  match hint with
  | None ->
    (match kind with
    | Ast_defs.FGenerator ->
      let r = Reason.Rret_fun_kind_from_decl (pos, kind) in
      mk
        ( r,
          Tapply
            ((pos, SN.Classes.cGenerator), [ret_ty (); ret_ty (); ret_ty ()]) )
    | Ast_defs.FAsyncGenerator ->
      let r = Reason.Rret_fun_kind_from_decl (pos, kind) in
      mk
        ( r,
          Tapply
            ( (pos, SN.Classes.cAsyncGenerator),
              [ret_ty (); ret_ty (); ret_ty ()] ) )
    | Ast_defs.FAsync ->
      let r = Reason.Rret_fun_kind_from_decl (pos, kind) in
      mk (r, Tapply ((pos, SN.Classes.cAwaitable), [ret_ty ()]))
    | Ast_defs.FSync -> ret_ty ())
  | Some _ -> ret_ty ()

let type_param = Decl_hint.aast_tparam_to_decl_tparam

let where_constraint env (ty1, ck, ty2) =
  (Decl_hint.hint env ty1, ck, Decl_hint.hint env ty2)

(* Functions building the types for the parameters of a function *)
(* It's not completely trivial because of optional arguments  *)

let check_params paraml =
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
  loop false paraml

let make_params env ~is_lambda paraml =
  List.map paraml ~f:(make_param_ty env ~is_lambda)

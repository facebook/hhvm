(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
open Typing_env_return_info
module Env = Typing_env
module TUtils = Typing_utils
module MakeType = Typing_make_type

(* The regular strip_awaitable function depends on expand_type and only works on locl types *)
let strip_awaitable_decl fun_kind env (ty : decl_ty) =
  if not Ast_defs.(equal_fun_kind fun_kind FAsync) then
    ty
  else
    match (Env.get_fn_kind env, get_node ty) with
    | (Ast_defs.FAsync, Tapply ((_, class_name), [inner_ty]))
      when String.equal class_name Naming_special_names.Classes.cAwaitable ->
      inner_ty
    | _ -> ty

let strip_awaitable fun_kind env et =
  if not Ast_defs.(equal_fun_kind fun_kind FAsync) then
    et
  else
    let (_env, ty) = Env.expand_type env et.et_type in
    match get_node ty with
    | Tclass ((_, class_name), _, [ty])
      when String.equal class_name Naming_special_names.Classes.cAwaitable ->
      { et with et_type = ty }
    (* In non-strict code we might find Awaitable without type arguments. Assume Tany *)
    | Tclass ((_, class_name), _, [])
      when String.equal class_name Naming_special_names.Classes.cAwaitable ->
      { et with et_type = mk (Reason.Rnone, TUtils.tany env) }
    | _ -> et

let enforce_return_not_disposable fun_kind env et =
  let stripped_et = strip_awaitable fun_kind env et in
  match Typing_disposable.is_disposable_type env stripped_et.et_type with
  | Some class_name ->
    Errors.invalid_disposable_return_hint
      (get_pos et.et_type)
      (Utils.strip_ns class_name)
  | None -> ()

let has_attribute attr l =
  List.exists l (fun { Aast.ua_name; _ } -> String.equal attr (snd ua_name))

let has_return_disposable_attribute attrs =
  has_attribute SN.UserAttributes.uaReturnDisposable attrs

let has_mutable_return_attribute attrs =
  has_attribute SN.UserAttributes.uaMutableReturn attrs

let has_return_void_to_rx_attribute attrs =
  has_attribute SN.UserAttributes.uaReturnsVoidToRx attrs

let make_info fun_kind attributes env ~is_explicit locl_ty decl_ty =
  let return_disposable = has_return_disposable_attribute attributes in
  let return_mutable = has_mutable_return_attribute attributes in
  let return_void_to_rx = has_return_void_to_rx_attribute attributes in
  let et_enforced =
    match decl_ty with
    | None -> false
    | Some decl_ty ->
      let stripped_decl_ty = strip_awaitable_decl fun_kind env decl_ty in
      Typing_enforceability.is_enforceable env stripped_decl_ty
  in
  let return_type = { et_type = locl_ty; et_enforced } in
  if not return_disposable then
    enforce_return_not_disposable fun_kind env return_type;
  {
    return_type;
    return_disposable;
    return_mutable;
    return_explicit = is_explicit;
    return_void_to_rx;
  }

(* For async functions, wrap Awaitable<_> around the return type *)
let wrap_awaitable env p rty =
  match Env.get_fn_kind env with
  | Ast_defs.FCoroutine
  | Ast_defs.FSync ->
    rty
  | Ast_defs.FGenerator
  (* Is an error, but caught in Nast_check. *)
  | Ast_defs.FAsyncGenerator ->
    TUtils.terr env Reason.Rnone
  | Ast_defs.FAsync ->
    MakeType.awaitable (Reason.Rret_fun_kind (p, Ast_defs.FAsync)) rty

let make_return_type localize env (ty : decl_ty) =
  match (Env.get_fn_kind env, deref ty) with
  | (Ast_defs.FAsync, (_, Tapply ((_, class_name), [inner_ty])))
    when String.equal class_name Naming_special_names.Classes.cAwaitable ->
    let (env, ty) = localize env inner_ty in
    (env, wrap_awaitable env (get_pos ty) ty)
  | (Ast_defs.FAsync, (r_like, Tlike ty_like)) ->
    begin
      match get_node ty_like with
      | Tapply ((_, class_name), [inner_ty])
        when String.equal class_name Naming_special_names.Classes.cAwaitable ->
        let ty = mk (r_like, Tlike inner_ty) in
        let (env, ty) = localize env ty in
        (env, wrap_awaitable env (get_pos ty_like) ty)
      | _ -> localize env ty
    end
  | _ -> localize env ty

let force_awaitable env p ty =
  let fun_kind = Env.get_fn_kind env in
  let (env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tclass ((_, class_name), _, _)
    when Ast_defs.(equal_fun_kind fun_kind FAsync)
         && String.equal class_name Naming_special_names.Classes.cAwaitable ->
    (env, ty)
  | Tany _ when Ast_defs.(equal_fun_kind fun_kind FAsync) ->
    (env, wrap_awaitable env p ty)
  | _ when Ast_defs.(equal_fun_kind fun_kind FAsync) ->
    let (env, underlying_ty) = Env.fresh_type env p in
    let wrapped_ty = wrap_awaitable env p underlying_ty in
    let env =
      Typing_ops.sub_type p Reason.URnone env wrapped_ty ty Errors.unify_error
    in
    (env, wrapped_ty)
  | _ -> (env, ty)

let make_default_return ~is_method env name =
  let pos = fst name in
  let r = Reason.Rwitness pos in
  if is_method && String.equal (snd name) SN.Members.__construct then
    MakeType.void r
  else
    mk (r, Typing_utils.tany env)

let async_suggest_return fkind hint pos =
  let is_async = Ast_defs.(equal_fun_kind FAsync fkind) in
  if is_async then
    let e_func = Errors.expecting_awaitable_return_type_hint in
    match snd hint with
    | Aast.Hlike (_, Aast.Happly (s, _))
    | Aast.Happly (s, _) ->
      if String.( <> ) (snd s) Naming_special_names.Classes.cAwaitable then
        e_func pos
    | _ -> e_func pos

let implicit_return env pos ~expected ~actual =
  let env =
    Typing_ops.sub_type
      pos
      Reason.URreturn
      env
      expected
      actual
      Errors.missing_return
  in
  Typing_coercion.coerce_type
    pos
    Reason.URreturn
    env
    actual
    { et_type = expected; et_enforced = false }
    Errors.missing_return

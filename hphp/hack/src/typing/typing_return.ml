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
module SN = Naming_special_names

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

let enforce_return_not_disposable ret_pos fun_kind env et =
  let stripped_et = strip_awaitable fun_kind env et in
  match Typing_disposable.is_disposable_type env stripped_et.et_type with
  | Some class_name ->
    Errors.invalid_disposable_return_hint ret_pos (Utils.strip_ns class_name)
  | None -> ()

let has_attribute attr l =
  List.exists l ~f:(fun { Aast.ua_name; _ } -> String.equal attr (snd ua_name))

let has_return_disposable_attribute attrs =
  has_attribute SN.UserAttributes.uaReturnDisposable attrs

let make_info ret_pos fun_kind attributes env ~is_explicit locl_ty decl_ty =
  let return_disposable = has_return_disposable_attribute attributes in
  let et_enforced =
    match decl_ty with
    | None -> Unenforced
    | Some decl_ty ->
      let stripped_decl_ty = strip_awaitable_decl fun_kind env decl_ty in
      Typing_enforceability.get_enforcement env stripped_decl_ty
  in
  let return_type = { et_type = locl_ty; et_enforced } in
  if not return_disposable then
    enforce_return_not_disposable ret_pos fun_kind env return_type;
  {
    return_type;
    return_disposable;
    return_explicit = is_explicit;
    return_dynamically_callable = false;
  }

let make_return_type localize env (ty : decl_ty) =
  let wrap_awaitable p =
    MakeType.awaitable (Reason.Rret_fun_kind_from_decl (p, Ast_defs.FAsync))
  in
  match (Env.get_fn_kind env, deref ty) with
  | (Ast_defs.FAsync, (_, Tapply ((_, class_name), [inner_ty])))
    when String.equal class_name Naming_special_names.Classes.cAwaitable ->
    let (env, ty) = localize env inner_ty in
    (env, wrap_awaitable (get_pos ty) ty)
  | (Ast_defs.FAsync, (r_like, Tlike ty_like)) ->
    begin
      match get_node ty_like with
      | Tapply ((_, class_name), [inner_ty])
        when String.equal class_name Naming_special_names.Classes.cAwaitable ->
        let ty = mk (r_like, Tlike inner_ty) in
        let (env, ty) = localize env ty in
        (env, wrap_awaitable (get_pos ty_like) ty)
      | _ -> localize env ty
    end
  | _ -> localize env ty

(* Create a return type with fresh type variables  *)
let make_fresh_return_type env p =
  let (env, rty) = Env.fresh_type env p in
  let fun_kind = Env.get_fn_kind env in
  let r = Reason.Rret_fun_kind_from_decl (Pos_or_decl.of_raw_pos p, fun_kind) in
  match fun_kind with
  | Ast_defs.FSync -> (env, rty)
  | Ast_defs.FAsync -> (env, MakeType.awaitable r rty)
  | Ast_defs.FGenerator ->
    let (env, key) = Env.fresh_type env p in
    let (env, send) = Env.fresh_type env p in
    (env, MakeType.generator r key rty send)
  | Ast_defs.FAsyncGenerator ->
    let (env, key) = Env.fresh_type env p in
    let (env, send) = Env.fresh_type env p in
    (env, MakeType.async_generator r key rty send)

let force_return_kind ?(is_toplevel = true) env p ty =
  let fun_kind = Env.get_fn_kind env in
  let (env, ty) = Env.expand_type env ty in
  match (fun_kind, get_node ty) with
  (* Sync functions can return anything *)
  | (Ast_defs.FSync, _) -> (env, ty)
  (* Each other fun kind needs a specific return type *)
  | (Ast_defs.FAsync, _) when is_toplevel ->
    (* For toplevel functions, this is already checked in the parser *)
    (env, ty)
  | (Ast_defs.FAsync, Tclass ((_, class_name), _, _))
    when String.equal class_name Naming_special_names.Classes.cAwaitable ->
    (* For toplevel functions, this is already checked in the parser *)
    (env, ty)
  | (Ast_defs.FGenerator, Tclass ((_, class_name), _, _))
    when String.equal class_name Naming_special_names.Classes.cGenerator ->
    (env, ty)
  | (Ast_defs.FAsyncGenerator, Tclass ((_, class_name), _, _))
    when String.equal class_name Naming_special_names.Classes.cAsyncGenerator ->
    (env, ty)
  | _ ->
    let (env, wrapped_ty) = make_fresh_return_type env p in
    let env =
      Typing_ops.sub_type p Reason.URreturn env wrapped_ty ty Errors.unify_error
    in
    (env, wrapped_ty)

let make_default_return ~is_method env name =
  let pos = fst name in
  let r = Reason.Rwitness pos in
  if is_method && String.equal (snd name) SN.Members.__construct then
    MakeType.void r
  else
    mk (r, Typing_utils.tany env)

let implicit_return env pos ~expected ~actual =
  let reason = Reason.URreturn in
  let error = Errors.missing_return in
  let open Typing_env_types in
  if TypecheckerOptions.enable_sound_dynamic env.genv.tcopt then
    Typing_coercion.coerce_type
      pos
      reason
      env
      actual
      { et_type = expected; et_enforced = Unenforced }
      error
  else
    Typing_ops.sub_type pos reason env actual expected error

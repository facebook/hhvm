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
open Typing_env_types
open Typing_env_return_info
module Env = Typing_env
module TUtils = Typing_utils
module MakeType = Typing_make_type
module SN = Naming_special_names

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
  Option.iter ~f:Errors.add_typing_error
  @@ Option.map
       (Typing_disposable.is_disposable_type env stripped_et.et_type)
       ~f:(fun class_name ->
         let open Typing_error in
         primary
         @@ Primary.Invalid_disposable_return_hint
              { pos = ret_pos; class_name = Utils.strip_ns class_name })

let has_attribute attr l =
  List.exists l ~f:(fun { Aast.ua_name; _ } -> String.equal attr (snd ua_name))

let has_return_disposable_attribute attrs =
  has_attribute SN.UserAttributes.uaReturnDisposable attrs

let make_info ret_pos fun_kind attributes env return_type =
  let return_disposable = has_return_disposable_attribute attributes in
  if not return_disposable then
    enforce_return_not_disposable ret_pos fun_kind env return_type;
  { return_type; return_disposable }

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

(** Force the return type of a function to adhere to the fun_kind specified in
    the env *)
let force_return_kind ~is_toplevel env p ety =
  let fun_kind = Env.get_fn_kind env in
  let (env, ty) = Env.expand_type env ety.et_type in
  let (env, ty) =
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
      when String.equal class_name Naming_special_names.Classes.cAsyncGenerator
      ->
      (env, ty)
    | _ ->
      let (env, wrapped_ty) = make_fresh_return_type env p in
      let (env, ty_err_opt) =
        Typing_ops.sub_type
          p
          Reason.URreturn
          env
          wrapped_ty
          ty
          Typing_error.Callback.unify_error
      in
      Option.iter ~f:Errors.add_typing_error ty_err_opt;
      (env, wrapped_ty)
  in
  (env, { ety with et_type = ty })

let make_return_type
    ~ety_env
    env
    ~hint_pos
    ~(explicit : decl_ty option)
    ~(default : locl_ty option) =
  match explicit with
  | None ->
    let (env, ty) =
      match default with
      | None -> make_fresh_return_type env hint_pos
      | Some ty -> (env, ty)
    in
    (env, MakeType.unenforced ty)
  | Some ty ->
    let wrap_awaitable p ty =
      MakeType.awaitable
        (Reason.Rret_fun_kind_from_decl (p, Ast_defs.FAsync))
        ty
    in
    let localize ~wrap env dty =
      let et_enforced = Typing_enforceability.get_enforcement env dty in
      (match et_enforced with
      | Unenforced -> Typing_log.log_pessimise_return env hint_pos
      | Enforced -> ());
      let ((env, ty_err_opt), et_type) =
        Typing_phase.localize ~ety_env env dty
      in
      Option.iter ~f:Errors.add_typing_error ty_err_opt;
      (* If return type t is enforced we permit values of type ~t to be returned *)
      let et_type =
        match et_enforced with
        | Enforced
          when TypecheckerOptions.enable_sound_dynamic env.genv.tcopt
               && Env.get_support_dynamic_type env ->
          Typing_utils.make_like env et_type
        | _ -> et_type
      in
      let et_type =
        if wrap then
          wrap_awaitable (get_pos et_type) et_type
        else
          et_type
      in
      (env, { et_enforced; et_type })
    in
    (match (Env.get_fn_kind env, deref ty) with
    | (Ast_defs.FAsync, (_, Tapply ((_, class_name), [inner_ty])))
      when String.equal class_name Naming_special_names.Classes.cAwaitable ->
      localize ~wrap:true env inner_ty
    | (Ast_defs.FAsync, (r_like, Tlike ty_like)) ->
      begin
        match get_node ty_like with
        | Tapply ((_, class_name), [inner_ty])
          when String.equal class_name Naming_special_names.Classes.cAwaitable
          ->
          let ty = mk (r_like, Tlike inner_ty) in
          localize ~wrap:true env ty
        | _ -> localize ~wrap:false env ty
      end
    | _ -> localize ~wrap:false env ty)

let make_return_type
    ~ety_env ?(is_toplevel = true) env ~hint_pos ~explicit ~default =
  let (env, ty) = make_return_type ~ety_env env ~hint_pos ~explicit ~default in
  force_return_kind ~is_toplevel env hint_pos ty

let implicit_return env pos ~expected ~actual ~hint_pos ~is_async =
  let reason = Reason.URreturn in
  let error =
    Typing_error.Primary.(
      Wellformedness (Wellformedness.Missing_return { pos; hint_pos; is_async }))
  in
  let (env, ty_err_opt) =
    if TypecheckerOptions.enable_sound_dynamic env.genv.tcopt then
      Typing_utils.sub_type
        env
        ~coerce:(Some Typing_logic.CoerceToDynamic)
        actual
        expected
      @@ Some (Typing_error.Reasons_callback.of_primary_error error)
    else
      Typing_ops.sub_type pos reason env actual expected
      @@ Typing_error.Callback.of_primary_error error
  in

  Option.iter ~f:Errors.add_typing_error ty_err_opt;
  env

let check_inout_return ret_pos env =
  let params = Local_id.Map.elements (Env.get_params env) in
  let (env, ty_errs) =
    List.fold
      params
      ~init:(env, [])
      ~f:(fun (env, ty_errs) (id, (ty, param_pos, mode)) ->
        match mode with
        | FPinout ->
          (* Whenever the function exits normally, we require that each local
           * corresponding to an inout parameter be compatible with the original
           * type for the parameter (under subtyping rules). *)
          let (local_ty, local_pos) = Env.get_local_pos env id in
          let (env, ety) = Env.expand_type env local_ty in
          let pos =
            if not (Pos.equal Pos.none local_pos) then
              local_pos
            else if not (Pos.equal Pos.none ret_pos) then
              ret_pos
            else
              param_pos
          in
          let param_ty = mk (Reason.Rinout_param (get_pos ty), get_node ty) in
          let (env, ty_err_opt) =
            Typing_ops.sub_type
              pos
              Reason.URassign_inout
              env
              ety
              param_ty
              Typing_error.Callback.inout_return_type_mismatch
          in
          let ty_errs =
            Option.value_map
              ~default:ty_errs
              ~f:(fun e -> e :: ty_errs)
              ty_err_opt
          in
          (env, ty_errs)
        | _ -> (env, ty_errs))
  in
  Option.iter ~f:Errors.add_typing_error @@ Typing_error.multiple_opt ty_errs;
  env

let rec remove_like_for_return env ty =
  match TUtils.try_strip_dynamic env ty with
  | Some ty -> ty
  | None ->
    (match get_node ty with
    | Tclass ((p, class_name), exact, [ty])
      when String.equal class_name Naming_special_names.Classes.cAwaitable ->
      mk
        ( get_reason ty,
          Tclass ((p, class_name), exact, [remove_like_for_return env ty]) )
    | _ -> ty)

let fun_implicit_return env pos ret =
  let ret =
    if TypecheckerOptions.enable_sound_dynamic env.genv.tcopt then
      (* Under sound dynamic, void <D: dynamic, which means that it void <D: ~T for any T.
       * However, for ergonomic reasons, it should not be the case that a function
       * erroring about a missing `return` statement would stop if the return type were
       * pessimised. This preserves the behavior that a function explicitly returning dynamic
       * can have an implicit return, while requiring an explicit return for like types. *)
      remove_like_for_return env ret
    else
      ret
  in
  let ret_pos = Some (Typing_defs_core.get_pos ret) in

  function
  | Ast_defs.FGenerator
  | Ast_defs.FAsyncGenerator ->
    env
  | Ast_defs.FSync ->
    (* A function without a terminal block has an implicit return; the
     * "void" type *)
    let env = check_inout_return Pos.none env in
    let r = Reason.Rno_return pos in
    let rty = MakeType.void r in
    implicit_return
      env
      pos
      ~expected:ret
      ~actual:rty
      ~hint_pos:ret_pos
      ~is_async:false
  | Ast_defs.FAsync ->
    (* An async function without a terminal block has an implicit return;
     * the Awaitable<void> type *)
    let r = Reason.Rno_return_async pos in
    let rty = MakeType.awaitable r (MakeType.void r) in
    implicit_return
      env
      pos
      ~expected:ret
      ~actual:rty
      ~hint_pos:ret_pos
      ~is_async:true

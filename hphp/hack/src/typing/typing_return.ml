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
module Cls = Decl_provider.Class

let strip_awaitable fun_kind env et =
  if not Ast_defs.(equal_fun_kind fun_kind FAsync) then
    et
  else
    let (_env, ty) = Env.expand_type env et.et_type in
    match get_node ty with
    | Tclass ((_, class_name), _, [ty])
      when String.equal class_name SN.Classes.cAwaitable ->
      { et with et_type = ty }
    | _ -> et

let enforce_return_not_disposable ret_pos fun_kind env et =
  let stripped_et = strip_awaitable fun_kind env et in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env)
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

(* Make a fresh Awaitable<_> etc making sure that any constraints (e.g. supportdyn)
 * are added to the type variable
 *)
let make_wrapped_fresh_type env p r id =
  let class_ = Env.get_class env id in
  match class_ with
  | None -> Env.fresh_type_error env p (* Shouldn't happen *)
  | Some class_ ->
    let ((env, _), ty, _tal) =
      Typing_phase.localize_targs_and_check_constraints
        ~exact:nonexact
        ~check_well_kinded:false
        ~def_pos:(Cls.pos class_)
        ~use_pos:p
        ~check_explicit_targs:false
        env
        (p, id)
        r
        (Cls.tparams class_)
        []
    in
    (env, ty)

(* Create a return type with fresh type variables  *)
let make_fresh_return_type env p =
  let fun_kind = Env.get_fn_kind env in
  let r = Reason.Rret_fun_kind_from_decl (Pos_or_decl.of_raw_pos p, fun_kind) in
  match fun_kind with
  | Ast_defs.FSync -> Env.fresh_type env p
  | Ast_defs.FAsync -> make_wrapped_fresh_type env p r SN.Classes.cAwaitable
  | Ast_defs.FGenerator -> make_wrapped_fresh_type env p r SN.Classes.cGenerator
  | Ast_defs.FAsyncGenerator ->
    make_wrapped_fresh_type env p r SN.Classes.cAsyncGenerator

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
      when String.equal class_name SN.Classes.cAwaitable ->
      (* For toplevel functions, this is already checked in the parser *)
      (env, ty)
    | (Ast_defs.FGenerator, Tclass ((_, class_name), _, _))
      when String.equal class_name SN.Classes.cGenerator ->
      (env, ty)
    | (Ast_defs.FAsyncGenerator, Tclass ((_, class_name), _, _))
      when String.equal class_name SN.Classes.cAsyncGenerator ->
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
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      (env, wrapped_ty)
  in
  (env, { ety with et_type = ty })

let make_return_type
    ~ety_env
    ~this_class
    ~supportdyn
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
    let (env, ty) =
      if supportdyn then
        TUtils.make_supportdyn
          (Reason.Rsupport_dynamic_type (Pos_or_decl.of_raw_pos hint_pos))
          env
          ty
      else
        (env, ty)
    in
    (env, MakeType.unenforced ty)
  | Some ty ->
    let wrap_awaitable p ty =
      MakeType.awaitable
        (Reason.Rret_fun_kind_from_decl (p, Ast_defs.FAsync))
        ty
    in
    let localize ~wrap (env : env) (dty : decl_ty) =
      if TypecheckerOptions.everything_sdt env.genv.tcopt then (
        let pos = get_pos dty in
        let dty =
          match get_node dty with
          | Tfun _ -> TUtils.make_supportdyn_decl_type pos (get_reason dty) dty
          | _ -> dty
        in
        let ((env, ty_err_opt), ty) = Typing_phase.localize ~ety_env env dty in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
        (* If type doesn't already support dynamic then wrap it if supportdyn=true *)
        let (env, ty) =
          if supportdyn then
            TUtils.make_supportdyn (Reason.Rsupport_dynamic_type pos) env ty
          else
            (env, ty)
        in
        (* If <<__NoAutoLikes>> is specified, then add a like- to enforced returns
         * so that we can return an expression of like-type.
         * If <<__NoAutoLikes>> is not specified, then add a like- for the same reason,
         * but add a like type even for non-enforced returns because these we pessimise anyway.
         * Never pessimise void.
         *)
        let add_like =
          if Env.get_no_auto_likes env then
            match Typing_enforceability.get_enforcement ~this_class env dty with
            | Unenforced -> false
            | Enforced -> true
          else
            true
        in
        let et_type =
          match get_node ty with
          | Tprim Aast.Tvoid when not wrap -> ty
          | _ ->
            if add_like then
              TUtils.make_like ~reason:(Reason.Rpessimised_return pos) env ty
            else
              ty
        in
        let et_type =
          if wrap then
            wrap_awaitable (get_pos et_type) et_type
          else
            et_type
        in
        (env, { et_type; et_enforced = Unenforced })
      ) else
        let et_enforced =
          Typing_enforceability.get_enforcement ~this_class env dty
        in
        let et_enforced =
          match et_enforced with
          | Unenforced ->
            if not (Env.get_no_auto_likes env) then
              Typing_log.log_pessimise_return env hint_pos None;
            (* Return type is not fully enforced according to the naive check
             * in Typing_enforceability. Let's now ask for what type it is actually
             * enforced at. If this is a subtype of the Hack type, then we can
             * treat the return type as fully enforced.
             * Example: return type is declared to be (~E & arraykey)
             * for enum E : int as int
             * get_enforced_type (~E & arraykey) is int
             * Now since int <:D (~E & arraykey) it suffices to treat
             * the return type as fully enforced.
             *)
            if not (TypecheckerOptions.enable_sound_dynamic env.genv.tcopt) then
              Unenforced
            else
              let enforced_type =
                Typing_partial_enforcement.get_enforced_type
                  env
                  (Env.get_self_class env)
                  dty
              in
              let is_sub_type =
                Typing_phase.is_sub_type_decl
                  ~coerce:(Some Typing_logic.CoerceToDynamic)
                  env
                  enforced_type
                  dty
              in
              if is_sub_type then
                Enforced
              else
                Unenforced
          | Enforced -> Enforced
        in
        let ((env, ty_err_opt), et_type) =
          Typing_phase.localize ~ety_env env dty
        in
        Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
        (* If type doesn't already support dynamic then wrap it if supportdyn=true *)
        let (env, et_type) =
          if supportdyn then
            TUtils.make_supportdyn
              (Reason.Rsupport_dynamic_type (get_pos et_type))
              env
              et_type
          else
            (env, et_type)
        in
        (* If return type t is enforced we permit values of type ~t to be returned *)
        let ety = TUtils.make_like_if_enforced env { et_enforced; et_type } in
        let et_type =
          if wrap then
            wrap_awaitable (get_pos et_type) ety.et_type
          else
            ety.et_type
        in
        (env, { ety with et_type })
    in
    (match (Env.get_fn_kind env, deref ty) with
    | (Ast_defs.FAsync, (_, Tapply ((_, class_name), [inner_ty])))
      when String.equal class_name SN.Classes.cAwaitable ->
      localize ~wrap:true env inner_ty
    | (Ast_defs.FAsync, (r_like, Tlike ty_like)) -> begin
      match get_node ty_like with
      | Tapply ((_, class_name), [inner_ty])
        when String.equal class_name SN.Classes.cAwaitable ->
        let ty = mk (r_like, Tlike inner_ty) in
        localize ~wrap:true env ty
      | _ -> localize ~wrap:false env ty
    end
    | _ -> localize ~wrap:false env ty)

let make_return_type
    ~ety_env
    ~this_class
    ?(is_toplevel = true)
    ~supportdyn
    env
    ~hint_pos
    ~explicit
    ~default =
  let (env, ty) =
    make_return_type
      ~ety_env
      ~this_class
      env
      ~supportdyn
      ~hint_pos
      ~explicit
      ~default
  in
  force_return_kind ~is_toplevel env hint_pos ty

let implicit_return env pos ~expected ~actual ~hint_pos ~is_async =
  let reason = Reason.URreturn in
  let error =
    Typing_error.Primary.(
      Wellformedness (Wellformedness.Missing_return { pos; hint_pos; is_async }))
  in
  let (env, ty_err_opt) =
    Typing_ops.sub_type pos reason env actual expected
    @@ Typing_error.Callback.of_primary_error error
  in

  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  env

let check_inout_return ret_pos env =
  let params = Local_id.Map.elements (Env.get_params env) in
  let (env, ty_errs) =
    List.fold
      params
      ~init:(env, [])
      ~f:(fun (env, ty_errs) (id, (_ty, param_pos, out_ty_opt)) ->
        match out_ty_opt with
        | Some out_ty ->
          (* Whenever the function exits normally, we require that each local
           * corresponding to an inout parameter be compatible with the original
           * type for the parameter (under subtyping rules). *)
          let local = Env.get_local env id in
          let open Typing_local_types in
          let (env, ety) = Env.expand_type env local.ty in
          let pos =
            if not (Pos.equal Pos.none local.pos) then
              local.pos
            else if not (Pos.equal Pos.none ret_pos) then
              ret_pos
            else
              param_pos
          in
          let param_ty =
            mk (Reason.Rinout_param (get_pos out_ty), get_node out_ty)
          in
          let (env, ty_err_opt) =
            Typing_coercion.coerce_type
              pos
              Reason.URassign_inout
              env
              ety
              (MakeType.unenforced param_ty)
              Typing_error.Callback.inout_return_type_mismatch
          in
          let ty_errs =
            Option.value_map
              ~default:ty_errs
              ~f:(fun e -> e :: ty_errs)
              ty_err_opt
          in
          (env, ty_errs)
        | None -> (env, ty_errs))
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env)
  @@ Typing_error.multiple_opt ty_errs;
  env

let rec remove_like_for_return env ty =
  match TUtils.try_strip_dynamic env ty with
  | Some ty -> ty
  | None ->
    (match get_node ty with
    | Tclass ((p, class_name), exact, [ty])
      when String.equal class_name SN.Classes.cAwaitable ->
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

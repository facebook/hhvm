(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
open Typing_env_types
module Cls = Decl_provider.Class
module MakeType = Typing_make_type
module Env = Typing_env

let wrap_like ty =
  let r = Typing_reason.Renforceable (Typing_reason.to_pos (fst ty)) in
  MakeType.like r ty

let rec is_enforceable (env : env) (ty : decl_ty) =
  match snd ty with
  | Tthis -> false
  | Tapply ((_, name), _) when Env.is_enum env name -> false
  | Tapply ((_, name), _) when Env.is_typedef name ->
    begin
      match Env.get_typedef env name with
      | Some { td_vis = Aast.Transparent; td_tparams; td_type; _ } ->
        (* So that the check does not collide with reified generics *)
        let env = Env.add_generic_parameters env td_tparams in
        is_enforceable env td_type
      | _ -> false
    end
  | Tapply ((_, name), tyl) ->
    begin
      match Env.get_class env name with
      | Some tc ->
        let tparams = Cls.tparams tc in
        begin
          match tyl with
          | [] -> true
          | targs ->
            List.Or_unequal_lengths.(
              begin
                match
                  List.fold2
                    ~init:true
                    targs
                    tparams
                    ~f:(fun acc targ tparam ->
                      match targ with
                      | (_, Tdynamic)
                      (* We accept the inner type being dynamic regardless of reification *)
                      
                      | (_, Tlike _) ->
                        acc
                      | _ ->
                        (match tparam.tp_reified with
                        | Aast.Erased -> false
                        | Aast.SoftReified -> false
                        | Aast.Reified -> is_enforceable env targ && acc))
                with
                | Ok new_acc -> new_acc
                | Unequal_lengths -> true
              end)
        end
      | None -> true
    end
  | Tgeneric name ->
    begin
      match (Env.get_reified env name, Env.get_enforceable env name) with
      | (Aast.Erased, _) -> false
      | (Aast.SoftReified, _) -> false
      | (Aast.Reified, false) -> false
      | (Aast.Reified, true) -> true
    end
  | Taccess _ -> false
  | Tlike _ -> false
  | Tarray (None, None) -> true
  | Tarray _ -> false
  | Tprim prim ->
    begin
      match prim with
      | Aast.Tvoid
      | Aast.Tnoreturn ->
        false
      | _ -> true
    end
  | Tany _ -> true
  | Terr -> true
  | Tnonnull -> true
  | Tdynamic -> true
  | Tfun _ -> false
  | Ttuple _ -> false
  | Tunion _ -> false
  | Tintersection _ -> false
  | Tshape _ -> false
  | Tmixed -> true
  | Tnothing -> true
  | Tvar _ -> false
  | Tdarray _ -> false
  | Tvarray _ -> false
  (* With no parameters, we enforce varray_or_darray just like array *)
  | Tvarray_or_darray (_, Tany _) -> true
  | Tvarray_or_darray _ -> false
  | Toption ty -> is_enforceable env ty
  (* TODO(T36532263) make sure that this is what we want *)
  | Tpu_access _ -> false

let make_locl_like_type env ty =
  if env.Typing_env_types.pessimize then
    let dyn =
      MakeType.dynamic (Reason.Renforceable (Reason.to_pos (fst ty)))
    in
    Typing_union.union env dyn ty
  else
    (env, ty)

let is_enforced env ~explicitly_untrusted ty =
  let enforceable = is_enforceable env ty in
  let is_hhi =
    fst ty
    |> Reason.to_pos
    |> Pos.filename
    |> Relative_path.prefix
    |> Relative_path.(equal_prefix Hhi)
  in
  enforceable && (not is_hhi) && not explicitly_untrusted

let pessimize_type env { et_type; et_enforced } =
  let et_type =
    if et_enforced || not env.pessimize then
      et_type
    else
      match et_type with
      | (_, Tprim (Aast.Tvoid | Aast.Tnoreturn)) -> et_type
      | _ -> wrap_like et_type
  in
  { et_type; et_enforced }

let compute_enforced_ty env ?(explicitly_untrusted = false) (ty : decl_ty) =
  let et_enforced = is_enforced env ~explicitly_untrusted ty in
  { et_type = ty; et_enforced }

let compute_enforced_and_pessimize_ty
    env ?(explicitly_untrusted = false) (ty : decl_ty) =
  let ety = compute_enforced_ty env ~explicitly_untrusted ty in
  pessimize_type env ety

let handle_awaitable_return
    env ft_fun_kind (ft_ret : decl_possibly_enforced_ty) =
  let { et_type = return_type; _ } = ft_ret in
  match (ft_fun_kind, return_type) with
  | (Ast_defs.FAsync, (_, Tapply ((_, name), [inner_ty])))
    when String.equal name Naming_special_names.Classes.cAwaitable ->
    let { et_enforced; _ } = compute_enforced_ty env inner_ty in
    pessimize_type env { et_type = return_type; et_enforced }
  | _ -> compute_enforced_and_pessimize_ty env return_type

let compute_enforced_and_pessimize_fun_type env (ft : decl_fun_type) =
  let { ft_params; ft_ret; ft_fun_kind; _ } = ft in
  let ft_ret = handle_awaitable_return env ft_fun_kind ft_ret in
  let ft_params =
    List.map
      ~f:(fun fp ->
        let { fp_type = { et_type; _ }; fp_kind; _ } = fp in
        let f =
          if equal_param_mode fp_kind FPinout then
            compute_enforced_and_pessimize_ty
          else
            compute_enforced_ty
        in
        let fp_type = f env et_type in
        { fp with fp_type })
      ft_params
  in
  { ft with ft_params; ft_ret }

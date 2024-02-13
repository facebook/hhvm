(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
open Typing_env_types
module Cls = Decl_provider.Class
module Env = Typing_env

module FoldedContextAccess :
  Decl_enforceability.ContextAccess with type t = env and type class_t = Cls.t =
struct
  type t = env

  type class_t = Cls.t

  let get_tcopt env = env.genv.tcopt

  let get_class_or_typedef ctx name =
    (* Preserve the decl access behaviour of the enforceability code that used to be here. *)
    ignore (Env.get_class ctx name);
    Env.get_class_or_typedef ctx name |> Decl_entry.to_option

  let get_typedef ctx x = Env.get_typedef ctx x

  let get_class ctx x = Env.get_class ctx x |> Decl_entry.to_option

  let get_typeconst_type _ cls name =
    match Cls.get_typeconst cls name with
    | None -> None
    | Some tc ->
      (match tc.ttc_kind with
      | TCAbstract abstract -> abstract.atc_as_constraint
      | TCConcrete concrete -> Some concrete.tc_type)

  let get_tparams = Cls.tparams

  let get_name = Cls.name

  let get_enum_type = Cls.enum_type
end

module E = Decl_enforceability.Enforce (FoldedContextAccess)

let get_enforcement ~this_class (env : env) (ty : decl_ty) :
    Typing_defs.enforcement =
  match E.get_enforcement ~return_from_async:false ~this_class env ty with
  | Decl_enforceability.Unenforced _ -> Unenforced
  | Decl_enforceability.Enforced _ -> Enforced

let is_enforceable ~this_class (env : env) (ty : decl_ty) =
  match get_enforcement ~this_class env ty with
  | Enforced -> true
  | Unenforced -> false

let get_enforced ~this_class env ~explicitly_untrusted ty =
  if explicitly_untrusted then
    Unenforced
  else
    get_enforcement ~this_class env ty

let compute_enforced_ty
    ~this_class env ?(explicitly_untrusted = false) (ty : decl_ty) =
  let et_enforced = get_enforced ~this_class env ~explicitly_untrusted ty in
  (et_enforced, ty)

let compute_enforced_and_pessimize_ty
    ~this_class env ?(explicitly_untrusted = false) (ty : decl_ty) =
  compute_enforced_ty ~this_class env ~explicitly_untrusted ty

let handle_awaitable_return ~this_class env ft_fun_kind (ft_ret : decl_ty) =
  match (ft_fun_kind, get_node ft_ret) with
  | (Ast_defs.FAsync, Tapply ((_, name), [_inner_ty]))
    when String.equal name Naming_special_names.Classes.cAwaitable ->
    ft_ret
  | _ -> snd (compute_enforced_and_pessimize_ty ~this_class env ft_ret)

let compute_enforced_and_pessimize_fun_type ~this_class env (ft : decl_fun_type)
    =
  let { ft_params; ft_ret; _ } = ft in
  let ft_fun_kind = get_ft_fun_kind ft in
  let ft_ret = handle_awaitable_return ~this_class env ft_fun_kind ft_ret in
  let ft_params =
    List.map
      ~f:(fun fp ->
        let { fp_type; _ } = fp in
        let f =
          if equal_param_mode (get_fp_mode fp) FPinout then
            compute_enforced_and_pessimize_ty ~this_class
          else
            compute_enforced_ty ~this_class
        in
        let (_, fp_type) = f env fp_type in
        { fp with fp_type })
      ft_params
  in
  { ft with ft_params; ft_ret }

let compute_enforced_ty = compute_enforced_ty ?explicitly_untrusted:None

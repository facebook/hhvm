(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Env = Typing_env
module Cls = Decl_provider.Class

(* Not adding a Typing_dep here because it will be added when the
 * Nast is fully processed (by the caller of this code) *)
let get_fun ctx name =
  match Decl_provider.get_fun ctx name with
  | Some { fe_type; _ } ->
    begin
      match get_node fe_type with
      | Tfun ft -> Some ft
      | _ -> None
    end
  | _ -> None

let get_static_meth
    (ctx : Provider_context.t) (cls_name : string) (meth_name : string) =
  match Decl_provider.get_class ctx cls_name with
  | None -> None
  | Some cls ->
    begin
      match Cls.get_smethod cls meth_name with
      | None -> None
      | Some { Typing_defs.ce_type = (lazy ty); _ } ->
        begin
          match get_node ty with
          | Tfun fty -> Some fty
          | _ -> None
        end
    end

let funopt_is_noreturn = function
  | Some { ft_ret = { et_type; _ }; _ } -> is_prim Tnoreturn et_type
  | _ -> false

let raise_exit_if_terminal f = if funopt_is_noreturn f then raise Exit

let static_meth_is_noreturn env ci meth_id =
  let class_name =
    match ci with
    | CI cls_id -> Some (snd cls_id)
    | CIself
    | CIstatic ->
      Env.get_self_id env
    | CIparent -> Env.get_parent_id env
    | CIexpr _ -> None
    (* we declared the types, but didn't check the bodies yet
                       so can't tell anything here *)
  in
  match class_name with
  | Some class_name ->
    funopt_is_noreturn
      (get_static_meth (Typing_env.get_ctx env) class_name (snd meth_id))
  | None -> false

let typed_expression_exits ((_, ty), e) =
  match e with
  | Assert (AE_assert (_, False))
  | Yield_break ->
    true
  | _ -> is_type_no_return (get_node ty)

let expression_exits env (_, e) =
  match e with
  | Assert (AE_assert (_, False))
  | Yield_break ->
    true
  | Call ((_, Id (_, fun_name)), _, _, _) ->
    funopt_is_noreturn @@ get_fun (Typing_env.get_ctx env) fun_name
  | Call ((_, Class_const ((_, ci), meth_id)), _, _, _) ->
    static_meth_is_noreturn env ci meth_id
  | _ -> false

let is_noreturn env =
  let (_env, ret_ty) =
    Env.expand_type
      env
      (Env.get_return env).Typing_env_return_info.return_type.et_type
  in
  is_prim Tnoreturn ret_ty

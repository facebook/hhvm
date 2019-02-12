(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Nast
open Typing_defs

module Env = Typing_env
module T = Tast
module TLazyHeap = Typing_lazy_heap
module Cls = Typing_classes_heap

(* Not adding a Typing_dep here because it will be added when the
 * Nast is fully processed (by the caller of this code) *)
let get_fun = TLazyHeap.get_fun

let get_static_meth (cls_name:string) (meth_name:string) =
  match TLazyHeap.get_class cls_name with
  | None -> None
  | Some cls ->
    begin match Cls.get_smethod cls meth_name with
      | None -> None
      | Some { Typing_defs.ce_type = lazy (_r, Typing_defs.Tfun fty) ; _} ->
        Some fty
      | Some _ -> None
    end

let funopt_is_noreturn = function
  | Some ({ Typing_defs.ft_ret = (_r, Typing_defs.Tprim Nast.Tnoreturn); _})
    -> true
  | _ -> false

let raise_exit_if_terminal f =
  begin if funopt_is_noreturn f then raise Exit; end

let static_meth_is_noreturn env ci meth_id =
  let class_name = match ci with
    | CI (cls_id) -> Some (snd cls_id)
    | CIself | CIstatic -> Some (Typing_env.get_self_id env)
    | CIparent -> Some (Typing_env.get_parent_id env)
    | CIexpr _ -> None (* we declared the types, but didn't check the bodies yet
                       so can't tell anything here *)
  in
  match class_name with
  | Some class_name ->
    funopt_is_noreturn (get_static_meth class_name (snd meth_id))
  | None -> false

let expression_exits env (_, e) =
  match e with
  | Assert(AE_assert (_, False))
  | Yield_break -> true
  | Call (Cnormal, (_, Id (_, fun_name)), _, _, _) ->
    funopt_is_noreturn @@ get_fun fun_name
  | Call (Cnormal, (_, Class_const ((_, ci), meth_id)), _, _, _) ->
    static_meth_is_noreturn env ci meth_id
  | _ -> false

let is_noreturn env =
  match (Env.get_return env).Typing_env_return_info.return_type with
  | _, Tprim Tnoreturn -> true
  | _ -> false

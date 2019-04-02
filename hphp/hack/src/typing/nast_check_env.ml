(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Nast

module SN = Naming_special_names

type control_context =
  | Toplevel
  | LoopContext
  | SwitchContext

type env = {
  tcopt: TypecheckerOptions.t;
  is_reactive: bool;
  class_kind: Ast.class_kind option;
  class_name: string option;
  function_name: string option;
  file_mode: FileInfo.mode;
  function_kind: Ast.fun_kind option;
  is_finally: bool;
  control_context: control_context;
  rx_is_enabled_allowed: bool;
}

let is_some_reactivity_attribute { ua_name = (_, name); _ } =
  name = SN.UserAttributes.uaReactive ||
  name = SN.UserAttributes.uaLocalReactive ||
  name = SN.UserAttributes.uaShallowReactive

(* During NastCheck, all reactivity kinds are the same *)
let fun_is_reactive user_attributes =
  List.exists user_attributes ~f:is_some_reactivity_attribute

let fun_env env f =
  { env with
    function_name = Some (snd f.f_name);
    is_reactive = env.is_reactive || fun_is_reactive f.f_user_attributes;
    file_mode = f.f_mode;
    function_kind = Some f.f_fun_kind;
  }

let method_env env m =
  { env with
    is_reactive = fun_is_reactive m.m_user_attributes;
    function_name = Some (snd m.m_name);
    function_kind = Some m.m_fun_kind;
  }

let class_env env c =
  { env with
    class_kind = Some c.c_kind;
    class_name = Some (snd c.c_name);
    file_mode = c.c_mode;
  }

let typedef_env env t =
  { env with file_mode = t.t_mode; }

let get_empty_env () = {
  tcopt = GlobalNamingOptions.get ();
  is_reactive = false;
  class_kind = None;
  class_name = None;
  function_name = None;
  file_mode = FileInfo.Mstrict;
  function_kind = None;
  is_finally = false;
  control_context = Toplevel;
  rx_is_enabled_allowed = false;
}

let def_env x =
  let empty_env = get_empty_env () in
  match x with
  | Fun f -> fun_env empty_env f
  | Class c -> class_env empty_env c
  | Typedef t -> typedef_env empty_env t
  | Constant _
  | Stmt _
  | Namespace _
  | NamespaceUse _
  | SetNamespaceEnv _
  | FileAttributes _ -> empty_env

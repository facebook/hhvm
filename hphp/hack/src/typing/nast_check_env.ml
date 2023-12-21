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

type control_context =
  | Toplevel
  | LoopContext
  | SwitchContext

type env = {
  ctx: Provider_context.t;
  classish_kind: Ast_defs.classish_kind option;
  class_name: string option;
  function_name: Ast_defs.id option;
  file_mode: FileInfo.mode;
  function_kind: Ast_defs.fun_kind option;
  is_finally: bool;
  control_context: control_context;
  array_append_allowed: bool;
  module_: Ast_defs.id option;
}

let get_tcopt env = Provider_context.get_tcopt env.ctx

let fun_env env f = { env with function_kind = Some f.f_fun_kind }

let fun_def_env env fd =
  {
    (fun_env env fd.fd_fun) with
    function_name = Some fd.fd_name;
    file_mode = fd.fd_mode;
    module_ = fd.fd_module;
  }

let method_env env m =
  { env with function_name = Some m.m_name; function_kind = Some m.m_fun_kind }

let class_env env c =
  {
    env with
    classish_kind = Some c.c_kind;
    class_name = Some (snd c.c_name);
    file_mode = c.c_mode;
    module_ = c.c_module;
  }

let typedef_env env t = { env with file_mode = t.t_mode; module_ = t.t_module }

let get_empty_env ctx =
  {
    ctx;
    classish_kind = None;
    class_name = None;
    function_name = None;
    file_mode = FileInfo.Mstrict;
    function_kind = None;
    is_finally = false;
    control_context = Toplevel;
    array_append_allowed = false;
    module_ = None;
  }

let def_env ctx x =
  let empty_env = get_empty_env ctx in
  match x with
  | Fun f -> fun_def_env empty_env f
  | Class c -> class_env empty_env c
  | Typedef t -> typedef_env empty_env t
  | SetModule _
  | Constant _
  | Stmt _
  | Namespace _
  | NamespaceUse _
  | SetNamespaceEnv _
  | FileAttributes _
  | Module _ ->
    empty_env

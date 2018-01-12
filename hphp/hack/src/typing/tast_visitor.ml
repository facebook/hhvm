(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module TEnv = Typing_env
module Dep = Typing_deps.Dep

open Tast

let fun_env f =
  let tcopt = f.f_annotation.tcopt in
  let filename = Pos.filename (fst f.f_name) in
  let dep = Dep.Fun (snd f.f_name) in
  let env = TEnv.empty tcopt filename (Some dep) in
  Tast_expand.restore_saved_env env f.f_annotation

let class_env c =
  let tcopt = c.c_annotation.tcopt in
  let filename = Pos.filename (fst c.c_name) in
  let dep = Dep.Class (snd c.c_name) in
  let env = TEnv.empty tcopt filename (Some dep) in
  Tast_expand.restore_saved_env env c.c_annotation

let typedef_env t =
  let tcopt = t.t_annotation.tcopt in
  let filename = Pos.filename (fst t.t_kind) in
  let dep = Dep.Class (snd t.t_name) in
  let env = TEnv.empty tcopt filename (Some dep) in
  Tast_expand.restore_saved_env env t.t_annotation

let gconst_env cst =
  let tcopt = cst.cst_annotation.tcopt in
  let filename = Pos.filename (fst cst.cst_name) in
  let dep = Dep.GConst (snd cst.cst_name) in
  let env = TEnv.empty tcopt filename (Some dep) in
  Tast_expand.restore_saved_env env cst.cst_annotation

let method_env env m =
  Tast_expand.restore_saved_env env m.m_annotation

class virtual ['self] reduce = object (self : 'self)
  inherit [_] Tast.reduce as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def def =
    match def with
    | Fun x -> self#go_fun_ x
    | Class x -> self#go_class_ x
    | Typedef x -> self#go_typedef x
    | Constant x -> self#go_gconst x

  method go_fun_    x = self#on_fun_ (fun_env x) x
  method go_class_  x = self#on_class_ (class_env x) x
  method go_typedef x = self#on_typedef (typedef_env x) x
  method go_gconst  x = self#on_gconst (gconst_env x) x

  method! on_method_ env x = super#on_method_ (method_env env x) x
end

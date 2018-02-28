(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module EnvFromDef = Typing_env_from_def.EnvFromDef(Tast.Annotations)

open Tast

let fun_env f =
  let env = EnvFromDef.fun_env f.f_annotation.tcopt f in
  Tast_expand.restore_saved_env env f.f_annotation

let class_env c =
  let env = EnvFromDef.class_env c.c_annotation.tcopt c in
  Tast_expand.restore_saved_env env c.c_annotation

let typedef_env t =
  let env = EnvFromDef.typedef_env t.t_annotation.tcopt t in
  Tast_expand.restore_saved_env env t.t_annotation

let gconst_env cst =
  let env = EnvFromDef.gconst_env cst.cst_annotation.tcopt cst in
  Tast_expand.restore_saved_env env cst.cst_annotation

let method_env env m =
  Tast_expand.restore_saved_env env m.m_annotation

class virtual ['self] iter = object (self : 'self)
  inherit [_] Tast.iter as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def def =
    match def with
    | Fun x -> self#go_Fun x
    | Class x -> self#go_Class x
    | Typedef x -> self#go_Typedef x
    | Constant x -> self#go_Constant x

  method go_Fun x = self#on_Fun (fun_env x) x
  method go_Class x = self#on_Class (class_env x) x
  method go_Typedef x = self#on_Typedef (typedef_env x) x
  method go_Constant x = self#on_Constant (gconst_env x) x

  method! on_method_ env x = super#on_method_ (method_env env x) x

  method! on_static_var env x =
    super#on_static_var (Typing_env.set_static env) x
  method! on_static_method env x =
    super#on_static_method (Typing_env.set_static env) x
end

class virtual ['self] reduce = object (self : 'self)
  inherit [_] Tast.reduce as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def def =
    match def with
    | Fun x -> self#go_Fun x
    | Class x -> self#go_Class x
    | Typedef x -> self#go_Typedef x
    | Constant x -> self#go_Constant x

  method go_Fun x = self#on_Fun (fun_env x) x
  method go_Class x = self#on_Class (class_env x) x
  method go_Typedef x = self#on_Typedef (typedef_env x) x
  method go_Constant x = self#on_Constant (gconst_env x) x

  method! on_method_ env x = super#on_method_ (method_env env x) x

  method! on_static_var env x =
    super#on_static_var (Typing_env.set_static env) x
  method! on_static_method env x =
    super#on_static_method (Typing_env.set_static env) x
end

class virtual ['self] map = object (self : 'self)
  inherit [_] Tast.map as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def def =
    match def with
    | Fun x -> self#go_Fun x
    | Class x -> self#go_Class x
    | Typedef x -> self#go_Typedef x
    | Constant x -> self#go_Constant x

  method go_Fun x = self#on_Fun (fun_env x) x
  method go_Class x = self#on_Class (class_env x) x
  method go_Typedef x = self#on_Typedef (typedef_env x) x
  method go_Constant x = self#on_Constant (gconst_env x) x

  method! on_method_ env x = super#on_method_ (method_env env x) x

  method! on_static_var env x =
    super#on_static_var (Typing_env.set_static env) x
  method! on_static_method env x =
    super#on_static_method (Typing_env.set_static env) x
end

class virtual ['self] endo = object (self : 'self)
  inherit [_] Tast.endo as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def def =
    match def with
    | Fun x -> self#go_Fun def x
    | Class x -> self#go_Class def x
    | Typedef x -> self#go_Typedef def x
    | Constant x -> self#go_Constant def x

  method go_Fun def x = self#on_Fun (fun_env x) def x
  method go_Class def x = self#on_Class (class_env x) def x
  method go_Typedef def x = self#on_Typedef (typedef_env x) def x
  method go_Constant def x = self#on_Constant (gconst_env x) def x

  method! on_method_ env x = super#on_method_ (method_env env x) x

  method! on_static_var env x =
    super#on_static_var (Typing_env.set_static env) x
  method! on_static_method env x =
    super#on_static_method (Typing_env.set_static env) x
end

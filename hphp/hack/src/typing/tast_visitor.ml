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
    | Fun x -> self#go_fun_ x
    | Class x -> self#go_class_ x
    | Typedef x -> self#go_typedef x
    | Constant x -> self#go_gconst x

  method go_fun_    x = self#on_fun_ (fun_env x) x
  method go_class_  x = self#on_class_ (class_env x) x
  method go_typedef x = self#on_typedef (typedef_env x) x
  method go_gconst  x = self#on_gconst (gconst_env x) x

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
    | Fun x -> self#go_fun_ x
    | Class x -> self#go_class_ x
    | Typedef x -> self#go_typedef x
    | Constant x -> self#go_gconst x

  method go_fun_    x = self#on_fun_ (fun_env x) x
  method go_class_  x = self#on_class_ (class_env x) x
  method go_typedef x = self#on_typedef (typedef_env x) x
  method go_gconst  x = self#on_gconst (gconst_env x) x

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
    | Fun x -> Fun (self#go_fun_ x)
    | Class x -> Class (self#go_class_ x)
    | Typedef x -> Typedef (self#go_typedef x)
    | Constant x -> Constant (self#go_gconst x)

  method go_fun_    x = self#on_fun_ (fun_env x) x
  method go_class_  x = self#on_class_ (class_env x) x
  method go_typedef x = self#on_typedef (typedef_env x) x
  method go_gconst  x = self#on_gconst (gconst_env x) x

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
    | Fun x ->
      let y = self#go_fun_ x in
      if x == y then def else Fun y
    | Class x ->
      let y = self#go_class_ x in
      if x == y then def else Class y
    | Typedef x ->
      let y = self#go_typedef x in
      if x == y then def else Typedef y
    | Constant x ->
      let y = self#go_gconst x in
      if x == y then def else Constant y

  method go_fun_    x = self#on_fun_ (fun_env x) x
  method go_class_  x = self#on_class_ (class_env x) x
  method go_typedef x = self#on_typedef (typedef_env x) x
  method go_gconst  x = self#on_gconst (gconst_env x) x

  method! on_method_ env x = super#on_method_ (method_env env x) x

  method! on_static_var env x =
    super#on_static_var (Typing_env.set_static env) x
  method! on_static_method env x =
    super#on_static_method (Typing_env.set_static env) x
end

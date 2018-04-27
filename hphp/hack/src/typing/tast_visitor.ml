(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env = Tast_env

class virtual ['self] iter = object (self : 'self)
  inherit [_] Tast.iter as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def x = self#on_def (Env.def_env x) x

  method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x
  method! on_method_ env x = super#on_method_ (Env.restore_method_env env x) x

  method! on_static_var env x = super#on_static_var (Env.set_static env) x
  method! on_static_method env x = super#on_static_method (Env.set_static env) x
end

class virtual ['self] reduce = object (self : 'self)
  inherit [_] Tast.reduce as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def x = self#on_def (Env.def_env x) x

  method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x
  method! on_method_ env x = super#on_method_ (Env.restore_method_env env x) x

  method! on_static_var env x = super#on_static_var (Env.set_static env) x
  method! on_static_method env x = super#on_static_method (Env.set_static env) x
end

class virtual ['self] map = object (self : 'self)
  inherit [_] Tast.map as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def x = self#on_def (Env.def_env x) x

  method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x
  method! on_method_ env x = super#on_method_ (Env.restore_method_env env x) x

  method! on_static_var env x = super#on_static_var (Env.set_static env) x
  method! on_static_method env x = super#on_static_method (Env.set_static env) x
end

class virtual ['self] endo = object (self : 'self)
  inherit [_] Tast.endo as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def x = self#on_def (Env.def_env x) x

  method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x
  method! on_method_ env x = super#on_method_ (Env.restore_method_env env x) x

  method! on_static_var env x = super#on_static_var (Env.set_static env) x
  method! on_static_method env x = super#on_static_method (Env.set_static env) x
end

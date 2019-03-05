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

type env = {
  is_reactive: bool;
  class_kind: Ast.class_kind option;
  class_name: string option;
  function_name: string option;
  file_mode: FileInfo.mode;
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
  }

let method_env env m =
  { env with
    is_reactive = fun_is_reactive m.m_user_attributes;
    function_name = Some (snd m.m_name);
  }

let class_env env c =
  { env with
    class_kind = Some c.c_kind;
    class_name = Some (snd c.c_name);
    file_mode = c.c_mode;
  }

let typedef_env env t =
  { env with file_mode = t.t_mode; }

let empty_env = {
  is_reactive = false;
  class_kind = None;
  class_name = None;
  function_name = None;
  file_mode = FileInfo.Mstrict;
}

let def_env x =
  match x with
  | Nast.Fun f -> fun_env empty_env f
  | Nast.Class c -> class_env empty_env c
  | Nast.Typedef t -> typedef_env empty_env t
  | Nast.Constant _
  | Nast.Stmt _
  | Nast.Namespace _
  | Nast.NamespaceUse _
  | Nast.SetNamespaceEnv _ -> empty_env

class virtual iter = object (self)
  inherit [_] Nast.iter as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def x = self#on_def (def_env x) x

  method! on_fun_ env x = super#on_fun_ (fun_env env x) x
  method! on_method_ env x = super#on_method_ (method_env env x) x
  method! on_class_ env x = super#on_class_ (class_env env x) x
end

class virtual ['state] iter_with_state = object (self)
  inherit [_] Nast.iter as super

  (* Entry point *)
  method go (state: 'state) program =
    self#on_list (fun () -> self#go_def state) () program

  method go_def state x = self#on_def (def_env x, state) x

  method! on_fun_ (env, state) x =
    super#on_fun_ (fun_env env x, state) x

end

class type handler = object

  method at_fun_ : env -> Nast.fun_ -> unit
  method at_class_ : env -> Nast.class_ -> unit
  method at_method_ : env -> Nast.method_ -> unit
  method at_expr : env -> Nast.expr -> unit
  method at_stmt : env -> Nast.stmt -> unit
  method at_hint : env -> Nast.hint -> unit
end

class virtual handler_base : handler = object

  method at_fun_ _ _ = ()
  method at_class_ _ _ = ()
  method at_method_ _ _ = ()
  method at_expr _ _ = ()
  method at_stmt _ _ = ()
  method at_hint _ _ = ()

end

let iter_with (handlers : handler list) : iter = object

  inherit iter as super

  method! on_fun_ env x =
    List.iter handlers (fun v -> v#at_fun_ env x);
    super#on_fun_ env x;

  method! on_class_ env x =
    List.iter handlers (fun v -> v#at_class_ env x);
    super#on_class_ env x;

  method! on_method_ env x =
    List.iter handlers (fun v -> v#at_method_ env x);
    super#on_method_ env x;

  method! on_expr env x =
    List.iter handlers (fun v -> v#at_expr env x);
    super#on_expr env x;

  method! on_stmt env x =
    List.iter handlers (fun v -> v#at_stmt env x);
    super#on_stmt env x;

  method! on_hint env h =
    List.iter handlers (fun v -> v#at_hint env h);
    super#on_hint env h;

end

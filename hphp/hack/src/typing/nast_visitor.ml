(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)


(*
  Add env here so we're not dependent on removing
  tenv from nastCheck just yet.
  - We can add fields as we need them to the
  Nast_env we've created here.
*)
open Core_kernel
open Nast


type env = {
  def_type: string;
  is_reactive: bool;
}

let is_some_reactivity_attribute { ua_name = (_, name); _ } =
  name = SN.UserAttributes.uaReactive ||
  name = SN.UserAttributes.uaLocalReactive ||
  name = SN.UserAttributes.uaShallowReactive

(* During NastCheck, all reactivity kinds are the same *)
let fun_is_reactive user_attributes =
  List.exists user_attributes ~f:is_some_reactivity_attribute

let fun_env env f =
  { def_type = "fun";
    is_reactive = env.is_reactive || fun_is_reactive f.f_user_attributes; }

let method_env env m =
  { env with
    is_reactive = fun_is_reactive m.m_user_attributes; }

let empty_env = { def_type = ""; is_reactive = false; }

let def_env x =
  match x with
  | Nast.Fun f -> fun_env empty_env f
  | Nast.Class _ -> empty_env
  | Nast.Typedef _ -> empty_env
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
end


class type handler = object

  method at_fun_ : env -> Nast.fun_ -> unit
  method at_class_ : env -> Nast.class_ -> unit
  method at_method_ : env -> Nast.method_ -> unit

end

class virtual handler_base : handler = object

  method at_fun_ _ _ = ()
  method at_class_ _ _ = ()
  method at_method_ _ _ = ()
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

end

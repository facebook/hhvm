(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

module Env = Tast_env

class virtual iter = object (self)
  inherit [_] Tast.iter as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def x = self#on_def (Env.def_env x) x

  method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x
  method! on_method_ env x = super#on_method_ (Env.restore_method_env env x) x

  method! on_constructor env x = super#on_constructor (Env.set_inside_constructor env) x
  method! on_static_var env x = super#on_static_var (Env.set_static env) x
  method! on_static_method env x = super#on_static_method (Env.set_static env) x
end

class virtual ['a] reduce = object (self)
  inherit [_] Tast.reduce as super

  (* Entry point *)
  method go program : 'a = self#on_list (fun () -> self#go_def) () program

  method go_def x = self#on_def (Env.def_env x) x

  method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x
  method! on_method_ env x = super#on_method_ (Env.restore_method_env env x) x

  method! on_constructor env x = super#on_constructor (Env.set_inside_constructor env) x
  method! on_static_var env x = super#on_static_var (Env.set_static env) x
  method! on_static_method env x = super#on_static_method (Env.set_static env) x
end

class virtual map = object (self)
  inherit [_] Tast.map as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def x = self#on_def (Env.def_env x) x

  method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x
  method! on_method_ env x = super#on_method_ (Env.restore_method_env env x) x

  method! on_constructor env x = super#on_constructor (Env.set_inside_constructor env) x
  method! on_static_var env x = super#on_static_var (Env.set_static env) x
  method! on_static_method env x = super#on_static_method (Env.set_static env) x
end

class virtual endo = object (self)
  inherit [_] Tast.endo as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def x = self#on_def (Env.def_env x) x

  method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x
  method! on_method_ env x = super#on_method_ (Env.restore_method_env env x) x

  method! on_constructor env x = super#on_constructor (Env.set_inside_constructor env) x
  method! on_static_var env x = super#on_static_var (Env.set_static env) x
  method! on_static_method env x = super#on_static_method (Env.set_static env) x
end

(** A {!handler} is an {!iter} visitor which is not in control of the iteration
    (and thus cannot change the order of the iteration or choose not to visit
    some subtrees).

    Intended to be used with {!iter_with} to aggregate many checks into a
    single pass over a TAST. *)
class type handler = object
  method minimum_forward_compat_level : int

  method at_fun_ : Env.t -> Tast.fun_ -> unit
  method at_class_ : Env.t -> Tast.class_ -> unit
  method at_typedef : Env.t -> Tast.typedef -> unit
  method at_gconst : Env.t -> Tast.gconst -> unit

  method at_expr : Env.t -> Tast.expr -> unit
  method at_stmt : Env.t -> Tast.stmt -> unit
  method at_Call :
    Env.t ->
    Aast.call_type ->
    Tast.expr ->
    Aast.hint list ->
    Tast.expr list ->
    Tast.expr list ->
    unit
end

(** A {!handler} which does not need to make use of every visitation method can
    inherit from this no-op base class. *)
class virtual handler_base : handler = object
  method minimum_forward_compat_level = 0

  method at_fun_ _ _ = ()
  method at_class_ _ _ = ()
  method at_typedef _ _ = ()
  method at_gconst _ _ = ()

  method at_expr _ _ = ()
  method at_stmt _ _ = ()
  method at_Call _ _ _ _ _ _ = ()
end

let if_enabled env f handler =
  Env.error_if_forward_compat_ge handler#minimum_forward_compat_level env (fun () -> f handler)

(** Return an {!iter} visitor which invokes all of the given handlers upon
    visiting each node. *)
let iter_with (handlers : handler list) : iter = object

  inherit iter as super

  method! on_fun_ env x =
    List.iter handlers (if_enabled env (fun v -> v#at_fun_ env x));
    super#on_fun_ env x;

  method! on_class_ env x =
    List.iter handlers (if_enabled env (fun v -> v#at_class_ env x));
    super#on_class_ env x;

  method! on_typedef env x =
    List.iter handlers (if_enabled env (fun v -> v#at_typedef env x));
    super#on_typedef env x;

  method! on_gconst env x =
    List.iter handlers (if_enabled env (fun v -> v#at_gconst env x));
    super#on_gconst env x;

  method! on_expr env x =
    List.iter handlers (if_enabled env (fun v -> v#at_expr env x));
    super#on_expr env x;

  method! on_stmt env x =
    List.iter handlers (if_enabled env (fun v -> v#at_stmt env x));
    super#on_stmt env x;

  method! on_Call env ct e hl el uel =
    List.iter handlers (if_enabled env (fun v -> v#at_Call env ct e hl el uel));
    super#on_Call env ct e hl el uel;
end

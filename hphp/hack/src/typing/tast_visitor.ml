(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

module Env = Tast_env

class virtual iter = object (self)
  inherit [_] Tast.iter as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def x = self#on_def (Env.def_env x) x

  method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x
  method! on_method_ env x = super#on_method_ (Env.restore_method_env env x) x

  method! on_constructor env = super#on_constructor (Env.set_inside_constructor env)
  method! on_static_var env = super#on_static_var (Env.set_static env)
  method! on_static_method env = super#on_static_method (Env.set_static env)

  method! on_Efun env x = super#on_Efun (Env.set_ppl_lambda env) x
  method! on_Lfun env x = super#on_Lfun (Env.set_ppl_lambda env) x

  (* By default, ignore unsafe code. To visit it, use {!iter_unsafe}. *)
  method! on_Unsafe_expr _ _ = ()
  method! on_Unsafe_block _ _ = ()
end

class virtual ['state] iter_with_state = object (self)
  inherit [_] Tast.iter as super

  (* Entry point *)
  method go (state: 'state) program =
    self#on_list (fun () -> self#go_def state) () program

  method go_def state x = self#on_def (Env.def_env x, state) x

  method! on_fun_ (env, state) x =
    super#on_fun_ (Env.restore_fun_env env x, state) x
  method! on_method_ (env, state) x =
    super#on_method_ (Env.restore_method_env env x, state) x

  method! on_constructor (env, state) =
    super#on_constructor (Env.set_inside_constructor env, state)
  method! on_static_var (env, state) =
    super#on_static_var (Env.set_static env, state)
  method! on_static_method (env, state) =
    super#on_static_method (Env.set_static env, state)

  method! on_Efun (env, state) x =
    super#on_Efun (Env.set_ppl_lambda env, state) x
  method! on_Lfun (env, state) x =
    super#on_Lfun (Env.set_ppl_lambda env, state) x

  (* Ignore unsafe code. *)
  method! on_Unsafe_expr _ _ = ()
  method! on_Unsafe_block _ _ = ()
end

(** Like {!iter}, but visits unsafe code. Should not be used in the typechecker
    or typed linters. Unsafe code should not be visible to the typechecker. *)
class virtual iter_unsafe = object (self)
  inherit iter
  method! on_Unsafe_expr = self#on_expr
  method! on_Unsafe_block = self#on_block
end

class virtual ['a] reduce = object (self)
  inherit [_] Tast.reduce as super

  (* Entry point *)
  method go program : 'a = self#on_list (fun () -> self#go_def) () program

  method go_def x = self#on_def (Env.def_env x) x

  method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x
  method! on_method_ env x = super#on_method_ (Env.restore_method_env env x) x

  method! on_constructor env = super#on_constructor (Env.set_inside_constructor env)
  method! on_static_var env = super#on_static_var (Env.set_static env)
  method! on_static_method env = super#on_static_method (Env.set_static env)

  method! on_Efun env x = super#on_Efun (Env.set_ppl_lambda env) x
  method! on_Lfun env x = super#on_Lfun (Env.set_ppl_lambda env) x

  (* By default, ignore unsafe code. To visit it, use {!reduce_unsafe}. *)
  method! on_Unsafe_expr _ _ = self#zero
  method! on_Unsafe_block _ _ = self#zero
end

(** Like {!reduce}, but visits unsafe code. Should not be used in the
    typechecker or typed linters. Unsafe code should not be visible to the
    typechecker. *)
class virtual ['a] reduce_unsafe = object (self)
  inherit ['a] reduce
  method! on_Unsafe_expr = self#on_expr
  method! on_Unsafe_block = self#on_block
end

class virtual map = object (self)
  inherit [_] Tast.map as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def x = self#on_def (Env.def_env x) x

  method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x
  method! on_method_ env x = super#on_method_ (Env.restore_method_env env x) x

  method! on_constructor env = super#on_constructor (Env.set_inside_constructor env)
  method! on_static_var env = super#on_static_var (Env.set_static env)
  method! on_static_method env = super#on_static_method (Env.set_static env)

  method! on_Efun env x = super#on_Efun (Env.set_ppl_lambda env) x
  method! on_Lfun env x = super#on_Lfun (Env.set_ppl_lambda env) x

  (* By default, ignore unsafe code. To visit it, use {!map_unsafe}. *)
  method! on_Unsafe_expr _ e = Tast.Unsafe_expr e
  method! on_Unsafe_block _ b = Tast.Unsafe_block b
end

(** Like {!map}, but visits unsafe code. Should not be used in the typechecker
    or typed linters. Unsafe code should not be visible to the typechecker. *)
class virtual map_unsafe = object (self)
  inherit map
  method! on_Unsafe_expr env e = Tast.Unsafe_expr (self#on_expr env e)
  method! on_Unsafe_block env b = Tast.Unsafe_block (self#on_block env b)
end

class virtual endo = object (self)
  inherit [_] Tast.endo as super

  (* Entry point *)
  method go program = self#on_list (fun () -> self#go_def) () program

  method go_def x = self#on_def (Env.def_env x) x

  method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x
  method! on_method_ env x = super#on_method_ (Env.restore_method_env env x) x

  method! on_constructor env = super#on_constructor (Env.set_inside_constructor env)
  method! on_static_var env = super#on_static_var (Env.set_static env)
  method! on_static_method env = super#on_static_method (Env.set_static env)

  method! on_Efun env x = super#on_Efun (Env.set_ppl_lambda env) x
  method! on_Lfun env x = super#on_Lfun (Env.set_ppl_lambda env) x

  (* By default, ignore unsafe code. To visit it, use {!endo_unsafe}. *)
  method! on_Unsafe_expr _ x _ = x
  method! on_Unsafe_block _ x _ = x
end

(** Like {!endo}, but visits unsafe code. Should not be used in the typechecker
    or typed linters. Unsafe code should not be visible to the typechecker. *)
class virtual endo_unsafe = object (self)
  inherit endo
  method! on_Unsafe_expr env x e =
    let e' = self#on_expr env e in
    if e = e' then x else Tast.Unsafe_expr e'
  method! on_Unsafe_block env x b =
    let b' = self#on_block env b in
    if b = b' then x else Tast.Unsafe_block b'
end

(** A {!handler} is an {!iter} visitor which is not in control of the iteration
    (and thus cannot change the order of the iteration or choose not to visit
    some subtrees).

    Intended to be used with {!iter_with} to aggregate many checks into a
    single pass over a TAST. *)
class type handler = object
  method at_class_ : Env.t -> Tast.class_ -> unit
  method at_typedef : Env.t -> Tast.typedef -> unit
  method at_gconst : Env.t -> Tast.gconst -> unit
  method at_fun_def : Env.t -> Tast.fun_def -> unit
  method at_method_ : Env.t -> Tast.method_ -> unit
  method at_static_method : Env.t -> Tast.static_method -> unit
  method at_constructor : Env.t -> Tast.constructor -> unit

  method at_expr : Env.t -> Tast.expr -> unit
  method at_stmt : Env.t -> Tast.stmt -> unit
  method at_fun_ : Env.t -> Tast.fun_ -> unit
  method at_Call :
    Env.t ->
    Aast.call_type ->
    Tast.expr ->
    Aast.targ list ->
    Tast.expr list ->
    Tast.expr list ->
    unit
  method at_hint : Env.t -> Aast.hint -> unit
  method at_tparam : Env.t -> Tast.tparam -> unit
  method at_user_attribute : Env.t -> Tast.user_attribute -> unit
  method at_class_typeconst : Env.t -> Tast.class_typeconst -> unit

  method at_Is : Env.t -> Tast.expr -> Tast.hint -> unit
  method at_As : Env.t -> Tast.expr -> Tast.hint -> unit
end

(** A {!handler} which does not need to make use of every visitation method can
    inherit from this no-op base class. *)
class virtual handler_base : handler = object
  method at_class_ _ _ = ()
  method at_typedef _ _ = ()
  method at_gconst _ _ = ()
  method at_fun_def _ _ = ()
  method at_method_ _ _ = ()
  method at_static_method _ _ = ()
  method at_constructor _ _ = ()

  method at_expr _ _ = ()
  method at_stmt _ _ = ()
  method at_fun_ _ _ = ()
  method at_Call _ _ _ _ _ _ = ()
  method at_hint _ _ = ()
  method at_tparam _ _ = ()
  method at_user_attribute _ _ = ()
  method at_class_typeconst _ _ = ()

  method at_Is _ _ _ = ()
  method at_As _ _ _ = ()
end

(** Return an {!iter} visitor which invokes all of the given handlers upon
    visiting each node. *)
let iter_with (handlers : handler list) : iter = object

  inherit iter as super

  method! on_class_ env x =
    List.iter handlers (fun v -> v#at_class_ env x);
    super#on_class_ env x;

  method! on_typedef env x =
    List.iter handlers (fun v -> v#at_typedef env x);
    super#on_typedef env x;

  method! on_gconst env x =
    List.iter handlers (fun v -> v#at_gconst env x);
    super#on_gconst env x;

  method! on_fun_def env x =
    List.iter handlers (fun v -> v#at_fun_def env x);
    super#on_fun_def env x;

  method! on_method_ env x =
    List.iter handlers (fun v -> v#at_method_ env x);
    super#on_method_ env x;

  method! on_static_method env m =
    List.iter handlers (fun v -> v#at_static_method env m);
    super#on_static_method env m;

  method! on_expr env x =
    List.iter handlers (fun v -> v#at_expr env x);
    super#on_expr env x;

  method! on_stmt env x =
    List.iter handlers (fun v -> v#at_stmt env x);
    super#on_stmt env x;

  method! on_fun_ env x =
    List.iter handlers (fun v -> v#at_fun_ env x);
    super#on_fun_ env x;

  method! on_Call env ct e tal el uel =
    List.iter handlers (fun v -> v#at_Call env ct e tal el uel);
    super#on_Call env ct e tal el uel;

  method! on_hint env h =
    List.iter handlers (fun v -> v#at_hint env h);
    super#on_hint env h;

  method! on_constructor env h =
    List.iter handlers (fun v -> v#at_constructor env h);
    super#on_constructor env h;

  method! on_tparam env h =
    List.iter handlers (fun v -> v#at_tparam env h);
    super#on_tparam env h;

  method! on_user_attribute env ua =
    List.iter handlers (fun v -> v#at_user_attribute env ua);
    super#on_user_attribute env ua;

  method! on_class_typeconst env tc =
    List.iter handlers (fun v -> v#at_class_typeconst env tc);
    super#on_class_typeconst env tc;

  method! on_Is env e h =
    let env = Env.set_allow_wildcards env in
    List.iter handlers (fun v -> v#at_Is env e h);
    super#on_Is env e h;

  method! on_As env e h =
    let env = Env.set_allow_wildcards env in
    List.iter handlers (fun v -> v#at_As env e h);
    super#on_As env e h;

end

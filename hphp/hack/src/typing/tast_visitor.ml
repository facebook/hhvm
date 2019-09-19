(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module Env = Tast_env

class virtual iter =
  object (self)
    inherit [_] Aast.iter as super

    (* Entry point *)
    method go program = self#on_list (fun () -> self#go_def) () program

    method go_def x = self#on_def (Env.def_env x) x

    method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x

    method! on_method_ env x =
      let env =
        if snd x.Aast.m_name = Naming_special_names.Members.__construct then
          Env.set_inside_constructor env
        else if x.Aast.m_static then
          Env.set_static env
        else
          env
      in
      super#on_method_ (Env.restore_method_env env x) x

    method! on_class_var env cv =
      let env =
        if cv.Aast.cv_is_static then
          Env.set_static env
        else
          env
      in
      super#on_class_var env cv

    method! on_Efun env x = super#on_Efun (Env.set_ppl_lambda env) x

    method! on_Lfun env x = super#on_Lfun (Env.set_ppl_lambda env) x

    method! on_Binop env op e1 e2 =
      match op with
      | Ast_defs.Eq _ ->
        self#on_expr (Env.set_val_kind env Typing_defs.Lval) e1;
        self#on_expr env e2
      | _ -> super#on_Binop env op e1 e2
  end

class virtual ['state] iter_with_state =
  object (self)
    inherit [_] Aast.iter as super

    (* Entry point *)
    method go (state : 'state) program =
      self#on_list (fun () -> self#go_def state) () program

    method go_def state x = self#on_def (Env.def_env x, state) x

    method! on_fun_ (env, state) x =
      super#on_fun_ (Env.restore_fun_env env x, state) x

    method! on_method_ (env, state) x =
      let env =
        if snd x.Aast.m_name = Naming_special_names.Members.__construct then
          Env.set_inside_constructor env
        else if x.Aast.m_static then
          Env.set_static env
        else
          env
      in
      super#on_method_ (Env.restore_method_env env x, state) x

    method! on_class_var (env, state) cv =
      let env =
        if cv.Aast.cv_is_static then
          Env.set_static env
        else
          env
      in
      super#on_class_var (env, state) cv

    method! on_Efun (env, state) x =
      super#on_Efun (Env.set_ppl_lambda env, state) x

    method! on_Lfun (env, state) x =
      super#on_Lfun (Env.set_ppl_lambda env, state) x
  end

class virtual ['a] reduce =
  object (self)
    inherit [_] Aast.reduce as super

    (* Entry point *)
    method go program : 'a = self#on_list (fun () -> self#go_def) () program

    method go_def x = self#on_def (Env.def_env x) x

    method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x

    method! on_method_ env x =
      let env =
        if snd x.Aast.m_name = Naming_special_names.Members.__construct then
          Env.set_inside_constructor env
        else if x.Aast.m_static then
          Env.set_static env
        else
          env
      in
      super#on_method_ (Env.restore_method_env env x) x

    method! on_class_var env cv =
      let env =
        if cv.Aast.cv_is_static then
          Env.set_static env
        else
          env
      in
      super#on_class_var env cv

    method! on_Efun env x = super#on_Efun (Env.set_ppl_lambda env) x

    method! on_Lfun env x = super#on_Lfun (Env.set_ppl_lambda env) x
  end

class virtual map =
  object (self)
    inherit [_] Aast.map as super

    method on_'ex _ ex = ex

    method on_'fb _ fb = fb

    method on_'en _ en = en

    method on_'hi _ hi = hi

    (* Entry point *)
    method go program : Tast.program =
      self#on_list (fun () -> self#go_def) () program

    method go_def x = self#on_def (Env.def_env x) x

    method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x

    method! on_method_ env x =
      let env =
        if snd x.Aast.m_name = Naming_special_names.Members.__construct then
          Env.set_inside_constructor env
        else if x.Aast.m_static then
          Env.set_static env
        else
          env
      in
      super#on_method_ (Env.restore_method_env env x) x

    method! on_class_var env cv =
      let env =
        if cv.Aast.cv_is_static then
          Env.set_static env
        else
          env
      in
      super#on_class_var env cv

    method! on_Efun env x = super#on_Efun (Env.set_ppl_lambda env) x

    method! on_Lfun env x = super#on_Lfun (Env.set_ppl_lambda env) x
  end

class virtual endo =
  object (self)
    inherit [_] Aast.endo as super

    method on_'ex _ ex = ex

    method on_'fb _ fb = fb

    method on_'en _ en = en

    method on_'hi _ hi = hi

    (* Entry point *)
    method go program = self#on_list (fun () -> self#go_def) () program

    method go_def x = self#on_def (Env.def_env x) x

    method! on_fun_ env x = super#on_fun_ (Env.restore_fun_env env x) x

    method! on_method_ env x =
      let env =
        if snd x.Aast.m_name = Naming_special_names.Members.__construct then
          Env.set_inside_constructor env
        else if x.Aast.m_static then
          Env.set_static env
        else
          env
      in
      super#on_method_ (Env.restore_method_env env x) x

    method! on_class_var env cv =
      let env =
        if cv.Aast.cv_is_static then
          Env.set_static env
        else
          env
      in
      super#on_class_var env cv

    method! on_Efun env x = super#on_Efun (Env.set_ppl_lambda env) x

    method! on_Lfun env x = super#on_Lfun (Env.set_ppl_lambda env) x
  end

(** A {!handler} is an {!iter} visitor which is not in control of the iteration
    (and thus cannot change the order of the iteration or choose not to visit
    some subtrees).

    Intended to be used with {!iter_with} to aggregate many checks into a
    single pass over a TAST. *)
class type handler =
  object
    method at_class_ : Env.t -> Tast.class_ -> unit

    method at_typedef : Env.t -> Tast.typedef -> unit

    method at_gconst : Env.t -> Tast.gconst -> unit

    method at_fun_def : Env.t -> Tast.fun_def -> unit

    method at_method_ : Env.t -> Tast.method_ -> unit

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
class virtual handler_base : handler =
  object
    method at_class_ _ _ = ()

    method at_typedef _ _ = ()

    method at_gconst _ _ = ()

    method at_fun_def _ _ = ()

    method at_method_ _ _ = ()

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
let iter_with (handlers : handler list) : iter =
  object
    inherit iter as super

    method! on_class_ env x =
      List.iter handlers (fun v -> v#at_class_ env x);
      super#on_class_ env x

    method! on_typedef env x =
      List.iter handlers (fun v -> v#at_typedef env x);
      super#on_typedef env x

    method! on_gconst env x =
      List.iter handlers (fun v -> v#at_gconst env x);
      super#on_gconst env x

    method! on_fun_def env x =
      List.iter handlers (fun v -> v#at_fun_def env x);
      super#on_fun_def env x

    method! on_method_ env x =
      List.iter handlers (fun v -> v#at_method_ env x);
      super#on_method_ env x

    method! on_expr env x =
      List.iter handlers (fun v -> v#at_expr env x);
      super#on_expr env x

    method! on_stmt env x =
      List.iter handlers (fun v -> v#at_stmt env x);
      super#on_stmt env x

    method! on_fun_ env x =
      List.iter handlers (fun v -> v#at_fun_ env x);
      super#on_fun_ env x

    method! on_Call env ct e tal el uel =
      List.iter handlers (fun v -> v#at_Call env ct e tal el uel);
      super#on_Call env ct e tal el uel

    method! on_hint env h =
      List.iter handlers (fun v -> v#at_hint env h);
      super#on_hint env h

    method! on_tparam env h =
      List.iter handlers (fun v -> v#at_tparam env h);
      super#on_tparam env h

    method! on_user_attribute env ua =
      List.iter handlers (fun v -> v#at_user_attribute env ua);
      super#on_user_attribute env ua

    method! on_class_typeconst env tc =
      List.iter handlers (fun v -> v#at_class_typeconst env tc);
      super#on_class_typeconst env tc

    method! on_Is env e h =
      let env = Env.set_allow_wildcards env in
      List.iter handlers (fun v -> v#at_Is env e h);
      super#on_Is env e h

    method! on_As env e h =
      let env = Env.set_allow_wildcards env in
      List.iter handlers (fun v -> v#at_As env e h);
      super#on_As env e h
  end

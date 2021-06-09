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
open Nast_check_env

class virtual iter =
  object (self)
    inherit [_] Aast.iter as super

    (* Entry point *)
    method go (ctx : Provider_context.t) (program : Nast.program) =
      self#on_list (fun () -> self#go_def ctx) () program

    method go_def ctx x = self#on_def (def_env ctx x) x

    method! on_fun_def env x = super#on_fun_def (fun_def_env env x) x

    method! on_fun_ env x = super#on_fun_ (fun_env env x) x

    method! on_method_ env x = super#on_method_ (method_env env x) x

    method! on_class_ env x = super#on_class_ (class_env env x) x

    method! on_Try env b cl fb =
      self#on_block { env with is_finally = true } fb;
      super#on_Try env b cl fb

    method! on_Do env = super#on_Do { env with control_context = LoopContext }

    method! on_While env =
      super#on_While { env with control_context = LoopContext }

    method! on_For env = super#on_For { env with control_context = LoopContext }

    method! on_Foreach env =
      super#on_Foreach { env with control_context = LoopContext }

    method! on_Switch env =
      super#on_Switch { env with control_context = SwitchContext }

    method! on_Efun env =
      super#on_Efun { env with is_finally = false; control_context = Toplevel }

    method! on_Lfun env =
      super#on_Lfun { env with is_finally = false; control_context = Toplevel }

    method! on_Obj_get env =
      super#on_Obj_get { env with array_append_allowed = false }

    method! on_Array_get env =
      super#on_Array_get { env with array_append_allowed = false }

    method! on_Binop env op e1 e2 =
      match op with
      | Ast_defs.Eq None ->
        self#on_expr { env with array_append_allowed = true } e1;
        self#on_expr env e2
      | _ -> super#on_Binop env op e1 e2

    method! on_func_body env fb =
      match fb.fb_ast with
      | [(_, If (((_, Id (_, _)) as id), then_stmt, else_stmt))] ->
        super#on_expr env id;
        self#on_block env then_stmt;
        self#on_block env else_stmt
      | _ -> super#on_func_body env fb

    method! on_expr env e =
      match e with
      | (_, Call (e1, ta, el, unpacked_element)) ->
        self#on_Call env e1 ta el unpacked_element
      | (_, Binop (Ast_defs.Eq None, e1, rhs)) ->
        self#on_Binop env (Ast_defs.Eq None) e1 rhs
      | _ -> super#on_expr env e
  end

class virtual ['state] iter_with_state =
  object (self)
    inherit [_] Aast.iter as super

    (* Entry point *)
    method go
        (state : 'state) (ctx : Provider_context.t) (program : Nast.program) =
      self#on_list (fun () -> self#go_def state ctx) () program

    method go_def state ctx x = self#on_def (def_env ctx x, state) x

    method! on_fun_def (env, state) x =
      super#on_fun_def (fun_def_env env x, state) x
  end

class type handler =
  object
    method at_fun_def : env -> Nast.fun_def -> unit

    method at_fun_ : env -> Nast.fun_ -> unit

    method at_class_ : env -> Nast.class_ -> unit

    method at_method_ : env -> Nast.method_ -> unit

    method at_record_def : env -> Nast.record_def -> unit

    method at_expr : env -> Nast.expr -> unit

    method at_stmt : env -> Nast.stmt -> unit

    method at_hint : env -> hint -> unit

    method at_contexts : env -> contexts -> unit

    method at_typedef : env -> Nast.typedef -> unit

    method at_gconst : env -> Nast.gconst -> unit
  end

class virtual handler_base : handler =
  object
    method at_fun_def _ _ = ()

    method at_fun_ _ _ = ()

    method at_class_ _ _ = ()

    method at_method_ _ _ = ()

    method at_record_def _ _ = ()

    method at_expr _ _ = ()

    method at_stmt _ _ = ()

    method at_hint _ _ = ()

    method at_contexts _ _ = ()

    method at_typedef _ _ = ()

    method at_gconst _ _ = ()
  end

let iter_with (handlers : handler list) : iter =
  object
    inherit iter as super

    method! on_fun_def env x =
      List.iter handlers ~f:(fun v -> v#at_fun_def env x);
      super#on_fun_def env x

    method! on_fun_ env x =
      List.iter handlers ~f:(fun v -> v#at_fun_ env x);
      super#on_fun_ env x

    method! on_class_ env x =
      List.iter handlers ~f:(fun v -> v#at_class_ env x);
      super#on_class_ env x

    method! on_method_ env x =
      List.iter handlers ~f:(fun v -> v#at_method_ env x);
      super#on_method_ env x

    method! on_record_def env x =
      List.iter handlers ~f:(fun v -> v#at_record_def env x);
      super#on_record_def env x

    method! on_expr env x =
      List.iter handlers ~f:(fun v -> v#at_expr env x);
      super#on_expr env x

    method! on_stmt env x =
      List.iter handlers ~f:(fun v -> v#at_stmt env x);
      super#on_stmt env x

    method! on_hint env h =
      List.iter handlers ~f:(fun v -> v#at_hint env h);
      super#on_hint env h

    method! on_contexts env cl =
      List.iter handlers ~f:(fun v -> v#at_contexts env cl);
      super#on_contexts env cl

    method! on_typedef env t =
      List.iter handlers ~f:(fun v -> v#at_typedef env t);
      super#on_typedef env t

    method! on_gconst env gconst =
      List.iter handlers ~f:(fun v -> v#at_gconst env gconst);
      super#on_gconst env gconst
  end

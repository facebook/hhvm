(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast

type env = { in_lvalue: bool }

let visitor =
  object (self)
    inherit [_] Aast.iter as super

    method! on_expr env ((_, p, expr_) as e) =
      match expr_ with
      | Binop Aast.{ bop = Ast_defs.Eq None; lhs = e1; rhs = e2 } ->
        (* Allow list($foo) = $bar; *)
        self#on_expr { in_lvalue = true } e1;
        self#on_expr env e2
      | List _ ->
        if env.in_lvalue then
          (* list() can nest, so allow list(list($x)) = $foo; *)
          super#on_expr env e
        else
          (* Ban list() in rvalue positions, e.g. foo(list($bar)) *)
          Errors.add_error Nast_check_error.(to_user_error @@ List_rvalue p)
      | _ -> super#on_expr { in_lvalue = false } e

    method! on_stmt env s =
      match snd s with
      | Aast.Foreach (val_expr, as_expr, body) ->
        self#on_expr env val_expr;
        (* Allow foreach($foo as list($x, $y)) { ... } *)
        self#on_as_expr { in_lvalue = true } as_expr;
        self#on_block env body
      | _ -> super#on_stmt env s
  end

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_ _env f = visitor#on_fun_ { in_lvalue = false } f

    method! at_method_ _env m = visitor#on_method_ { in_lvalue = false } m
  end

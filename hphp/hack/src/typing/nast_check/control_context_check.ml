(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* open Core_kernel *)
open Nast

module SN = Naming_special_names

type control_context =
| Toplevel
| LoopContext
| SwitchContext

type ctx = {
  control_context: control_context;
  is_finally: bool;
}

let default_ctx = {
  control_context = Toplevel;
  is_finally = false;
}

let visitor = object(this)
  inherit [ctx] Nast_visitor.iter_with_state as super

  method! on_Do (env, ctx) = super#on_Do (env, { ctx with control_context = LoopContext })
  method! on_While (env, ctx) = super#on_While (env, { ctx with control_context = LoopContext })
  method! on_For (env, ctx) = super#on_For (env, { ctx with control_context = LoopContext })
  method! on_Foreach (env, ctx) = super#on_Foreach (env, { ctx with control_context = LoopContext })
  method! on_Switch (env, ctx) = super#on_Switch (env, { ctx with control_context = SwitchContext })
  method! on_Efun (env, _) = super#on_Efun (env, default_ctx)
  method! on_Lfun (env, _) = super#on_Lfun (env, default_ctx)

  method! on_Try (env, ctx) b _cl fb =
    this#on_block (env, ctx) b;
    this#on_block (env, { ctx with is_finally = true }) fb;

  method! on_stmt (env, ctx) s =
    begin match s, ctx.control_context with
    | Break p, Toplevel -> Errors.toplevel_break p
    | Continue p, Toplevel -> Errors.toplevel_continue p
    | Continue p, SwitchContext -> Errors.continue_in_switch p
    | Return (p, _), _ when ctx.is_finally -> Errors.return_in_finally p
    | _ -> ()
    end;
    super#on_stmt (env, ctx) s

end

let handler = object
  inherit Nast_visitor.handler_base

  method! at_fun_ env f = visitor#on_fun_ (env, default_ctx) f

end

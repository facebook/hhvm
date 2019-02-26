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

type ctx = { imm_ctrl_ctx: control_context }

let default_ctx = { imm_ctrl_ctx = Toplevel }

let visitor = object(_this)
  inherit [ctx] Nast_visitor.iter_with_state as super

  method! on_Do (env, _) = super#on_Do (env, { imm_ctrl_ctx = LoopContext })
  method! on_While (env, _) = super#on_While (env, { imm_ctrl_ctx = LoopContext })
  method! on_For (env, _) = super#on_For (env, { imm_ctrl_ctx = LoopContext })
  method! on_Foreach (env, _) = super#on_Foreach (env, { imm_ctrl_ctx = LoopContext })
  method! on_Switch (env, _) = super#on_Switch (env, { imm_ctrl_ctx = SwitchContext })
  method! on_Efun (env, _) = super#on_Efun (env, { imm_ctrl_ctx = Toplevel })

  method! on_stmt (env, ctx) s =
    begin match s, ctx.imm_ctrl_ctx with
    | Break p, Toplevel -> Errors.toplevel_break p
    | Continue p, Toplevel -> Errors.toplevel_continue p
    | Continue p, SwitchContext -> Errors.continue_in_switch p
    | _ -> ()
    end;
    super#on_stmt (env, ctx) s

end

let handler = object
  inherit Nast_visitor.handler_base

  method! at_fun_ env f = visitor#on_fun_ (env, default_ctx) f

end

(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Nast

module SN = Naming_special_names

let handler = object
  inherit Nast_visitor.handler_base
  method! at_stmt env s =
    match s, env.Nast_visitor.control_context with
    | Break p, Nast_visitor.Toplevel -> Errors.toplevel_break p
    | Continue p, Nast_visitor.Toplevel -> Errors.toplevel_continue p
    | Continue p, Nast_visitor.SwitchContext -> Errors.continue_in_switch p
    | Return (p, _), _ when env.Nast_visitor.is_finally -> Errors.return_in_finally p
    | _ -> ()
end

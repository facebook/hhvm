(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Nast
open Nast_check_env

module SN = Naming_special_names

let handler = object
  inherit Nast_visitor.handler_base
  method! at_stmt env s =
    match s, env.control_context with
    | Break p, Toplevel -> Errors.toplevel_break p
    | Continue p, Toplevel -> Errors.toplevel_continue p
    | Continue p, SwitchContext -> Errors.continue_in_switch p
    | Return (p, _), _ when env.is_finally -> Errors.return_in_finally p
    | _ -> ()
end

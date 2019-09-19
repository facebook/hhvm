(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Nast_check_env

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_stmt env s =
      let p = fst s in
      match (snd s, env.control_context) with
      | (Break, Toplevel) -> Errors.toplevel_break p
      | (Continue, Toplevel) -> Errors.toplevel_continue p
      | (Continue, SwitchContext) -> Errors.continue_in_switch p
      | (Return _, _) when env.is_finally -> Errors.return_in_finally p
      | (GotoLabel (label_pos, _), _) when env.is_finally ->
        Errors.goto_label_defined_in_finally label_pos
      | (Goto (label_pos, _), _) when env.is_finally ->
        Errors.goto_invoked_in_finally label_pos
      | _ -> ()
  end

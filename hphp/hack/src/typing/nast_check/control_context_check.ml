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
      let err_opt =
        let open Nast_check_error in
        match (snd s, env.control_context) with
        | (Break, Toplevel) -> Some (Toplevel_break p)
        | (Continue, Toplevel) -> Some (Toplevel_continue p)
        | (Continue, SwitchContext) -> Some (Continue_in_switch p)
        | (Return _, _) when env.is_finally -> Some (Return_in_finally p)
        | _ -> None
      in
      Option.iter
        (fun err -> Errors.add_error Nast_check_error.(to_user_error err))
        err_opt
  end

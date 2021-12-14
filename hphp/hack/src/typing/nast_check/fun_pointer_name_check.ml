(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_expr env (_, _, expr) =
      match expr with
      | Fun_id (p, name) when String.contains name ':' ->
        Errors.add_naming_error @@ Naming_error.Illegal_meth_fun p
      | Fun_id (pos, name)
        when Option.is_none
               (Naming_provider.get_fun_pos env.Nast_check_env.ctx name) ->
        Errors.add_naming_error
        @@ Naming_error.Invalid_fun_pointer { pos; name }
      | _ -> ()
  end

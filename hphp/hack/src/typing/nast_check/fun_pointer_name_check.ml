(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Aast

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_expr env expr =
      match snd expr with
      | Fun_id (p, name) when String.contains name ':' ->
        Errors.illegal_meth_fun p
      | Fun_id (p, name)
        when Naming_provider.get_fun_pos env.Nast_check_env.ctx name = None ->
        Errors.invalid_fun_pointer p name
      | _ -> ()
  end

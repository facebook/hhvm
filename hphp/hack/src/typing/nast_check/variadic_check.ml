(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
module Partial = Partial_provider

let check_variadic v =
  match v with
  | FVvariadicArg vparam when vparam.param_is_reference ->
    Errors.variadic_byref_param vparam.param_pos
  | _ -> ()

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_ _ f = check_variadic f.f_variadic

    method! at_method_ _ m = check_variadic m.m_variadic
  end

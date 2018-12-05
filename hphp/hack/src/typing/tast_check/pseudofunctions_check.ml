(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Tast

module Env = Tast_env
module SN  = Naming_special_names

let handler = object
  inherit Tast_visitor.handler_base

  method! at_expr env ((p, _), x) =
    if Env.is_strict env then
    match x with
    | Id (_, pseudo_func) when pseudo_func = SN.PseudoFunctions.isset ->
      Errors.isset_in_strict p
    | Id (_, pseudo_func) when pseudo_func = SN.PseudoFunctions.empty ->
      Errors.empty_in_strict p
    | _ -> ()
end

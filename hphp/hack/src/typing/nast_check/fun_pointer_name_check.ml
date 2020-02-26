(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Aast

let error_if_not_in_naming_table p name =
  if Naming_heap.Funs.get_pos name = None then Errors.invalid_fun_pointer p name

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_expr _ expr =
      match snd expr with
      | Fun_id (p, name) -> error_if_not_in_naming_table p name
      (* Due to the body of a function being run through naming
       * At a separate time than the rest of the program
       * we need to duplicate the logic here
       *)
      | Call (_, (_, Id (_, cn)), _, [(p, String fn)], _)
        when cn = Naming_special_names.AutoimportedFunctions.fun_
             && String.contains fn ':' ->
        Errors.illegal_meth_fun p
      | Call (_, (_, Id (_, cn)), _, [(p, String fn)], _)
        when cn = Naming_special_names.AutoimportedFunctions.fun_ ->
        error_if_not_in_naming_table p fn
      | _ -> ()
  end

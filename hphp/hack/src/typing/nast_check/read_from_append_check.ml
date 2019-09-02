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

    method! at_expr env s =
      match snd s with
      | Array_get ((p, _), None) when not env.array_append_allowed ->
        Errors.reading_from_append p
      | _ -> ()
  end

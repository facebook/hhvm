(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

open Aast
module Utils = Tast_utils

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, p, Obj_get ((ty, _, _), _, OG_nullsafe, _))
        when Utils.type_non_nullable env ty ->
        Lints_errors.nullsafe_not_needed p
      | _ -> ()
  end

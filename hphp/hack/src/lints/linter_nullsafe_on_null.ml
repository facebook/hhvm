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
open Typing_defs

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, p, Obj_get ((ty, _, _), _, OG_nullsafe, _)) ->
        let (_, ty) = Tast_env.expand_type env ty in
        begin
          match get_node ty with
          | Tprim Tnull -> Lints_errors.nullsafe_on_null p
          | _ -> ()
        end
      | _ -> ()
  end

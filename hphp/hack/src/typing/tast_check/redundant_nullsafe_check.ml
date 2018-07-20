(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast_defs
open Tast
open Typing_defs

module Env = Tast_env
module Utils = Tast_utils

let handler = object
  inherit Tast_visitor.handler_base

  method! minimum_forward_compat_level = 2018_07_23

  method! at_expr env = function
    | (p, _), Obj_get (((_, ty), _), _, OG_nullsafe)
        when Utils.type_non_nullable env ty ->
      let _, (r, _) = Env.expand_type env ty in
      Errors.nullsafe_not_needed p
        (Reason.to_string "This is what makes me believe it cannot be null" r)
    | _ -> ()
end

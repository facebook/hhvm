(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Tast
open Typing_defs

let handler = object
  inherit Tast_visitor.handler_base

  method! at_expr _ = function
    | _, Obj_get (((_, (_, (Tdynamic|Tany))),_) , _, _) -> ()
    | _, Obj_get (_, ((p, _), Lvar _) , _) -> Errors.lvar_in_obj_get p
    | _ -> ()
  end

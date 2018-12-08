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

module Env = Tast_env

let handler = object
  inherit Tast_visitor.handler_base

  method! at_Call env _ expr _ _ _ =
    match snd expr with
    (* If rhs's expression is some Id, then we've got a normal function call
     * $x->foo
     * That's totally fine, ignore it. *)
    | Obj_get (_, (_, Id _), _) -> ()
    (* Otherwise, rhs is some dynamic invocation expression like: $x->$f *)
    (* if lhs is already dynamic, we let the dynamic invoke happen for now *)
    | Obj_get (((_, (_, Tdynamic)), _), _, _) -> ()
    (* if it's not dynamic, then we're in our error case. Grab the offending
     * expression's position and report it *)
    | Obj_get (_, ((rhs_pos, _), _), _)
      when Env.is_strict env ->
        Errors.dynamic_method_call rhs_pos
    | _ -> ()
  end

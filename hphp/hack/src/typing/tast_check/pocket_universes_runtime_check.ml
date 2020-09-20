(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Tast

let in_type_assertion = ref false

class ['self] check_pu_in_refinement =
  object (self : 'self)
    inherit Tast_visitor.handler_base as super

    method! at_hint env (pos, h) =
      match h with
      | Hpu_access _
      | Hprim (Aast_defs.Tatom _) ->
        if !in_type_assertion then Errors.pu_typing_refinement pos
      | _ -> super#at_hint env (pos, h)

    method! at_expr env e =
      match snd e with
      | Aast.Is (e, h)
      | Aast.As (e, h, _) ->
        super#at_expr env e;
        let prev = !in_type_assertion in
        in_type_assertion := true;
        self#at_hint env h;
        in_type_assertion := prev
      | _ -> super#at_expr env e
  end

let handler : Tast_visitor.handler_base = new check_pu_in_refinement

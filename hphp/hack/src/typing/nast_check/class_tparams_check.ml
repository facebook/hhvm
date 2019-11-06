(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

type ctx = { class_tparams: Nast.tparam list }

let visitor =
  object (_this)
    inherit [ctx] Nast_visitor.iter_with_state as super

    method! on_hint (env, state) (pos, h) =
      begin
        match h with
        | Habstr tp_name ->
          List.iter
            state.class_tparams
            (fun { tp_name = (c_tp_pos, c_tp_name); _ } ->
              if String.equal c_tp_name tp_name then
                Errors.typeconst_depends_on_external_tparam
                  pos
                  c_tp_pos
                  c_tp_name)
        | _ -> ()
      end;
      super#on_hint (env, state) (pos, h)
  end

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env c =
      let state = { class_tparams = c.c_tparams.c_tparam_list } in
      let on_hint = visitor#on_hint (env, state) in
      List.iter c.c_typeconsts (fun t ->
          Option.iter t.c_tconst_type on_hint;
          Option.iter t.c_tconst_constraint on_hint)
  end

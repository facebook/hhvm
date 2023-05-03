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
  object (this)
    inherit [ctx] Nast_visitor.iter_with_state as super

    method! on_hint (env, state) (pos, h) =
      begin
        match h with
        | Habstr (tp_name, args) ->
          List.iter
            state.class_tparams
            ~f:(fun { tp_name = (c_tp_pos, c_tp_name); _ } ->
              if String.equal c_tp_name tp_name then
                Errors.add_error
                  Nast_check_error.(
                    to_user_error
                    @@ Typeconst_depends_on_external_tparam
                         { pos; ext_pos = c_tp_pos; ext_name = c_tp_name }));
          List.iter args ~f:(this#on_hint (env, state))
        | _ -> ()
      end;
      super#on_hint (env, state) (pos, h)
  end

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env c =
      let state = { class_tparams = c.c_tparams } in
      let on_hint = visitor#on_hint (env, state) in
      List.iter c.c_typeconsts ~f:(fun t ->
          match t.c_tconst_kind with
          | TCAbstract
              {
                c_atc_as_constraint = a;
                c_atc_super_constraint = s;
                c_atc_default = d;
              } ->
            Option.iter a ~f:on_hint;
            Option.iter s ~f:on_hint;
            Option.iter d ~f:on_hint
          | TCConcrete { c_tc_type = t } -> on_hint t)
  end

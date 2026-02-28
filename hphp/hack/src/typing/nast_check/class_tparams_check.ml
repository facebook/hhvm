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

type ctx = { class_tparams: (Pos.t * string) list }

let visitor =
  object
    inherit [ctx] Nast_visitor.iter_with_state as super

    method! on_hint (env, state) (pos, h) =
      let state =
        match h with
        | Habstr tp_name ->
          List.iter state.class_tparams ~f:(fun (c_tp_pos, c_tp_name) ->
              if String.equal c_tp_name tp_name then
                Diagnostics.add_diagnostic
                  Nast_check_error.(
                    to_user_diagnostic
                    @@ Typeconst_depends_on_external_tparam
                         { pos; ext_pos = c_tp_pos; ext_name = c_tp_name }));
          state
        | Hfun { hf_tparams; _ } ->
          {
            class_tparams =
              state.class_tparams
              @ List.map hf_tparams ~f:(fun { htp_name; _ } -> htp_name);
          }
        | _ -> state
      in
      super#on_hint (env, state) (pos, h)
  end

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env c =
      let state =
        {
          class_tparams =
            List.map c.c_tparams ~f:(fun { tp_name; _ } -> tp_name);
        }
      in
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

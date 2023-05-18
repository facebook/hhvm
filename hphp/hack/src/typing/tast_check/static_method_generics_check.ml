(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

let static_method_check env reified_params m =
  let visitor =
    object (this)
      inherit [_] Aast.iter as super

      method! on_hint env (pos, h) =
        match h with
        | Aast.Habstr (t, args) ->
          if SSet.mem t reified_params then
            Typing_error_utils.add_typing_error
              ~env:(Tast_env.tast_env_as_typing_env env)
              Typing_error.(
                primary
                @@ Primary.Static_meth_with_class_reified_generic
                     { pos = m.m_span; generic_pos = pos });
          List.iter args ~f:(this#on_hint env)
        | _ -> super#on_hint env (pos, h)
    end
  in
  List.iter m.m_params ~f:(visitor#on_fun_param env);
  visitor#on_type_hint env m.m_ret

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ env c =
      let (_, static_methods, _) = split_methods c.c_methods in
      if
        not
          (Pos.filename c.c_span |> Relative_path.prefix |> Relative_path.is_hhi)
      then
        let reified_params =
          List.filter_map c.c_tparams ~f:(function tp ->
              if not (equal_reify_kind tp.tp_reified Erased) then
                Some (snd tp.tp_name)
              else
                None)
          |> SSet.of_list
        in
        List.iter static_methods ~f:(static_method_check env reified_params)
  end

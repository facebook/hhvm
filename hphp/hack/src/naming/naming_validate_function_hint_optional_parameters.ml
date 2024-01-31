(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let on_hint on_error hint ~ctx =
  let err_list =
    match hint with
    | (pos, Aast.(Hfun { hf_param_info; _ })) ->
      let (_, optional_precedes_non_optional) =
        List.fold
          hf_param_info
          ~init:(false, false)
          ~f:(fun (seen_optional, optional_precedes_non_optional) pi ->
            match pi with
            | Some { Aast.hfparam_optional = Some Ast_defs.Optional; _ } ->
              (true, optional_precedes_non_optional)
            | _ ->
              (seen_optional, optional_precedes_non_optional || seen_optional))
      in
      let optional_inouts =
        List.filter_map hf_param_info ~f:(fun pi ->
            match pi with
            | Some
                {
                  Aast.hfparam_optional = Some Ast_defs.Optional;
                  Aast.hfparam_kind = Ast_defs.Pinout pos;
                  _;
                } ->
              Some
                (Naming_phase_error.parsing
                   Parsing_error.(
                     Parsing_error
                       {
                         pos;
                         msg = "Optional parameter cannot be inout";
                         quickfixes = [];
                       }))
            | _ -> None)
      in
      if optional_precedes_non_optional then
        Naming_phase_error.parsing
          Parsing_error.(
            Parsing_error
              {
                pos;
                msg = "Optional parameter cannot precede non-optional parameter";
                quickfixes = [];
              })
        :: optional_inouts
      else
        optional_inouts
    | _ -> []
  in
  List.iter ~f:on_error err_list;
  (ctx, Ok hint)

let pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.{ id with on_ty_hint = Some (on_hint on_error) }

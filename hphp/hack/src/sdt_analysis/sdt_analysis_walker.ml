(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Sdt_analysis_types
module A = Aast
module T = Typing_defs
module SN = Naming_special_names

let collect_sdts =
  object
    inherit [_] Tast_visitor.reduce as super

    method zero = []

    method plus = ( @ )

    method! on_expr env e =
      let ids =
        match e with
        | (_, hack_pos, A.Call ((base_ty, _, base_exp), _tel, el, _unpacked_el))
          ->
          let doesnt_subtype (fp, (_, (arg_ty, _, _))) =
            not @@ Tast_env.is_sub_type env arg_ty fp.T.fp_type.T.et_type
          in
          let go id ty =
            match T.get_node ty with
            | T.Tfun ft ->
              let param_arg_pairs =
                let open List.Or_unequal_lengths in
                match List.zip ft.T.ft_params el with
                | Ok pairs -> pairs
                | Unequal_lengths -> []
              in
              if List.exists ~f:doesnt_subtype param_arg_pairs then
                let constraint_ =
                  DecoratedConstraint.
                    {
                      origin = __LINE__;
                      hack_pos;
                      constraint_ = Constraint.SDT;
                    }
                in
                [(id, DecoratedConstraint.Set.singleton constraint_)]
              else
                []
            | _ -> []
          in
          begin
            match base_exp with
            | A.Id (_, id) -> begin
              match T.get_node base_ty with
              | T.Tnewtype (c, [ty], _)
                when String.equal c SN.Classes.cSupportDyn ->
                go id ty
              | _ -> go id base_ty
            end
            | _ -> []
          end
        | _ -> []
      in
      ids @ super#on_expr env e

    method! on_fun_def env (A.{ fd_name = (_, id); fd_fun; _ } as fd) =
      let is_dynamically_callable =
        List.exists ~f:(fun attr ->
            String.equal
              (snd attr.A.ua_name)
              SN.UserAttributes.uaDynamicallyCallable)
      in
      let ids =
        if is_dynamically_callable fd_fun.A.f_user_attributes then
          let constraint_ =
            DecoratedConstraint.
              {
                origin = __LINE__;
                hack_pos = fd_fun.A.f_span;
                constraint_ = Constraint.SDT;
              }
          in
          [(id, DecoratedConstraint.Set.singleton constraint_)]
        else
          []
      in
      ids @ super#on_fun_def env fd
  end

let program () (ctx : Provider_context.t) (tast : Tast.program) =
  let def def =
    let tast_env = Tast_env.def_env ctx def in
    collect_sdts#on_def tast_env def
  in
  List.concat_map ~f:def tast |> SMap.of_list

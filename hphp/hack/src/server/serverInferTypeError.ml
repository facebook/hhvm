(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let visitor line char =
  object
    inherit [_] Tast_visitor.reduce

    inherit [Pos.t * _ * _ * _] Ast_defs.option_monoid

    method private merge ((lpos, _, _, _) as lhs) ((rpos, _m, _, _) as rhs) =
      if Pos.length lpos <= Pos.length rpos then
        lhs
      else
        rhs

    method! on_Hole env expr from_ty to_ty hole_source =
      match (expr, hole_source) with
      | (((pos, _), _), Aast.Typing) when Pos.inside pos line char ->
        Some (pos, env, from_ty, to_ty)
      | _ -> None
  end

let type_error_at_pos
    (ctx : Provider_context.t) (tast : Tast.program) (line : int) (char : int) :
    (Tast_env.env * Tast.ty * Tast.ty) option =
  Option.(
    (visitor line char)#go ctx tast >>| fun (_, env, actual_ty, expected_ty) ->
    (env, actual_ty, expected_ty))

let go_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : InferErrorAtPosService.result =
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  Option.(
    type_error_at_pos ctx tast line column >>| fun (env, from_ty, to_ty) ->
    Tast_env.(
      InferErrorAtPosService.
        {
          actual_ty_string = print_ty env from_ty;
          actual_ty_json = Hh_json.json_to_string @@ ty_to_json env from_ty;
          expected_ty_string = print_ty env to_ty;
          expected_ty_json = Hh_json.json_to_string @@ ty_to_json env to_ty;
        }))

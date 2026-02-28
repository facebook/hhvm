(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let starts_at pos line char_start =
  let (l, c, _) = Pos.info_pos pos in
  line = l && char_start = c

let visitor line char =
  object
    inherit [_] Tast_visitor.reduce as super

    method private zero = None

    (* A node with position P is not always a parent of every other node with
       * a position contained by P. Some desugaring can cause nodes to be
       * rearranged so that this is no longer the case (e.g., `invariant`).
       *
       * Since we are finding `Hole`s based on their exact starting position,
       * we _shouldn't_ encounter the case of two simultaneous `Hole`s with
       * different parents but the logic to handle this is retained - we simply
       * take the smaller node. *)
    method private plus lhs rhs =
      Option.merge
        lhs
        rhs
        ~f:(fun ((lpos, _, _, _) as lhs) ((rpos, _, _, _) as rhs) ->
          if Pos.length lpos <= Pos.length rpos then
            lhs
          else
            rhs)

    (* Find the the `Hole` which exactly starts at the provided position;
       if the current hole does not, call method on supertype to continue
       recursion *)
    method! on_Hole env expr from_ty to_ty hole_source =
      match (expr, hole_source) with
      | ((_, pos, _), Aast.Typing) when starts_at pos line char ->
        Some (pos, env, from_ty, to_ty)
      | _ -> super#on_Hole env expr from_ty to_ty hole_source
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
    type_error_at_pos
      ctx
      tast.Tast_with_dynamic.under_normal_assumptions
      line
      column
    >>| fun (env, from_ty, to_ty) ->
    Tast_env.(
      InferErrorAtPosService.
        {
          actual_ty_string = print_ty env from_ty;
          actual_ty_json = Hh_json.json_to_string @@ ty_to_json env from_ty;
          expected_ty_string = print_ty env to_ty;
          expected_ty_json = Hh_json.json_to_string @@ ty_to_json env to_ty;
        }))

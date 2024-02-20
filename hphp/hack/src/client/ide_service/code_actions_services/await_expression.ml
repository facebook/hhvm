(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type candidate = { expr_pos: Pos.t }

let find_candidate ctx tast selection : candidate option =
  let visitor =
    let expr_positions_overlapping_selection = ref [] in
    (* We don't want to provide the refactoring in cases like this:
       /*range-start*/
       $x = 1;
       $y = gen_foo();
       /*range-end*/
    *)
    let ensure_valid_selection =
      Option.filter ~f:(fun candidate ->
          List.for_all
            !expr_positions_overlapping_selection
            ~f:(Pos.contains candidate.expr_pos))
    in
    let in_await = ref false in
    object
      inherit [candidate option] Tast_visitor.reduce as super

      method zero = None

      method plus = Option.first_some

      method! on_def env def =
        let def_pos =
          match def with
          | Aast.Fun fun_def -> Aast.(fun_def.fd_fun.f_span)
          | Aast.Class class_def -> Aast.(class_def.c_span)
          | Aast.Stmt (pos, _) -> pos
          | _ -> Pos.none
        in
        (* Avoid expensive unnecessary traversals *)
        if Pos.contains def_pos selection then
          ensure_valid_selection @@ super#on_def env def
        else
          None

      method! on_fun_ env fun_ =
        in_await := false;
        super#on_fun_ env fun_

      method! on_Await env await =
        in_await := true;
        super#on_Await env await

      method! on_expr env expr =
        let (ty, expr_pos, _) = expr in
        if Pos.contains selection expr_pos then
          expr_positions_overlapping_selection :=
            expr_pos :: !expr_positions_overlapping_selection;
        match super#on_expr env expr with
        | None when Pos.contains selection expr_pos ->
          let needs_await =
            match Typing_defs_core.get_node ty with
            | Typing_defs_core.Tclass ((_, c_name), _, _) ->
              (not !in_await)
              && String.equal c_name Naming_special_names.Classes.cAwaitable
            | _ -> false
          in
          Option.some_if needs_await { expr_pos }
        | candidate_opt -> candidate_opt
    end
  in
  visitor#go ctx tast

let pos_of_offset path source_text offset : Pos.t =
  let (line, col) =
    Full_fidelity_source_text.offset_to_position source_text offset
  in
  let bol = offset - col + 1 in
  let triple = (line, bol, offset) in
  Pos.make_from_lnum_bol_offset ~pos_file:path ~pos_start:triple ~pos_end:triple

let find_modifier_edits path source_text positioned_tree pos :
    Code_action_types.edit list option =
  let module Syn = Full_fidelity_positioned_syntax in
  let parents =
    let root = Provider_context.PositionedSyntaxTree.root positioned_tree in
    let offset =
      let (line, start, _) = Pos.info_pos pos in
      Full_fidelity_source_text.position_to_offset source_text (line, start)
    in
    Syn.parentage root offset
  in
  parents
  |> List.map ~f:Syn.syntax
  |> List.find_map ~f:(function
         | Syn.LambdaExpression
             { lambda_async = Syn.{ syntax = Missing; _ } as node; _ } -> begin
           Syn.offset node
           |> Option.map ~f:(fun offset ->
                  [
                    Code_action_types.
                      {
                        pos = pos_of_offset path source_text offset;
                        text = "async ";
                      };
                  ])
         end
         | Syn.LambdaExpression _ -> Some []
         | _ -> None)

let edits_of_candidate ctx entry { expr_pos } : Code_action_types.edit list =
  let path = entry.Provider_context.path in
  let source_text = Ast_provider.compute_source_text ~entry in
  let positioned_tree = Ast_provider.compute_cst ~ctx ~entry in

  find_modifier_edits path source_text positioned_tree expr_pos
  |> Option.value ~default:[]
  |> ( @ )
       [
         Code_action_types.
           { pos = Pos.shrink_to_start expr_pos; text = "await " };
       ]

let refactor_of_candidate ctx entry candidate =
  let edits =
    lazy
      (Relative_path.Map.singleton
         entry.Provider_context.path
         (edits_of_candidate ctx entry candidate))
  in
  Code_action_types.Refactor.{ title = "await expression"; edits }

let find ~entry selection ctx =
  if Pos.length selection <> 0 then
    let { Tast_provider.Compute_tast.tast; _ } =
      Tast_provider.compute_tast_quarantined ~ctx ~entry
    in
    find_candidate ctx tast.Tast_with_dynamic.under_normal_assumptions selection
    |> Option.map ~f:(refactor_of_candidate ctx entry)
    |> Option.to_list
  else
    []

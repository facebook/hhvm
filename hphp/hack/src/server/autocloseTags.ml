(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module FFP = Full_fidelity_positioned_syntax

module AutocloseTag = struct
  type result = {
    xhp_open: FFP.t;
    xhp_open_right_angle: FFP.t;
    xhp_close: FFP.t;
    insert_text: string;
  }
end

(** Get xhp positions from the FFP for every xhp that is an
    xhp open tag. **)
let all_tags (tree : FFP.t) : AutocloseTag.result list =
  let open Full_fidelity_positioned_syntax in
  (* Walk FFP syntax node [s], tracking the current context [ctx]
     and accumulate xhp tags. *)
  let rec aux ctx (acc : AutocloseTag.result list) (s : FFP.t) :
      AutocloseTag.result list =
    match s.syntax with
    | XHPExpression { xhp_open; xhp_close; xhp_body = _ } ->
      let acc =
        match xhp_open.syntax with
        | XHPOpen
            {
              xhp_open_left_angle = _;
              xhp_open_name;
              xhp_open_attributes = _;
              xhp_open_right_angle;
            } ->
          let insert_text =
            Format.sprintf
              "</%s>"
              (Full_fidelity_positioned_syntax.text xhp_open_name)
          in
          AutocloseTag.
            { xhp_open; xhp_open_right_angle; xhp_close; insert_text }
          :: acc
        | _ -> acc
      in
      List.fold (children s) ~init:acc ~f:(aux ctx)
    | _ -> List.fold (children s) ~init:acc ~f:(aux ctx)
  in

  aux None [] tree

let go_xhp_tags
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : AutocloseTag.result option =
  let open Full_fidelity_positioned_syntax in
  let source_text = Ast_provider.compute_source_text ~entry in
  let target_offset =
    Full_fidelity_source_text.position_to_offset source_text (line, column)
  in
  let cst = Ast_provider.compute_cst ~ctx ~entry in
  let tree = Provider_context.PositionedSyntaxTree.root cst in
  let all_tags = all_tags tree in
  let xhp_tags =
    all_tags
    |> List.filter
         ~f:(fun AutocloseTag.{ xhp_open; xhp_open_right_angle; xhp_close; _ }
            ->
           (* The target offset should be the position after the end of the
              open right angle *)
           let open Int in
           target_offset = end_offset xhp_open_right_angle + 1
           && end_offset xhp_open > start_offset xhp_open
           && end_offset xhp_open < end_offset xhp_close)
  in
  match xhp_tags with
  | [result] -> Some result
  | [] -> None
  | _ ->
    HackEventLogger.invariant_violation_bug
      "Should only find one close tag, found more than 1";
    None

let go_xhp_close_tag
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : string option =
  let tags = go_xhp_tags ~ctx ~entry ~line ~column in
  match tags with
  | Some
      AutocloseTag.
        { xhp_open = _; xhp_open_right_angle = _; xhp_close = _; insert_text }
    ->
    Some insert_text
  | None -> None

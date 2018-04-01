(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module PS = Full_fidelity_positioned_syntax
module PositionedTree = Full_fidelity_syntax_tree
  .WithSyntax(PS)

module TS = Full_fidelity_typed_positioned_syntax
module TypedTree = Full_fidelity_syntax_tree
  .WithSyntax(TS)

(*
  Ideally this would be just Pos.AbsolutePosMap.get, however the positions
  in the Tast are off by 1 from positions in the full fidelity parse trees.

  TODO: Fix this when the full fidelity parse tree becomes the parser for type checking.
*)
let get_from_pos_map (position: Pos.absolute) (map: Hh_json.json list Pos.AbsolutePosMap.t) =
  let rec aux es =
    match es with
    | [] -> []
    | (pos, tys) :: tl ->
        if ((Pos.start_cnum pos) == (Pos.start_cnum position)
            && (Pos.end_cnum pos) == (1 + (Pos.end_cnum position))) then
          tys
        else
          aux tl
  in
  let elements = Pos.AbsolutePosMap.elements map in
  aux elements

let typed_from_positioned
    (file: Relative_path.t)
    (map: Hh_json.json list Pos.AbsolutePosMap.t)
    (tree: PositionedTree.t): TypedTree.t =
  let rec aux (positioned_node: PS.t): TS.t =
    let value = PS.value positioned_node in
    let position = Pos.to_absolute (Option.value_exn (PS.position file positioned_node)) in
    let types = get_from_pos_map position map in
    match PS.syntax positioned_node with
    | PS.Token token ->
      let syntax = TS.Token token in
      TS.make syntax (TS.positioned_value_to_typed position types value)
    | _ ->
      let kind = PS.kind positioned_node in
      let positioneds = PS.children positioned_node in
      let typeds = List.map aux positioneds in
      let syntax = TS.syntax_from_children kind typeds in
      TS.make syntax (TS.positioned_value_to_typed position types value)
  in
  let root = aux (PositionedTree.root tree) in
  TypedTree.build
    (PositionedTree.text tree)
    root
    (PositionedTree.all_errors tree)
    (PositionedTree.language tree)
    (PositionedTree.mode tree)

let go:
  ServerEnv.env ->
  string ->
  string =
fun env filename ->
  let ServerEnv.{tcopt; files_info; _} = env in

  (* get the typed ast *)
  let _, tast = ServerIdeUtils.check_file_input tcopt files_info (ServerUtils.FileName filename) in
  let type_map = Tast_type_collector.collect_types tast in

  (* get the parse tree *)
  let file = Relative_path.create Relative_path.Dummy filename in
  let source_text = Full_fidelity_source_text.from_file file in
  let positioned_tree = PositionedTree.make source_text in
  let typed = typed_from_positioned file type_map positioned_tree in

  let json = TypedTree.to_json ~with_value:true typed in
  Hh_json.json_to_string json

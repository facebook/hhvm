(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module PS = Full_fidelity_positioned_syntax
module PositionedTree = Full_fidelity_syntax_tree
  .WithSyntax(PS)

module TS = Full_fidelity_typed_positioned_syntax
module TypedTree = Full_fidelity_syntax_tree
  .WithSyntax(TS)

let typed_from_positioned
    (file: Relative_path.t)
    (map: Tast_type_collector.collected_type list Pos.AbsolutePosMap.t)
    (tree: PositionedTree.t): TypedTree.t =
  let rec aux (positioned_node: PS.t): TS.t =
    let value = PS.value positioned_node in
    let position = Pos.to_absolute (Option.value_exn (PS.position file positioned_node)) in
    let types = Tast_type_collector.get_from_pos_map position map
      |> Option.value ~default:[] in
    match PS.syntax positioned_node with
    | PS.Token token ->
      let syntax = TS.Token token in
      TS.make syntax (TS.positioned_value_to_typed position types value)
    | _ ->
      let kind = PS.kind positioned_node in
      let positioneds = PS.children positioned_node in
      let typeds = List.map ~f:aux positioneds in
      let syntax = TS.syntax_from_children kind typeds in
      TS.make syntax (TS.positioned_value_to_typed position types value)
  in
  let root = aux (PositionedTree.root tree) in
  TypedTree.build
    (PositionedTree.text tree)
    root
    (PositionedTree.all_errors tree)
    (PositionedTree.mode tree)

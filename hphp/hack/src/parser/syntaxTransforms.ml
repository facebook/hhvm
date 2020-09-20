(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module ES = Full_fidelity_editable_syntax
module PS = Full_fidelity_positioned_syntax
module ET = ES.Token
module PT = PS.Token
module PositionedSyntaxTree = Full_fidelity_syntax_tree.WithSyntax (PS)

let editable_from_positioned tree =
  let rec aux text positioned_node offset =
    match PS.syntax positioned_node with
    | PS.Token token ->
      let source_text = PT.source_text token in
      let width = PT.width token in
      let leading =
        Full_fidelity_editable_trivia.from_positioned_list
          source_text
          (PT.leading token)
          offset
      in
      let trailing =
        Full_fidelity_editable_trivia.from_positioned_list
          source_text
          (PT.trailing token)
          (offset + PT.leading_width token + width)
      in
      let editable_token =
        ET.make
          (PT.kind token)
          source_text
          (PT.leading_start_offset token)
          width
          leading
          trailing
      in
      let syntax = ES.Token editable_token in
      ES.make syntax ES.Value.NoValue
    | _ ->
      let folder (acc, offset) child =
        let new_child = aux text child offset in
        let w = PS.full_width child in
        (new_child :: acc, offset + w)
      in
      let kind = PS.kind positioned_node in
      let positioneds = PS.children positioned_node in
      let (editables, _) = List.fold_left folder ([], offset) positioneds in
      let editables = List.rev editables in
      let syntax = ES.syntax_from_children kind editables in
      ES.make syntax ES.Value.NoValue
  in
  aux (PositionedSyntaxTree.text tree) (PositionedSyntaxTree.root tree) 0

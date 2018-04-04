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

let create_typed_parse_tree_json_string
  (filename: Relative_path.t) (positioned_tree: PositionedTree.t) (tast: Tast.program) : string =
  let type_map = Tast_type_collector.collect_types tast in
  let typed = TypedSyntaxTransforms.typed_from_positioned filename type_map positioned_tree in
  let json = TypedTree.to_json ~with_value:true typed in
  Hh_json.json_to_string json

let go:
  ServerEnv.env ->
  string ->
  string =
fun env filename ->
  let ServerEnv.{tcopt; files_info; _} = env in

  (* get the typed ast *)
  let _, tast = ServerIdeUtils.check_file_input tcopt files_info (ServerUtils.FileName filename) in

  (* get the parse tree *)
  let file = Relative_path.create Relative_path.Dummy filename in
  let source_text = Full_fidelity_source_text.from_file file in
  let positioned_tree = PositionedTree.make source_text in

  create_typed_parse_tree_json_string file positioned_tree tast

(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module PS = Full_fidelity_positioned_syntax
module PositionedTree = Full_fidelity_syntax_tree
  .WithSyntax(PS)

module TS = Full_fidelity_typed_positioned_syntax
module TypedTree = Full_fidelity_syntax_tree
  .WithSyntax(TS)

let create_typed_parse_tree
    ~(filename: Relative_path.t)
    ~(positioned_tree: PositionedTree.t)
    ~(tast: Tast.program)
    : TypedTree.t
  =
  let type_map = Tast_type_collector.collect_types tast in
  TypedSyntaxTransforms.typed_from_positioned filename type_map positioned_tree

let typed_parse_tree_to_json (typed_tree: TypedTree.t): Hh_json.json =
  TypedTree.to_json ~with_value:true typed_tree

let go:
  ServerEnv.env ->
  string ->
  string =
fun env filename ->
  let ServerEnv.{tcopt; naming_table; _} = env in

  (* get the typed ast *)
  let _, tast = ServerIdeUtils.check_file_input tcopt naming_table
    (ServerCommandTypes.FileName filename) in

  (* get the parse tree *)
  let filename = Relative_path.create Relative_path.Dummy filename in
  let source_text = Full_fidelity_source_text.from_file filename in
  let positioned_tree = PositionedTree.make source_text in

  create_typed_parse_tree ~filename ~positioned_tree ~tast
    |> typed_parse_tree_to_json
    |> Hh_json.json_to_string

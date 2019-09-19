(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_minimal_syntax)

(* Entry Point *)
let go filename =
  let file = Relative_path.create Relative_path.Dummy filename in
  let source_text = Full_fidelity_source_text.from_file file in
  let syntax_tree = SyntaxTree.make source_text in
  let json = SyntaxTree.to_json syntax_tree in
  Hh_json.json_to_string json

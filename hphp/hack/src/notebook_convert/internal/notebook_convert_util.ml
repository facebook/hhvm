(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Tree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)

let tree_of_code (hack : string) : Tree.t =
  let env = Full_fidelity_parser_env.make () in
  let source_text = Full_fidelity_source_text.make Relative_path.default hack in
  Tree.make ~env source_text

let parse (hack : string) : Full_fidelity_positioned_syntax.t =
  Tree.root @@ tree_of_code hack

let hackfmt (hack : string) : string =
  let module Tree =
    Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax) in
  let tree = tree_of_code hack in
  if List.is_empty (Tree.all_errors tree) then
    Libhackfmt.format_tree tree
  else
    hack

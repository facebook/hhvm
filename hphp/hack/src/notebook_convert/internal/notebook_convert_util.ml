(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let parse hack : Full_fidelity_positioned_syntax.t =
  let env = Full_fidelity_parser_env.make () in
  let source_text = Full_fidelity_source_text.make Relative_path.default hack in
  let module Tree =
    Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax) in
  let positioned_syntax_tree = Tree.make ~env source_text in
  Tree.root positioned_syntax_tree

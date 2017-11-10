(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
module MinimalTree = Full_fidelity_syntax_tree
  .WithSyntax(Full_fidelity_minimal_syntax)
module PositionedTree = Full_fidelity_syntax_tree
  .WithSyntax(Full_fidelity_positioned_syntax)

let positioned_from_minimal minimal_tree =
  let text = MinimalTree.text minimal_tree in
  let root = MinimalTree.root minimal_tree in
  let root = Full_fidelity_positioned_syntax.from_minimal text root in
  let errors = MinimalTree.all_errors minimal_tree in
  PositionedTree.from_root text root errors

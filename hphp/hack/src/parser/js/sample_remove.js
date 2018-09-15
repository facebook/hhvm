/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 */

"use strict";
let parser = require('./full_fidelity_parser.js');
let editable = require("./full_fidelity_editable.js");

function main(json)
{
  // In an "editable" tree every node knows its text, but not its
  // position within the tree. This means that we can reorganize /
  // replace nodes in the tree without having to recompute positions
  // of every node in the tree.

  // Editable trees are immutable. We do not mutate
  // an existing tree; we run a non-detructive visitor over it which
  // produces a new tree. The new tree shares as many nodes as possible
  // with the old tree, so this is pretty memory-efficient.

  let original = editable.from_json(json);
  let without_tries = original.remove_where(
    (node) => node.syntax_kind === 'try_statement');
  let methods = without_tries.filter(
    (node) => node.syntax_kind === 'methodish_declaration');

  let without_first = without_tries.without(methods[0]);

  console.log("---original---");
  console.log(original.full_text);

  console.log("---without tries---");
  console.log(without_tries.full_text);

  console.log("---without tries or first method ---");
  console.log(without_first.full_text);
}

function on_error(error)
{
  console.log("---error---");
  console.log(error);
}

parser.parse_file_to_json(
  'sample_remove.php',
  main, on_error);

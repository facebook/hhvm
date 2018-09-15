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


// Turn any comment associated with a try statement into blah blah blah.
// Note that this will detect both comments that appear before the try
// token and comments that appear anywhere inside the try / catch / finally,
// and comments which immediately follow the trailing } of the statement.
function rewriter(node, parents)
{
  if (node.syntax_kind === "single_line_comment")
    return node.with_text("// blah blah blah");
  else if (node.syntax_kind === "delimited_comment")
    return node.with_text("/* blah blah blah */")
  return node;
};

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
  console.log("---original---");
  console.log(original.full_text);
  let rewritten = original.rewrite(rewriter);

  console.log("---rewritten---");
  console.log(rewritten.full_text);
}

function on_error(error)
{
  console.log("---error---");
  console.log(error);
}

parser.parse_file_to_json('sample_rewrite_comments.php',
  main, on_error);

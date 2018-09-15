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
  let tries = original.of_syntax_kind('try_statement');
  let first_try_body = tries[0].compound_statement;
  let comment = new editable.DelimitedComment("/* HELLO WORLD */");
  let rewritten_try = original.insert_before(comment, first_try_body);
  let catches = rewritten_try.of_syntax_kind('catch_clause');
  let left_brace = catches[0].body.left_brace;
  let rewritten = rewritten_try.insert_after(comment, left_brace);

  console.log("---original---");
  console.log(original.full_text);

  console.log("---rewritten---");
  console.log(rewritten.full_text);
}

function on_error(error)
{
  console.log("---error---");
  console.log(error);
}

parser.parse_file_to_json(
  'sample_insert.php',
  main, on_error);

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

function count(node, predicate)
{
  function reducer(node, accumulator)
  {
    return accumulator + (predicate(node) ? 1 : 0);
  }
  return node.reduce(reducer, 0);
}

function main(json)
{
  function predicate(node) {
    return node.syntax_kind === "token" && node.token_kind === "variable"
      && node.text.length <= 2;
  }
  let root = editable.from_json(json);
  console.log("A two-character variable appears " +
    count(root, predicate) + " times.");
}

function on_error(error)
{
  console.log("---error---");
  console.log(error);
}

parser.parse_file_to_json(
  'sample_reduce.php',
  main, on_error);

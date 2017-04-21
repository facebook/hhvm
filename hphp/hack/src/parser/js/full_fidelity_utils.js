/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 */

"use strict";

let child_process = require('child_process');
let spawn = child_process.spawn;

// array is A[]
// mapper is (A, C) => B
// reducer is (A, C) => C
// accumulator is C
function array_map_reduce(array, mapper, reducer, accumulator)
{
  let acc = accumulator;
  let result = [];
  for(let item of array) {
    result.push(mapper(item, acc));
    acc = reducer(item, acc);
  }
  return result;
}

function array_sum(array)
{
  return array.reduce((a, b) => a + b, 0);
}

function all_output(program, args, on_complete)
{
  let stdout_text = "";
  let stderr_text = "";
  let process = spawn(program, args);
  let stdout = process.stdout;
  let stderr = process.stderr;
  stdout.setEncoding('utf8');
  stderr.setEncoding('utf8');
  stdout.on('data', (data) =>
  {
    stdout_text += data;
  });
  stderr.on('data', (data) =>
  {
    stderr_text += data;
  });
  process.on('close', (code) =>
  {
    on_complete(code, stdout_text, stderr_text);
  });
}

exports.array_map_reduce = array_map_reduce;
exports.array_sum = array_sum;
exports.all_output = all_output;

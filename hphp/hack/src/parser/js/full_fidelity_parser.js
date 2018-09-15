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

let utils = require('./full_fidelity_utils.js');
let all_output = utils.all_output;

function parse_file_to_json(file, on_parsed, on_error)
{
  // TODO: Can we provide an API for parsing text, not just a file?
  let program = 'hh_parse';
  let args = ['--full-fidelity-json', file];
  let on_complete = (code, stdout, stderr) =>
  {
    if (stderr !== "")
    {
      on_error(stderr);
      return;
    }
    let json = JSON.parse(stdout);
    on_parsed(json);
  };
  all_output(program, args, on_complete);
}

exports.parse_file_to_json = parse_file_to_json;

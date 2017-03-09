<?hh // strict
/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the 'hack' directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 */

require_once 'full_fidelity_utilities.php';
require_once 'full_fidelity_editable.php';

function parse_file_to_json(string $file) : mixed {
  $results = execute("hh_parse --full-fidelity-json $file");
  $json = json_decode($results[0]);
  return $json;
}

function parse_file_to_editable(string $file) : EditableSyntax {
  $json = parse_file_to_json($file);
  return from_json($json);
}

function parse_text_to_json(string $text) : mixed {
  $file = tempnam("/tmp", "");
  $handle = fopen($file, "w");
  fwrite($handle, $text);
  fclose($handle);
  $json = parse_file_to_json($file);
  unlink($file);
  return $json;
}

function parse_text_to_editable(string $text) : EditableSyntax {
  $json = parse_text_to_json($text);
  return from_json($json);
}

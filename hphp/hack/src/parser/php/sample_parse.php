#!/bin/env php
<?hh

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

$_SERVER['PHP_ROOT'] = realpath(__DIR__.'/../../../../../../../www-hg');
require_once $_SERVER['PHP_ROOT'].'/flib/__flib.php';
flib_init(FlibContext::SCRIPT);
require_once 'full_fidelity_parser.php';

async function my_script_main(): Awaitable<void> {
  $file = 'sample_parse_input.php';
  $root_file = parse_file_to_editable($file);
  $declaration_file = $root_file->declarations()[0]->full_text();

  $text = "<?hh\nfunction foo() {}\nfunction bar() {}";
  $root_text = parse_text_to_editable($text);
  $declaration_text = $root_text->declarations()[1]->full_text();

  print "\n---file---\n{$declaration_file}\n";
  print "\n---text---\n{$declaration_text}\n";
}
Asio::enterAsyncEntryPoint(() ==> my_script_main());

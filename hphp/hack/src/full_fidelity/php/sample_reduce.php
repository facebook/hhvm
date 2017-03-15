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

/*
* A reducer traverses a parse tree and produces a summary value. In this case
* we build a simple counter by traversing the tree, applying a predicate to
* every node, and incrementing a counter if the predicate is true.
*/

function count_filter(
  EditableSyntax $node,
  (function(EditableSyntax):bool) $predicate,
){
  return $node->reduce(($n, $a) ==> $a + ($predicate($n) ? 1 : 0), 0);
}

async function my_script_main(): Awaitable<void> {
  $file = 'sample_reduce_input.php';
  $root = parse_file_to_editable($file);
  $predicate = ($node) ==> {
    return $node->syntax_kind() === 'token'
      && $node->token_kind() === 'variable'
      && strlen($node->text()) <= 2;
  };
  $c = count_filter($root, $predicate);
  print "A two-character variable appears {$c} times.\n";
}
Asio::enterAsyncEntryPoint(() ==> my_script_main());

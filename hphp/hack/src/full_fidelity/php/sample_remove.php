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

  // In an "editable" tree every node knows its text, but not its
  // position within the tree. This means that we can reorganize /
  // replace nodes in the tree without having to recompute positions
  // of every node in the tree.

  // Editable trees are immutable. We do not mutate
  // an existing tree; we run a non-detructive visitor over it which
  // produces a new tree. The new tree shares as many nodes as possible
  // with the old tree, so this is pretty memory-efficient.

  $file = 'sample_remove_input.php';
  $original = parse_file_to_editable($file);

  $without_tries = $original->remove_where(
    ($node) ==> $node->syntax_kind() === 'try_statement'
  );

  $methods = $without_tries->filter(
    ($node) ==> $node->syntax_kind() === 'methodish_declaration'
  );

  $without_first = $without_tries->without(firstx($methods));
  $original_text = $original->full_text();
  $without_tries_text = $without_tries->full_text();
  $without_first_text = $without_first->full_text();

  print "\n---original---\n{$original_text}\n";
  print "\n---without tries---\n{$without_tries_text}\n";
  print "---without tries or first method ---\n{$without_first_text}\n";
}

Asio::enterAsyncEntryPoint(() ==> my_script_main());

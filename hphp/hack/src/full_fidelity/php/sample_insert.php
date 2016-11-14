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
require_once 'full_fidelity_parser.php';
$file = 'sample_insert_input.php';
$original = parse_file_to_editable($file);

// In an "editable" tree every node knows its text, but not its
// position within the tree. This means that we can reorganize /
// replace nodes in the tree without having to recompute positions
// of every node in the tree.

// Editable trees are immutable. We do not mutate
// an existing tree; we run a non-detructive visitor over it which
// produces a new tree. The new tree shares as many nodes as possible
// with the old tree, so this is pretty memory-efficient.

$tries = $original->of_syntax_kind('try_statement');
$first_try_body = firstx($tries)->compound_statement();
$comment = new DelimitedComment('/* HELLO WORLD */');
$rewritten_try = $original->insert_before($comment, $first_try_body);
$catches = $rewritten_try->of_syntax_kind('catch_clause');
$left_brace = firstx($catches)->body()->left_brace();
$rewritten = $rewritten_try->insert_after($comment, $left_brace);
print "---original---";
print $original->full_text();
print "---rewritten---";
print $rewritten->full_text();

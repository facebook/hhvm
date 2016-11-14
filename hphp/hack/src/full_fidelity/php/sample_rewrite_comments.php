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

require_once 'full_fidelity_editable.php';

// Turn any comment associated with a try statement into blah blah blah.
// Note that this will detect both comments that appear before the try
// token and comments that appear anywhere inside the try / catch / finally,
// and comments which immediately follow the trailing } of the statement.
$rewriter = function (EditableSyntax $node, array<EditableSyntax> $parents) {
  if ($node->syntax_kind() === 'single_line_comment')
    return $node->with_text('// blah blah blah');
  else if ($node->syntax_kind() === 'delimited_comment')
    return $node->with_text('/* blah blah blah */');
  return $node;
};
// In an "editable" tree every node knows its text, but not its
// position within the tree. This means that we can reorganize /
// replace nodes in the tree without having to recompute positions
// of every node in the tree.

// Editable trees are immutable. We do not mutate
// an existing tree; we run a non-detructive visitor over it which
// produces a new tree. The new tree shares as many nodes as possible
// with the old tree, so this is pretty memory-efficient.

$file = 'sample_rewrite_comments_input.php';
$json = parse_file_to_json($file);
$original = from_json($json);
$rewritten = $original->rewrite($rewriter);

print ('---original---\n');
print ($original->full_text());

print ('\n---rewritten---\n');
print ($rewritten->full_text());

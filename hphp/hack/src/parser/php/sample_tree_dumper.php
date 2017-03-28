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

function dump(
  EditableSyntax $node,
  string $indent,
  (function (string): void) $printer) : void {
  // Precondition: indentation and prefix has already been output
  // Precondition: indent is correct for node's children
  $printer ($node->syntax_kind());
  if ($node->is_token())
    $printer (' '. $node->token_kind());
  $printer ("\n");
  $children = iterator_to_array($node->children());
  $c = count($children);
  $i = 0;
  $horiz = "\u{2500}";
  $el = "\u{2514}";
  $turn = "\u{251C}";
  $vert = "\u{2502}";
  if (!$node->is_token())
  {
    foreach($children as $child) {
      $last = $i === $c - 1;
      $printer($indent);
      $printer($last ? $el : $turn);
      $printer($horiz);
      dump($child, $indent . ($last ? ' ' : $vert) . ' ', $printer);
      ++$i;
    }
  }
}

async function my_script_main(): Awaitable<void> {
  $text = "<?hh\nfunction foo() {}\nfunction bar() {}";
  $root = parse_text_to_editable($text);
  $printer = ($s) ==> { print $s; };
  dump($root, '', $printer);
}
Asio::enterAsyncEntryPoint(() ==> my_script_main());

<?php

function bar($x) {
  // just here to make sure foo isn't a leaf function
  // since FCallBuiltin doesn't count as a php-call.
}

function foo($x) {
  $x = (array)$x;
  array_walk_recursive($x, "foo");
  bar($x);
}

foo(1);

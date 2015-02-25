<?php

function f($a, $b, $c) {
  // no need for parens
  if ($a or $b or $c) {};
  if ($a and $b and $c) {};
  if ($a and $b or $c) {};
  if ($a and $b + $c) {};
  if ($a or $b && $c) {}
  if ($a and $b xor $c) {}
  // this doesn't need parens since `&&` is associative
  if ($a and $b && $c) {}

  // these need parens
  if ($a and $b ?: $c) {}
  if ($a = $b and $c) {}
  if ($a and $b || $c) {}
  if ($a or $b xor $c) {}

  return $a <> $b;
}

<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function badInt(bool $b): int {
  if ($b) {
    $x = 3;
  } else {
    $x = 3.4;
  }
  $z = 3 * $x;
  if (is_int($z)) {
    echo 'int';
  } else {
    echo 'not int';
  }
  return $z;
}

/* HH_FIXME[1002] */
badInt(false);

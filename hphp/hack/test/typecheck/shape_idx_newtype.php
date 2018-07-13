//// def.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

newtype N as shape('x' => ?int, ...) = shape('x' => ?int, 'y' => string);

//// use.php
<?hh // strict

function test(N $s): int {
  return Shapes::idx($s, 'x', 0);
}

//// def.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

newtype N as shape('x' => ?int, ...) = shape('x' => ?int, 'y' => string);

//// use.php
<?hh

function test(N $s): int {
  return Shapes::idx($s, 'x', 0);
}

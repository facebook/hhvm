//// def.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

newtype N = mixed;

//// use.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test<T>(N $x): ?T {
  return $x;
}

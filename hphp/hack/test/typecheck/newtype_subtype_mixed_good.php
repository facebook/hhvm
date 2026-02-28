//// def.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

newtype N = mixed;

//// use.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test<T super nonnull>(N $x): ?T {
  return $x;
}

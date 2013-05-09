<?hh
// Copyright 2004-2013 Facebook. All Rights Reserved.

function toto(mixed $x, ...): int {
  return (int)$x;
}

echo toto(1), "\n";
echo toto("1"), "\n";

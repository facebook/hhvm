<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

function toto(mixed $x, ...$_): int {
  return (int)$x;
}
<<__EntryPoint>> function main(): void {
echo toto(1), "\n";
echo toto("1"), "\n";
}

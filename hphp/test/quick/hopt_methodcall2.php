<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

function foo($a, $b, $c) :mixed{
  return $a + $b + $c;
}

function main() :mixed{
  $x = 4;
  $x = foo(1, $x == 3 ? 3: 2, $x);
  echo $x;
}
<<__EntryPoint>> function main_entry(): void {
echo "Starting\n";
main();
echo "\ndone";
}

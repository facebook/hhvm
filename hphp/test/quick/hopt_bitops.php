<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

function foo($a, $b):mixed{
  return (int)$a & (int)$b;
}

function test($a, $b) :mixed{
  $a__str = (string)($a);
  $b__str = (string)($b);
  echo "test foo($a__str, $b__str)\n";
  for ($i = 0; $i < 20; $i++) {
    $x = foo($a, $b);
    echo $x;
    echo ", ";
  }
  echo "\n";
}
<<__EntryPoint>> function main(): void {
test(true, true);
test(1, 3);
test( 2.3, 4.6);
}

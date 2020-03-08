// SUPPORT IS NOT YET IMPLEMENTED <?hh // strict

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015-2016 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

function gen(): Generator<int, int, void> {
    yield 1;
    yield from gen2();
    yield 4;
}

function gen2(): Generator<int, int, void> {
    yield 2;
    yield 3;
}

function g(): Generator<int, int, void> {
  yield 1;
  yield from [2, 3];
  yield 4;
}

function main(): void {
  echo "====== yielding from another generator =========\n\n";

  foreach (gen() as $val)
  {
    echo $val . "\n";
  }

  echo "\n====== yielding from an array =========\n\n";
 
  $g = g();
  foreach ($g as $yielded) {
    echo $yielded . "\n";
  }
}

/* HH_FIXME[1002] call to main in strict*/
main();

// NOT YET IMPLEMENTED   <?hh // strict

namespace NS_Generator_getReturn;

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015-2016 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

function foo(): \Generator<int, int, void> {
    yield 1;
    yield 2;
    return 42;
}

function main(): void { 
  $bar = foo();

  try {
    var_dump($bar->getReturn());
    echo "Call to getReturn succeded\n";
  }
  catch (\Exception $e) {
    echo "Call to getReturn failed\n";
    echo $e . "\n";
  }

  foreach ($bar as $element) {
    echo $element, "\n";
  }
 
  var_dump($bar->getReturn());
}

/* HH_FIXME[1002] call to main in strict*/
main();

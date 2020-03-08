<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015-2016 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

declare(strict_types = 1);

function doubler(int $p) : int
{
    return $p * 2;
}

function main(): void {
//  var_dump(doubler(10.3)); // Throws a TypeError
  var_dump(doubler(10));

  echo "\n================ End of script ===================\n\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();

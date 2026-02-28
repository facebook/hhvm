<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

function doit($p1)  // assigned the value TRUE when called
:mixed{
    $count = 10;
//  …
    if ($p1)
    {
        $message = "Can't open master file.";
//      …
    }
//  …
}

function factorial($i)
:mixed{
    if ($i > 1) return $i * factorial($i - 1);
    else if ($i == 1) return $i;
    else return 0;
}

const MAX_HEIGHT2 = 10.5;       // define two c-constants
const UPPER_LIMIT2 = MAX_HEIGHT2;
const COEFFICIENT_2 = 2.345; // define two d-constants
const FAILURE2 = TRUE;

function globalConst()
:mixed{
    echo "Inside " . __FUNCTION__ . "\n";
    echo "MAX_HEIGHT2 = " . (string)(MAX_HEIGHT2) . "\n";
    echo "COEFFICIENT_2 = " . (string)(COEFFICIENT_2) . "\n";
}

class Point
{
    const MAX_COUNT = 1000;

    private static $pointCount = 0;

    public $x;
    public $y;
}
<<__EntryPoint>>
function entrypoint_variable_kinds(): void {

  error_reporting(-1);


  echo "---------------- Local variables -------------------\n";
  doit(TRUE);

  echo "---------------- Array elements -------------------\n";

  echo "---------------- recursive function example -------------------\n";

  \HH\global_set('result', factorial(10));
  $result = \HH\global_get('result');
  echo "\$result = {$result}\n";

  echo "---------------- Global Constants -------------------\n";
  echo "MAX_HEIGHT2 = " . (string)(MAX_HEIGHT2) . "\n";

  globalConst();

  echo "---------------- instance/static properties & constants -------------------\n";
}

<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

echo "---------------- Local constants -------------------\n";

function localConst($p)
{
    echo "Inside " . __FUNCTION__ . "\n";
    define('COEFFICIENT_1', 2.345); // define two d-constants
    echo "COEFFICIENT_1 = " . COEFFICIENT_1 . "\n";
    if ($p)
    {
        echo "COEFFICIENT_1 = " . COEFFICIENT_1 . "\n";
        define('FAILURE', TRUE);
        echo "FAILURE = " . FAILURE . "\n";
    }
}

localConst(TRUE);
echo "COEFFICIENT_1 = " . COEFFICIENT_1 . "\n";
echo "FAILURE = " . FAILURE . "\n"; // as it's visible here, it's not really a local!

echo "---------------- Local variables -------------------\n";

function doit($p1)  // assigned the value TRUE when called
{
    $count = 10;
//  …
    if ($p1)
    {
        $message = "Can't open master file.";
//      …
    }
//  …
}
doit(TRUE);

echo "---------------- Array elements -------------------\n";

echo "---------------- Function statics -------------------\n";

function f()
{
    $lv = 1;
    static $fs = 1;
    static $fs2;
    var_dump($fs2);     // show default value is NULL

    echo "\$lv = $lv, \$fs = $fs\n";
    ++$lv;
    ++$fs;
    if (TRUE)
    {
        static $fs3 = 99;
        echo "\$fs3 = $fs3\n";
        ++$fs3;
    }
}

for ($i = 1; $i <= 3; ++$i)
    f();

echo "---------------- recursive function example -------------------\n";

function factorial($i)
{
    if ($i > 1) return $i * factorial($i - 1);
    else if ($i == 1) return $i;
    else return 0;
}

$result = factorial(10);
echo "\$result = $result\n";

echo "---------------- Global Constants -------------------\n";

const MAX_HEIGHT2 = 10.5;       // define two c-constants
const UPPER_LIMIT2 = MAX_HEIGHT2;
define('COEFFICIENT_2', 2.345); // define two d-constants
define('FAILURE2', TRUE);
echo "MAX_HEIGHT2 = " . MAX_HEIGHT2 . "\n";

function globalConst()
{
    echo "Inside " . __FUNCTION__ . "\n";
    echo "MAX_HEIGHT2 = " . MAX_HEIGHT2 . "\n";
    echo "COEFFICIENT_2 = " . COEFFICIENT_2 . "\n";
}

globalConst();

echo "---------------- Global Variables using names directly -------------------\n";

$colors = array("red", "white", "blue");

$min = 10;
$max = 100;
$average = NULL;

global $min, $max;      // allowed, but serve no purpose

function compute($p)
{
    global $min, $max;
    global $average;
    $average = ($max + $min)/2;

    if ($p)
    {
        global $result;
        $result = 3.456;        // initializes a global, creating it if necessary
    }
}

compute(TRUE);
echo "\$average = $average\n";
echo "\$result = $result\n";

//var_dump($GLOBALS);

echo "---------------- Global Variables using \$GLOBALS -------------------\n";

$GLOBALS['done'] = FALSE;
var_dump($done);

$GLOBALS['min'] = 10;
$GLOBALS['max'] = 100;
$GLOBALS['average'] = NULL;

global $min, $max;      // allowed, but serve no purpose

function compute2($p)
{
    $GLOBALS['average'] = ($GLOBALS['max'] + $GLOBALS['min'])/2;

    if ($p)
    {
        $GLOBALS['result'] = 3.456;     // initializes a global, creating it if necessary
    }
}

compute2(TRUE);
echo "\$average = $average\n";
echo "\$result = $result\n";

//var_dump($GLOBALS);

echo "---------------- instance/static properties & constants -------------------\n";

class Point
{
    const MAX_COUNT = 1000;

    private static $pointCount = 0;

    public $x;
    public $y;
}

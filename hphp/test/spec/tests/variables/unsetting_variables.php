<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

echo "--- At top-level\n";

///*
const CON1 = 1;
//unset(CON1);              // invalid
//var_dump(isset(CON1));    // invalid
var_dump(defined('CON1'));  // TRUE

define('CON2', 2);
//unset(CON2);              // invalid
//var_dump(isset(CON2));    // invalid
var_dump(defined('CON2'));  // TRUE

echo "Top1: CON1 = " . CON1 . "\n";
echo "Top1: CON2 = " . CON2 . "\n";

echo "---\n";
//*/

static $sVar1 = 11;
echo "Top1: \$sVar1 = " . $sVar1 . "\n";
//unset($sVar1);    // unsets this variable
//echo "Top1: \$sVar1 = " . $sVar1 . "\n";

$tmp =& $sVar1;
echo "Top1: \$tmp = " . $tmp . "\n";

$gVar1 = 21;
echo "Top1: \$gVar1 = " . $gVar1 . "\n";
//unset($gVar1);    // unsets this variable
//echo "Top1: \$gVar1 = " . $gVar1 . "\n";

function f($p)
{
    echo "--- Inside " . __FUNCTION__ . ", block-level 1\n";
///*
    define('CON3', 3);
    echo "f:1 CON1 = " . CON1 . "\n";
    echo "f:1 CON2 = " . CON2 . "\n";
    echo "f:1 CON3 = " . CON3 . "\n";
//*/
    if ($p)
    {
        echo "--- Inside " . __FUNCTION__ . ", block-level 2\n";
///*
        define('CON4', 4);
        echo "f:2 CON1 = " . CON1 . "\n";
        echo "f:2 CON2 = " . CON2 . "\n";
        echo "f:2 CON3 = " . CON3 . "\n";
        echo "f:2 CON4 = " . CON4 . "\n";
//*/
        global $sVar1;
        $sVar1 = -1;        // hmm; is setting the top-level static
        echo "f:2 \$sVar1 = " . $sVar1 . "\n";
        static $sVar1 = 12;
        echo "f:2 \$sVar1 = " . $sVar1 . "\n";
        ++$sVar1;
//      unset($sVar1);  // removes this alias; doesn't unset the inner static itself
        
        global $gVar1;
        echo "f:2 \$gVar1 = " . $gVar1 . "\n";
        $gVar1 = 25;
        echo "f:2 \$gVar1 = " . $gVar1 . "\n";
    }
    echo "--- Back at f:1\n";
///*
    echo "f:1 CON4 = " . CON4 . "\n";
//*/
    echo "f:1 \$sVar1 = " . $sVar1 . "\n";  // is getting f:2's static, so that didn't
                                            // go out of scope at the closing }
    echo "f:1 \$gVar1 = " . $gVar1 . "\n";  // still seeing $gVar1 even though global decl
                                            // is presumed to have gone out of scope, and there is
                                            // global import at block level 1
}

f(TRUE);
echo "--- Back at top-level\n";
f(TRUE);
echo "--- Back at top-level\n";

///*
echo "Top2: CON1 = " . CON1 . "\n";
echo "Top2: CON2 = " . CON2 . "\n";
echo "Top2: CON3 = " . CON3 . "\n";
echo "Top2: CON4 = " . CON4 . "\n";

echo "---\n";
//*/
echo "Top2: \$sVar1 = " . $sVar1 . "\n";
echo "Top2: \$tmp = " . $tmp . "\n";

echo "Top2: \$gVar1 = " . $gVar1 . "\n";

echo "-----------------------------------------\n";

function f2($p)
{
    static $s2 = 100;

    if ($p)
    {
//      global $s2; // with this in, var_dump($s2) is NULL, becuse there is no such global
        var_dump($s2);
        var_dump(isset($s2));
    }
}

f2(TRUE);

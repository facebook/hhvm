<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

echo "--- At top-level\n";

///*
const CON1 = 1;
//unset(CON1);              // invalid
//var_dump(isset(CON1));    // invalid
var_dump(defined('CON1'));  // TRUE

const CON2 = 2;
//unset(CON2);              // invalid
//var_dump(isset(CON2));    // invalid
var_dump(defined('CON2'));  // TRUE

echo "Top1: CON1 = " . CON1 . "\n";
echo "Top1: CON2 = " . CON2 . "\n";

echo "---\n";
//*/

$gVar1 = 21;
echo "Top1: \$gVar1 = " . $gVar1 . "\n";
//unset($gVar1);    // unsets this variable
//echo "Top1: \$gVar1 = " . $gVar1 . "\n";

const CON3 = 3;
const CON4 = 4;
function f($p)
{
    echo "--- Inside " . __FUNCTION__ . ", block-level 1\n";
///*
    echo "f:1 CON1 = " . CON1 . "\n";
    echo "f:1 CON2 = " . CON2 . "\n";
    echo "f:1 CON3 = " . CON3 . "\n";
//*/
    if ($p)
    {
        echo "--- Inside " . __FUNCTION__ . ", block-level 2\n";
///*
        echo "f:2 CON1 = " . CON1 . "\n";
        echo "f:2 CON2 = " . CON2 . "\n";
        echo "f:2 CON3 = " . CON3 . "\n";
        echo "f:2 CON4 = " . CON4 . "\n";
//*/
        global $sVar1;
        $sVar1 = -1;        // hmm; is setting the top-level static
        echo "f:2 \$sVar1 = " . $sVar1 . "\n";

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

echo "Top2: \$gVar1 = " . $gVar1 . "\n";

echo "-----------------------------------------\n";

<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

///*
$oper = array([1, 5 => FALSE, "red"], [NULL], [4 => -5, 1.23]);

foreach ($oper as $e1)
{
    foreach ($oper as $e2)
    {
        echo ">$e1< + >$e2<, result: "; var_dump($e1 + $e2);
    }
    echo "-------------------------------------\n";
}
//*/

/*
// the following LHS-operands all have "Unsupported operand types"

-10    + [1, 5 => FALSE, "red"];
3.4e10 + [1, 5 => FALSE, "red"];
TRUE   + [1, 5 => FALSE, "red"];
NULL   + [1, 5 => FALSE, "red"];
""     + [1, 5 => FALSE, "red"];
"123"  + [1, 5 => FALSE, "red"];

// likewise for the following RHS-operands

[1, 5 => FALSE, "red"] + -10;
[1, 5 => FALSE, "red"] + 3.4e10;
[1, 5 => FALSE, "red"] + TRUE;
[1, 5 => FALSE, "red"] + NULL;
[1, 5 => FALSE, "red"] + "";
[1, 5 => FALSE, "red"] + "123";

// So we conclude that if one operand is an array and the other not, the array
// is not converted to a string and concatenated, and neither is the non-array
// converted to an array and merged with the other array.
*/

///*
// show that a new array is created and that the operand-arrays are unchanged

$a1 = [1, 5 => FALSE, "red"];   // [0], [5], [6]
$a2 = [4 => -5, 1.23];          // [4], [5]

$a3 = $a1 + $a2;    // [0], [5], [6], [4]
var_dump($a3);
$a3[0] = 11;
$a3[6] = 99;
var_dump($a3);
var_dump($a1);
var_dump($a2);
//*/

<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

// check order of evaluation of arguments in a function call w.r.t side effects

function f($p1, $p2, $p3, $p4, $p5)
{
    echo "f: \$p1 = $p1, \$p2 = $p2, \$p3 = $p3, \$p4 = $p4, \$p5 = $p5\n";
}

$i = 0;
f($i, ++$i, $i, $i = 12, --$i);     // arguments are evaluated L->R
// f: $p1 = 0, $p2 = 1, $p3 = 1, $p4 = 12, $p5 = 11

function g($p1, $p2, $p3, $p4, $p5)
{
    echo "g: \$p1 = $p1, \$p2 = $p2, \$p3 = $p3, \$p4 = $p4, \$p5 = $p5\n";
}

function h($p1, $p2, $p3, $p4, $p5)
{
    echo "h: \$p1 = $p1, \$p2 = $p2, \$p3 = $p3, \$p4 = $p4, \$p5 = $p5\n";
}

// Create a table of function designators

$funcTable = array(f, g, h);    // list of 3 functions
var_dump($funcTable);           // array of 3 strings
var_dump($funcTable[0]);        // a string

// Call all 3 functions indirectly through table

$funcTable[0](1,2,3,4,5);
$funcTable[1](10,20,30,40,50);
$funcTable[2](100,200,300,400,500);

// Put a side effect in the function designator and see the order of evaluation of
// that compared with the argument list expressions.

$i = 1;
$funcTable[$i++]($i, ++$i, $i, $i = 12, --$i);  // function designator side effect done first
// g: $p1 = 2, $p2 = 3, $p3 = 3, $p4 = 12, $p5 = 11

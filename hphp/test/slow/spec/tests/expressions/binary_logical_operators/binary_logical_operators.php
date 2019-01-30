<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

$month = 6;
if ($month > 1 && $month <= 12)
    echo "\$month $month is in-bounds\n";
else
    echo "\$month $month is out-of-bounds\n";

$month = 14;
if ($month > 1 && $month <= 12)
    echo "\$month $month is in-bounds\n";
else
    echo "\$month $month is out-of-bounds\n";

$month = 6;
if ($month < 1 || $month > 12)
    echo "\$month $month is out-of-bounds\n";
else
    echo "\$month $month is in-bounds\n";

$month = 14;
if ($month < 1 || $month > 12)
    echo "\$month $month is out-of-bounds\n";
else
    echo "\$month $month is in-bounds\n";

// sequence point

function f($a) { echo "inside f($a)\n"; return 10;}
function g($a) { echo "inside g($a)\n"; return 0;}

$i = 5;
$v = (f($i++) AND g($i));
var_dump($v);
$i = 0;
$v = (g($i++) OR f($i));
var_dump($v);
$i = 5;
$v = (f($i++) XOR g($i));
var_dump($v);

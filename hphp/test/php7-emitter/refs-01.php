<?php

$x = 2;
$y =& $x;

$y++;
echo $x . "\n";

$z =& $y;
$z++;
echo $x . "\n";

$i= 100;
$y =& $i;
echo $x . "\n";
echo $y . "\n";

$y *= 3;
echo $x . "\n";
echo $i . "\n";
echo $z . "\n";

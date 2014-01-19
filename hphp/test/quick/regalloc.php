<?php

// This test exercises the register allocator, because it results in a single
// big tracelet that has more live values in flight than there are physical
// registers.  (For reference, this was written when three registers are
// reserved for VM state.)  It will expose bugs more noticeably when register
// poisoning is enabled.

$a = 1;
$b = 1;
$c = 2;
$d = 2;
$e = 3;
$f = 3;
$g = 4;
$h = 4;
$i = 5;
$j = 5;

echo $a;
echo $b;
echo $c;
echo $d;
echo $e;
echo $f;
echo $g;
echo $h;
echo $i;
echo $j;

echo "\n";

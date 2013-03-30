<?php

$a = 1;
$c = function($add) use(&$a) { return $a+$add; };

$cc = clone $c;

echo $c(10)."\n";
echo $cc(10)."\n";

$a++;

echo $c(10)."\n";
echo $cc(10)."\n";

echo "Done.\n";
?>
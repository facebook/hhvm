<?php

$d1 = 1.1;
$d2 = 434234.234;

$c = $d2 / $d1;
var_dump($c);

$d1 = 1.1;
$d2 = "434234.234";

$c = $d2 / $d1;
var_dump($c);

$d1 = "1.1";
$d2 = "434234.234";

$c = $d2 / $d1;
var_dump($c);

echo "Done\n";
?>
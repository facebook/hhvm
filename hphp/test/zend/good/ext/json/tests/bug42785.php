<?php
setlocale(LC_ALL, "de_DE", "de", "german", "ge", "de_DE.ISO8859-1", "ISO8859-1");

$foo = Array(100.10,"bar");
var_dump(json_encode($foo));

Class bar {}
$bar1 = new bar;
$bar1->a = 100.10;
$bar1->b = "foo";
var_dump(json_encode($bar1));

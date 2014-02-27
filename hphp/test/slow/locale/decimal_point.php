<?php

// PHP's string -> double conversion doesn't care about locale.
setlocale(LC_ALL, 'fr_FR');

$a1 = '1.5';
$a2 = '1,5';
$b = 1;
var_dump($a1 * $b);
var_dump($a2 * $b);

var_dump('1.5' * 1);
var_dump('1,5' * 1);

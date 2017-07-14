<?php

$a = [1, 2, 3];
list($a, $b, $c) = $a;
var_dump($a, $b, $c);

$b = [1, 2, 3];
list($a, $b, $c) = $b;
var_dump($a, $b, $c);

$c = [1, 2, 3];
list($a, $b, $c) = $c;
var_dump($a, $b, $c);

$a = [[1, 2], 3];
list(list($a, $b), $c) = $a;
var_dump($a, $b, $c);

$b = [[1, 2], 3];
list(list($a, $b), $c) = $b;
var_dump($a, $b, $c);

$b = [1, [2, 3]];
list($a, list($b, $c)) = $b;
var_dump($a, $b, $c);

$c = [1, [2, 3]];
list($a, list($b, $c)) = $c;
var_dump($a, $b, $c);

?>

<?php

$a = create_function('$x', 'return function($y) use ($x) { return $x * $y; };');

var_dump($a(2)->__invoke(4));

?>
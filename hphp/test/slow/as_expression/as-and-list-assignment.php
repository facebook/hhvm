<?hh

$x = tuple(1,2);

list($a, $b) = $x as (int, int);

var_dump($x, $a, $b);

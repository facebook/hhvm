<?php

$a = 'foo';
$b =& $a;

var_dump(isset($b));

var_dump(isset($a[0], $b[1]));

var_dump(isset($a[0]->a));

var_dump(isset($c[0][1][2]->a->b->c->d));

var_dump(isset(${$a}->{$b->$c[$d]}));

var_dump(isset($GLOBALS));

var_dump(isset($GLOBALS[1]));

var_dump(isset($GLOBALS[1]->$GLOBALS));

?>
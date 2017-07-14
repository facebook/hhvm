<?php

var_dump(+(-0.0));
var_dump(+(float)"-0");

$foo = +(-sin(0));

var_dump($foo);

?>

<?php

$name = 'var';
$ref =& $name;

$$name = 42;
var_dump(isset($$name));
unset($$name);
var_dump(isset($$name));

?>

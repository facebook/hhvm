<?php

$name = 'var';
var_dump(isset($$name));
$var = 42;
var_dump(isset($$name));

?>

<?php

$i = PHP_INT_MAX;
$array = [$i => 42, new stdClass];
var_dump($array);

const FOO = [PHP_INT_MAX => 42, "foo"];
var_dump(FOO);

?>

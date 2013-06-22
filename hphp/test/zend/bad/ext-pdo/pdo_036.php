<?php

$instance = new reflectionclass('pdostatement');
$x = $instance->newInstance();
var_dump($x);

$instance = new reflectionclass('pdorow');
$x = $instance->newInstance();
var_dump($x);

?>
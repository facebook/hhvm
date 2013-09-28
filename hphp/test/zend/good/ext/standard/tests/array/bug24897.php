<?php
$a = array(1 => 2);
shuffle($a);
var_dump($a);

$a = array(1 => 2);
array_multisort($a);
var_dump($a);
?>
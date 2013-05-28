<?php

$a = array('a' => '1', 2 => 2, 'c' => '3');
var_dump($a);
$a = array('a' => '1', 2 => 2, 'c' => '3',           'd' => array('a' => '1', 2 => 2, 'c' => '3'));
var_dump($a);

<?php
error_reporting(-1);
$arr = array('a', 'b', 'c');
$args = array(array(3, 1, 2), &$arr);
var_dump(call_user_func_array('array_multisort', $args));
var_dump($arr);
var_dump($args);

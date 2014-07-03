<?php
error_reporting(-1);
$arr = array('a', 'b', 'c');
var_dump(array_multisort(array(3, 1, 2), $arr));
var_dump($arr);

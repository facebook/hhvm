<?php 

$a = new stdClass;
var_dump($a instanceof stdClass);

var_dump(new stdCLass instanceof stdClass);

$b = create_function('', 'return new stdClass;');
var_dump($b() instanceof stdClass);

$c = array(new stdClass);
var_dump($c[0] instanceof stdClass);

var_dump(@$inexistent instanceof stdClass);

var_dump("$a" instanceof stdClass);

?>
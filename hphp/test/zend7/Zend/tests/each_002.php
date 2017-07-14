<?php

$a = new stdClass;
$foo = each($a);
var_dump($foo);

$a = new stdClass;
var_dump(each($a));

$a = array(new stdClass);
var_dump(each($a));


?>

<?php

$foo = each(new stdClass);
var_dump($foo);

var_dump(each(new stdClass));

$a = array(new stdClass);
var_dump(each($a));


?>
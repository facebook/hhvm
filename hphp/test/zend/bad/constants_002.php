<?php

define('foo', new stdClass);
var_dump(foo);

define('foo', fopen(__FILE__, 'r'));
var_dump(foo);

?>
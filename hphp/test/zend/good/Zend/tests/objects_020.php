<?php

error_reporting(E_ALL);

$test = 'stdclass';

$$test->a =& $$test;
$$test->a->b[] = 2;

var_dump($$test);

?>
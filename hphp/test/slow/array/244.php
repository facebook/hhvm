<?php

$a = array('a' => null, 'b' => 123, 'c' => false);
var_dump(array_keys($a));
var_dump(array_keys($a, null));
var_dump(array_keys($a, null, true));

<?php

var_dump(HH\could_include(__FILE__));
$n = tempnam(sys_get_temp_dir(), 'could_include');
var_dump(HH\could_include($n));
unlink($n);
var_dump(HH\could_include($n));

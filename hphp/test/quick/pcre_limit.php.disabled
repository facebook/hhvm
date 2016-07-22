<?php

$a = 'baab' . str_repeat('a', 1000000);
$b = preg_replace('/b.*b/', '', $a);
var_dump(preg_last_error());

ini_set('pcre.backtrack_limit', PHP_INT_MAX);

$b = preg_replace('/b.*b/', '', $a);
var_dump(preg_last_error());

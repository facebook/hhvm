<?php

$closure = function() {};
$closure_in_array = (array)$closure;

var_dump(is_array($closure_in_array));
var_dump(count($closure_in_array) === 1);
var_dump($closure_in_array[0] === $closure);

<?php

$array = array("step one", "step two", "step three", "step four");

// by default, the pointer is on the first element
var_dump(current($array));

// skip two steps
next($array);
next($array);
var_dump(current($array));

// reset pointer, start again on step one
reset($array);
var_dump(current($array));

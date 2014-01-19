<?php

var_dump(strripos("t", "t", PHP_INT_MAX+1));
var_dump(strripos("tttt", "tt", PHP_INT_MAX+1));
var_dump(strripos(100, 101, PHP_INT_MAX+1));
var_dump(strripos(1024, 1024, PHP_INT_MAX+1));
var_dump(strripos(array(), array(), PHP_INT_MAX+1));
var_dump(strripos(1024, 1024, -PHP_INT_MAX));
var_dump(strripos(1024, "te", -PHP_INT_MAX));
var_dump(strripos(1024, 1024, -PHP_INT_MAX-1));
var_dump(strripos(1024, "te", -PHP_INT_MAX-1));

echo "Done\n";
?>
<?php

var_dump(strrpos("t", "t", PHP_INT_MAX+1));
var_dump(strrpos("tttt", "tt", PHP_INT_MAX+1));
var_dump(strrpos(100, 101, PHP_INT_MAX+1));
var_dump(strrpos(1024, 1024, PHP_INT_MAX+1));
var_dump(strrpos(1024, 1024, -PHP_INT_MAX));
var_dump(strrpos(1024, "te", -PHP_INT_MAX));
var_dump(strrpos(1024, 1024, -PHP_INT_MAX-1));
var_dump(strrpos(1024, "te", -PHP_INT_MAX-1));

echo "Done\n";
?>

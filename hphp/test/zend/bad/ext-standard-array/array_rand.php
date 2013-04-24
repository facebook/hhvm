<?php

var_dump(array_rand());
var_dump(array_rand(array()));
var_dump(array_rand(array(), 0));
var_dump(array_rand(0, 0));
var_dump(array_rand(array(1,2,3), 0));
var_dump(array_rand(array(1,2,3), -1));
var_dump(array_rand(array(1,2,3), 10));
var_dump(array_rand(array(1,2,3), 3));
var_dump(array_rand(array(1,2,3), 2));

echo "Done\n";
?>
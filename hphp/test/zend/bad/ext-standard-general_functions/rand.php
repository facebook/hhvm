<?php

var_dump(mt_rand());
var_dump(mt_rand(-1));
var_dump(mt_rand(-1,1));
var_dump(mt_rand(0,3));

var_dump(rand());
var_dump(rand(-1));
var_dump(rand(-1,1));
var_dump(rand(0,3));

var_dump(srand());
var_dump(srand(-1));
var_dump(srand(array()));

var_dump(mt_srand());
var_dump(mt_srand(-1));
var_dump(mt_srand(array()));

var_dump(getrandmax());
var_dump(getrandmax(1));

var_dump(mt_getrandmax());
var_dump(mt_getrandmax(1));

echo "Done\n";
?>
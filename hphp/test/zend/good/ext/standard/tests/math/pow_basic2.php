<?php
ini_set('precision', 14);


$large_exp = 20000;

echo "\n-- The following all result in INF --\n";
var_dump(pow(24, $large_exp)); 
var_dump(pow(0.24, -$large_exp));
var_dump(pow(-0.24, -$large_exp));

echo "\n\n-- The following all result in 0 --\n";
var_dump(pow(0.24, $large_exp));
var_dump(pow(-0.24, $large_exp));
var_dump(pow(24, -$large_exp));
var_dump(pow(-24, -$large_exp));  

echo "\n\n-- The following all result in -0 --\n";
var_dump(pow(-0.24, $large_exp+1));

echo "\n\n-- The following all result in -INF --\n";
var_dump(pow(-24, $large_exp+1));
var_dump(pow(-0.24, -$large_exp+1));

?>
===Done===
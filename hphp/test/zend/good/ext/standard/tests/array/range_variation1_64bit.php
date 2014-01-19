<?php

echo "\n*** Testing max/outof range values ***\n";
/*var_dump( range("a", "z", 255) );
var_dump( range("z", "a", 255) ); */
var_dump( range(2147483645, 2147483646) );
var_dump( range(2147483646, 2147483648) );
var_dump( range(-2147483647, -2147483646) );
var_dump( range(-2147483648, -2147483647) );
var_dump( range(-2147483649, -2147483647) );

echo "\nDone";
?>
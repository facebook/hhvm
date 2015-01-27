<?php

var_dump(gmp_rootrem());

var_dump(gmp_rootrem(1000, 3));
var_dump(gmp_rootrem(100, 3));
var_dump(gmp_rootrem(-100, 3));

var_dump(gmp_rootrem(1000, 4));
var_dump(gmp_rootrem(100, 4));
var_dump(gmp_rootrem(-100, 4));

var_dump(gmp_rootrem(0, 3));
var_dump(gmp_rootrem(100, 0));
var_dump(gmp_rootrem(100, -3));

?>

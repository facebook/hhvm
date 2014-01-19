<?php 

$test = <<<"TEST"
TEST;

var_dump(strlen($test) == 0);

$test = <<<"TEST"
\
TEST;

var_dump(strlen($test) == 1);

$test = <<<"TEST"
\0
TEST;

var_dump(strlen($test) == 1);

$test = <<<"TEST"
\n
TEST;

var_dump(strlen($test) == 1);

$test = <<<"TEST"
\\'
TEST;

var_dump(strlen($test) == 2);


?>
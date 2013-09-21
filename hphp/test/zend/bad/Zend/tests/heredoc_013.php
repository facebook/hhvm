<?php
$test = "foo";
$var = prefix<<<"MYLABEL"
test: $test
MYLABEL;
echo $var;
?>
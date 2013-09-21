<?php
/* generate different failure conditions */
$int_var  = -19;
$item = array ("one" => 1, "two" => 2, "THREE" => 3, "FOUR" => "four");

var_dump( array_change_key_case($int_var) ); // args less than expected
var_dump( array_change_key_case($int_var, CASE_UPPER) ); // invalid first argument
var_dump( array_change_key_case() ); // Zero argument
var_dump( array_change_key_case($item, $item["one"], "CASE_UPPER") ); // more than expected numbers

echo "end\n";
?>
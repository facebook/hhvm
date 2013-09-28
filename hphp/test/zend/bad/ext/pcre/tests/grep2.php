<?php

var_dump(preg_grep(1,array(),3,4));
var_dump(preg_grep(1, 2));
var_dump(preg_grep('/+/', array()));

$array = array(5=>'a', 'x' => '1', 'xyz'=>'q6', 'h20');

var_dump(preg_grep('@^[a-z]+@', $array));
var_dump(preg_grep('@^[a-z]+@', $array, PREG_GREP_INVERT));

ini_set('pcre.recursion_limit', 1);
var_dump(preg_last_error() == PREG_NO_ERROR);
var_dump(preg_grep('@^[a-z]+@', $array));
var_dump(preg_last_error() == PREG_RECURSION_LIMIT_ERROR);

?>
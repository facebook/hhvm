<?php
$s = (string)(-PHP_INT_MAX-1);
var_dump(intval($s));
var_dump(filter_var($s, FILTER_VALIDATE_INT));
?>
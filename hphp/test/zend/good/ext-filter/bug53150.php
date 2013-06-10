<?php
var_dump(filter_var('127.0.0.1', FILTER_VALIDATE_IP));
var_dump(filter_var("::1", FILTER_VALIDATE_IP));

var_dump(filter_var('127.0.0.1', FILTER_VALIDATE_IP, FILTER_FLAG_NO_RES_RANGE));
var_dump(filter_var('::1', FILTER_VALIDATE_IP, FILTER_FLAG_NO_RES_RANGE));

var_dump(filter_var('128.0.0.1', FILTER_VALIDATE_IP));
var_dump(filter_var('128.0.0.1', FILTER_VALIDATE_IP, FILTER_FLAG_NO_RES_RANGE));

var_dump(filter_var('191.255.0.0', FILTER_VALIDATE_IP));
var_dump(filter_var('191.255.0.0', FILTER_VALIDATE_IP, FILTER_FLAG_NO_RES_RANGE));

?>
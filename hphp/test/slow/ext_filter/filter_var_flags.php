<?php
var_dump(filter_var('127.0.0.1', FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));
var_dump(filter_var('127.0.0.1', FILTER_VALIDATE_IP, 2097152));
var_dump(filter_var('127.0.0.1', FILTER_VALIDATE_IP, '2097152'));

var_dump(filter_var('127.0.0.1', FILTER_VALIDATE_IP, ''));
?>

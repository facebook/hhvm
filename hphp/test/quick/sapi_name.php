<?php

$const = PHP_SAPI;
$value_from_func = php_sapi_name();

var_dump($const == $value_from_func);

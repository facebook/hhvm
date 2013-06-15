<?php

$dt = date_create("asdfasdf");
$errs = date_get_last_errors();
var_dump(count($errs) === 4);

var_dump($errs['warning_count'] === 1);
$err_warnings = $errs['warnings'];
var_dump(count($err_warnings) === 1);
var_dump($err_warnings[6] === "Double timezone specification");

var_dump($errs['error_count'] === 1);
$err_errors = $errs['errors'];
var_dump(count($err_errors) === 1);
var_dump($err_errors[0] === "The timezone could not be found in the database");

<?php

var_dump(function_exists("strlen"));
var_dump(is_callable("strlen"));
var_dump(strlen("xxx"));
var_dump(defined("PHP_VERSION"));
var_dump(constant("PHP_VERSION"));
var_dump(call_user_func("strlen"));
var_dump(is_string("xxx"));


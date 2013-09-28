<?php

assert_options(1);
var_dump(dl(""));

var_dump(extension_loaded("bcmath"));
var_dump(extension_loaded("curl"));
var_dump(extension_loaded("simplexml"));
var_dump(extension_loaded("mysql"));

$x = get_loaded_extensions();
var_dump(empty($x));

var_dump(get_included_files()[0] === __FILE__);
var_dump(inclued_get_data());

var_dump(get_magic_quotes_gpc());
var_dump(get_magic_quotes_runtime());


clock_getres(CLOCK_THREAD_CPUTIME_ID, $sec, $nsec);
var_dump($sec);
var_dump($nsec);

var_dump(ini_get(""));
ini_set("memory_limit", 50000000);
var_dump(ini_get("memory_limit"));
set_time_limit(30);
var_dump(ini_get("max_execution_time"));
ini_set("max_execution_time", 40);
var_dump(ini_get("max_execution_time"));

var_dump(phpversion());

var_dump(putenv("FOO=bar"));
var_dump(!putenv("FOO"));

var_dump(!version_compare("1.3.0.dev", "1.1.2", "<"));

var_dump(version_compare(zend_version(), "2.4.99", ">="));

define("MY_CONSTANT", 1);
define("YOUR_CONSTANT", 2);
$arr = get_defined_constants(true);
var_dump(count($arr["user"]) === 2);
var_dump(in_array('PHP_BINARY', $arr["Core"]));

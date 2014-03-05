--TEST--
TSRMG() user globals
--SKIPIF--
<?php if (!extension_loaded("ezc_test")) print "skip"; ?>
--FILE--
<?php
var_dump(ezc_fetch_global());
ezc_set_global("hello");
var_dump(ezc_fetch_global());
?>
--EXPECT--
NULL
string(5) "hello"

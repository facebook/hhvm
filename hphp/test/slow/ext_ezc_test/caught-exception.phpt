--TEST--
Exception intercepted by extension
--SKIPIF--
<?php if (!extension_loaded("ezc_test")) print "skip"; ?>
--FILE--
<?php
var_dump(ezc_try_call(function () {
	throw new Exception("test");
}));

?>
--EXPECTF--
object(Exception)#%d (%d) {
  ["message":protected]=>
  string(4) "test"
%a
}

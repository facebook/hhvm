--TEST--
Wrapped function call throws an exception
--SKIPIF--
<?php if (!extension_loaded("ezc_test")) print "skip"; ?>
--FILE--
<?php

try {
	ezc_call(function () {
		throw new Exception("test");
	});
} catch (Exception $e) {
	var_dump($e);
}
		
?>
--EXPECTF--
object(Exception)#%d (%d) {
  ["message":protected]=>
  string(4) "test"
%a
}

<?php
function err2exception($errno, $errstr)
{
	throw new Exception("Error occuried: " . $errstr);
}

set_error_handler('err2exception');

class TestClass
{
	function testMethod()
	{
		$GLOBALS['t'] = new stdClass;
	}
}

try {
	TestClass::testMethod();
} catch (Exception $e) {
	echo "Catched: ".$e->getMessage()."\n";
}
?>
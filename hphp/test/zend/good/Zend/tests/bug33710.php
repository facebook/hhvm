<?php

class Foo implements ArrayAccess
{
	function offsetExists($offset) {/*...*/}
	function offsetGet($offset) {/*...*/}
	function offsetSet($offset, $value) {/*...*/}
	function offsetUnset($offset) {/*...*/}
	
	function fail()
	{
		$this['blah'];
	}
	
	function succeed()
	{
		$this;
		$this['blah'];
	}
}

$bar = new Foo();
$bar->succeed();
$bar->fail();

?>
===DONE===

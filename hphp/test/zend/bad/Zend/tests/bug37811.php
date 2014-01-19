<?php

class TestClass
{
	function __toString()
	{
		return "Foo";
	}
}

define("Bar",new TestClass);
var_dump(Bar);
define("Baz",new stdClass);
var_dump(Baz);

?>
===DONE===
<?php

class Foo  {
	public $bar = 1;
}
$f = new foo;

echo new ReflectionObject($f);

?>

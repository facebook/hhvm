<?php

class Foo  {
	public $bar = 1;
}
$f = new foo;

ReflectionObject::export($f);

?>

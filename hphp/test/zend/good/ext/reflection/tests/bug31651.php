<?php

class Test
{
	public $a = array('a' => 1);
}

$ref = new ReflectionClass('Test');

print_r($ref->getDefaultProperties());

?>

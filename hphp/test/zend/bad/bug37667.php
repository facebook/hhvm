<?php

class Test
{
	protected $property = array('foo' => 'bar');

	function __get($name)
	{
		return $this->property;
	}
}

$obj = new Test;

var_dump($obj->property['foo']);
var_dump($obj->property[2]);

var_dump($obj);

$obj->property[] = 1;
$obj->property[] = 2;

var_dump($obj);

?>
===DONE===
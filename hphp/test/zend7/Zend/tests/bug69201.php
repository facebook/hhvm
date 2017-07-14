<?php
class Entity
{

	protected $_properties = [];

	public function &__get($property)
	{
		$value = null;
		return $value;
	}

	public function __set($property, $value)
	{
	}
}

$e = new Entity;

$e->a += 1;
echo "okey";
?>

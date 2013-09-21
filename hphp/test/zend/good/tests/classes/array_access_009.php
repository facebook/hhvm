<?php 

// NOTE: This will become part of SPL

interface ArrayProxyAccess extends ArrayAccess
{
	function proxyGet($element);
	function proxySet($element, $index, $value);
	function proxyUnset($element, $index);
}

class ArrayProxy implements ArrayAccess
{
	private $object;
	private $element;
	
	function __construct(ArrayProxyAccess $object, $element)
	{
		echo __METHOD__ . "($element)\n";
		if (!$object->offsetExists($element))
		{
			$object[$element] = array();
		}
		$this->object = $object;
		$this->element = $element;
	}

	function offsetExists($index) {
		echo __METHOD__ . "($this->element, $index)\n";
		return array_key_exists($index, $this->object->proxyGet($this->element));
	}

	function offsetGet($index) {
		echo __METHOD__ . "($this->element, $index)\n";
		$tmp = $this->object->proxyGet($this->element);
		return isset($tmp[$index]) ? $tmp[$index] : NULL;
	}

	function offsetSet($index, $value) {
		echo __METHOD__ . "($this->element, $index, $value)\n";
		$this->object->proxySet($this->element, $index, $value);
	}

	function offsetUnset($index) {
		echo __METHOD__ . "($this->element, $index)\n";
		$this->object->proxyUnset($this->element, $index);
	}
}

class Peoples implements ArrayProxyAccess
{
	public $person;
	
	function __construct()
	{
		$this->person = array(array('name'=>'Foo'));
	}

	function offsetExists($index)
	{
		return array_key_exists($index, $this->person);
	}

	function offsetGet($index)
	{
		return new ArrayProxy($this, $index);
	}

	function offsetSet($index, $value)
	{
		$this->person[$index] = $value;
	}

	function offsetUnset($index)
	{
		unset($this->person[$index]);
	}

	function proxyGet($element)
	{
		return $this->person[$element];
	}

	function proxySet($element, $index, $value)
	{
		$this->person[$element][$index] = $value;
	}
	
	function proxyUnset($element, $index)
	{
		unset($this->person[$element][$index]);
	}
}

$people = new Peoples;

var_dump($people->person[0]['name']);
$people->person[0]['name'] = $people->person[0]['name'] . 'Bar';
var_dump($people->person[0]['name']);
$people->person[0]['name'] .= 'Baz';
var_dump($people->person[0]['name']);

echo "===ArrayOverloading===\n";

$people = new Peoples;

var_dump($people[0]);
var_dump($people[0]['name']);
$people[0]['name'] = 'FooBar';
var_dump($people[0]['name']);
$people[0]['name'] = $people->person[0]['name'] . 'Bar';
var_dump($people[0]['name']);
$people[0]['name'] .= 'Baz';
var_dump($people[0]['name']);
unset($people[0]['name']);
var_dump($people[0]);
var_dump($people[0]['name']);
$people[0]['name'] = 'BlaBla';
var_dump($people[0]['name']);

?>
===DONE===
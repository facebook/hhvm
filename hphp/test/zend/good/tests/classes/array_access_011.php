<?php 

// NOTE: This will become part of SPL

class ArrayAccessReferenceProxy implements ArrayAccess
{
	private $object;
	private $oarray;
	private $element;
	
	function __construct(ArrayAccess $object, array &$array, $element)
	{
		echo __METHOD__ . "($element)\n";
		$this->object = $object;
		$this->oarray = &$array;
		$this->element = $element;
	}

	function offsetExists($index) {
		echo __METHOD__ . "($this->element, $index)\n";
		return array_key_exists($index, $this->oarray[$this->element]);
	}

	function offsetGet($index) {
		echo __METHOD__ . "($this->element, $index)\n";
		return isset($this->oarray[$this->element][$index]) ? $this->oarray[$this->element][$index] : NULL;
	}

	function offsetSet($index, $value) {
		echo __METHOD__ . "($this->element, $index, $value)\n";
		$this->oarray[$this->element][$index] = $value;
	}

	function offsetUnset($index) {
		echo __METHOD__ . "($this->element, $index)\n";
		unset($this->oarray[$this->element][$index]);
	}
}

class Peoples implements ArrayAccess
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
		if (is_array($this->person[$index]))
		{
			return new ArrayAccessReferenceProxy($this, $this->person, $index);
		}
		else
		{
			return $this->person[$index];
		}
	}

	function offsetSet($index, $value)
	{
		$this->person[$index] = $value;
	}

	function offsetUnset($index)
	{
		unset($this->person[$index]);
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
<?php exit(0); ?>
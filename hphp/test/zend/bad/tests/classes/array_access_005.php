<?php 

class Peoples implements ArrayAccess {
	public $person;
	
	function __construct() {
		$this->person = array(array('name'=>'Joe'));
	}

	function offsetExists($index) {
		return array_key_exists($this->person, $index);
	}

	function offsetGet($index) {
		return $this->person[$index];
	}

	function offsetSet($index, $value) {
		$this->person[$index] = $value;
	}

	function offsetUnset($index) {
		unset($this->person[$index]);
	}
}

$people = new Peoples;

var_dump($people->person[0]['name']);
$people->person[0]['name'] = $people->person[0]['name'] . 'Foo';
var_dump($people->person[0]['name']);
$people->person[0]['name'] .= 'Bar';
var_dump($people->person[0]['name']);

echo "---ArrayOverloading---\n";

$people = new Peoples;

var_dump($people[0]);
var_dump($people[0]['name']);
var_dump($people->person[0]['name'] . 'Foo'); // impossible to assign this since we don't return references here
$x = $people[0]; // creates a copy
$x['name'] .= 'Foo';
$people[0] = $x;
var_dump($people[0]);
$people[0]['name'] = 'JoeFoo';
var_dump($people[0]['name']);
$people[0]['name'] = 'JoeFooBar';
var_dump($people[0]['name']);

?>
===DONE===
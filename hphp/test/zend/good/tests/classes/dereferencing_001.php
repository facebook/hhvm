<?php

class Name {
	function Name($_name) {
		$this->name = $_name;
	}

	function display() {
		echo $this->name . "\n";
	}
}

class Person {
	private $name;

	function person($_name, $_address) {
		$this->name = new Name($_name);
	}

	function getName() {
		return $this->name;
	}
}

$person = new Person("John", "New York");
$person->getName()->display();

?>
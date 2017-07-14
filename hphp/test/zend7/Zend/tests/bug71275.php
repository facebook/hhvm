<?php

trait MyTrait {
	public function _() {
		throw new RuntimeException('Should not be called');
	}
}


class MyClass {
	use MyTrait;

	public function __clone() {
		echo "I'm working hard to clone";
	}
}


$instance = new MyClass();
clone $instance;

?>

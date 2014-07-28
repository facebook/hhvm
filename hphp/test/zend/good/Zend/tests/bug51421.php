<?php

class ExampleClass {}

abstract class TestInterface {
	abstract public function __construct(ExampleClass $var);
}

class Test extends TestInterface {
	public function __construct() {}
}

?>
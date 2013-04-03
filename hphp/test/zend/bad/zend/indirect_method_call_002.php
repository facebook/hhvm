<?php

class foo {
	public $x = 'testing';

	public function bar() {
		return "foo";
	}
	public function baz() {
		return new self;
	}
	static function xyz() {
	}
}

var_dump((new foo())->bar());               // string(3) "foo"
var_dump((new foo())->baz()->x);            // string(7) "testing"
var_dump((new foo())->baz()->baz()->bar()); // string(3) "foo"
var_dump((new foo())->xyz());               // NULL
(new foo())->www();

?>
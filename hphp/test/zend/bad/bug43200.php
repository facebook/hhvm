<?php

interface a {
	function foo();
	function bar();
}
interface b {
	function foo();
}

abstract class c {
	function bar() { }
}

class x extends c implements a, b {
	function foo() { }
}

ReflectionClass::export('x');

?>
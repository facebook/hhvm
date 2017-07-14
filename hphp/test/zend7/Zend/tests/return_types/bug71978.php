<?php
class A {
	function foo(int $a) {}
}
class B extends A {
	function foo(string $a) {}
}
class A1 {
	function foo(int $a): int {}
}
class B1 extends A1 {
	function foo(string $a): int {}
}
?>

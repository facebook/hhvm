<?php
class A {
	function foo(): int {}
}
class B extends A {
	function foo(): ?int {}
}
?>
DONE

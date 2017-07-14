<?php
function foo() {
	static $this;
	var_dump($this);
}
foo();
?>

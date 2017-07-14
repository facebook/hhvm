<?php
function foo() {
	parse_str("this=42");
	var_dump($this);
}
foo();
?>

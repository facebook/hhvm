<?php
function foo() {
	mb_parse_str("this=42");
	var_dump($this);
}
foo();
?>

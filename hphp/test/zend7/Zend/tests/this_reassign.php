<?php
function foo() {
	$a = "this";
	$$a = 0;
	var_dump($$a);
}
foo();
?>

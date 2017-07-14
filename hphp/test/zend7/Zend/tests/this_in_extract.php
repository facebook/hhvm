<?php
function foo() {
	extract(["this"=>42]);
	var_dump($this);
}
foo();
?>

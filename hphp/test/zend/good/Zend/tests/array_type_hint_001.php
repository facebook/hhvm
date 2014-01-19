<?php
function foo(array $a) {
	echo count($a)."\n";
}

foo(array(1,2,3));
foo(123);
?>
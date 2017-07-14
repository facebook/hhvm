<?php

function foo () {
	assert(yield 1);
	return null;
}

var_dump(foo() instanceof Generator);

?>

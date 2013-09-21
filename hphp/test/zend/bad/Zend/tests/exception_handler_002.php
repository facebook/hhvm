<?php

set_exception_handler("foo");

function foo($e) {
	var_dump(get_class($e)." thrown!");
	throw new Exception();
}

class test extends Exception {
}

throw new test();

echo "Done\n";
?>
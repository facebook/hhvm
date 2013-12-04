<?php
function handler($errno, $errstr) {
	echo "in handler()\n";
	assert(E_RECOVERABLE_ERROR === $errno);
	var_dump($errstr);
}

set_error_handler('handler', E_RECOVERABLE_ERROR);

assert(1);
assert('1');
assert('$a');

assert('aa=sd+as+safsafasfasafsaf');

assert('0');

assert_options(ASSERT_BAIL, 1);
assert('aa=sd+as+safsafasfasafsaf');

echo "done\n";

?>
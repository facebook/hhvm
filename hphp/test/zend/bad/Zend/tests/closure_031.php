<?php
function foo($errno, $errstr, $errfile, $errline) {
	echo "Error: $errstr\n";
}
set_error_handler('foo');
$foo = function() {
};
var_dump($foo->a);
?>
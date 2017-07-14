<?php
$a = array('a');
class b{};
$b = new b;
$test[] =& $a;
$test[] =& $b;
test($test);
function test() {
	debug_print_backtrace();
}
?>

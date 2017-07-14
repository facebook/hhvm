<?php
function dummy($a) {
}

try {
	chr(0)[0][] = 1;
} catch (Error $e) {
	var_dump($e->getMessage());
}
try {
	unset(chr(0)[0][0]);
} catch (Error $e) {
	var_dump($e->getMessage());
}
eval("function runtimetest(&\$a) {} ");
try {
	runtimetest(chr(0)[0]);
} catch (Error $e) {
	var_dump($e->getMessage());
}

try {
	++chr(0)[0];
} catch (Error $e) {
	var_dump($e->getMessage());
}
?>

<?php
foo();
function foo() {
	global $LAST;
	($LAST = $LAST + 0) * 1;
	echo "ok\n";
}
?>
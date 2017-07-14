<?php
function foo($x) : ?array {
	return $x;
}

foo([]);
echo "ok\n";
foo(null);
echo "ok\n";
foo(0);
?>

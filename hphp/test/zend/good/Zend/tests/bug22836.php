<?php
function &f()
{
	$x = "foo";
	var_dump($x);
	print "'$x'\n";
	return ($a);
}
for ($i = 0; $i < 8; $i++) {
	$h =& f();
}
?>
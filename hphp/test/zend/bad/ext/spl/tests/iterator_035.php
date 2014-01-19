<?php
	
$tmp = 1;

$a = new ArrayIterator();
$a[] = $tmp;
$a[] = &$tmp;

echo "Done\n";
?>
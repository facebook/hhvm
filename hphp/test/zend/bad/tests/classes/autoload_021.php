<?php
function __autoload($name) {
	echo "$name\n";
}
$a = "../BUG";
$x = new $a;
echo "BUG\n";
?>
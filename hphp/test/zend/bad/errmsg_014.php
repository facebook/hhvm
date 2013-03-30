<?php

class test {
	function __clone() {
	}
}

$t = new test;
$t->__clone();

echo "Done\n";
?>
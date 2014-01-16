<?php

class test {
	function foo($arg) {}
}

class test2 extends test {
	function foo($arg) {} 
}

class test3 extends test {
	function foo(&$arg) {} 
}

echo "Done\n";
?>
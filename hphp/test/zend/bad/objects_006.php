<?php

class test {
	function foo($arg, $arg2 = NULL) {}
}

class test2 extends test {
	function foo($arg, $arg2 = NULL) {} 
}

class test3 extends test {
	function foo($arg, $arg2) {} 
}

echo "Done\n";
?>
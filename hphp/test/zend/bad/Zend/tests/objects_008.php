<?php

class test {
	function foo(Test $arg) {}
}

class test2 extends test {
	function foo(Test $arg) {} 
}

class test3 extends test {
	function foo(Test3 $arg) {} 
}

echo "Done\n";
?>
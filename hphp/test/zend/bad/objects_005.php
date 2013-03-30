<?php

class test {
	function &foo() {}
}

class test2 extends test {
	function &foo() {} 
}

class test3 extends test {
	function foo() {} 
}

echo "Done\n";
?>
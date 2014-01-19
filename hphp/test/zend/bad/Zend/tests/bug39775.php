<?php
class test {
	var $array = array();
	function __get($var) {
		$v =& $this->array;
		return $this->array;
	}
}
$t = new test;
$t->anything[] = 'bar';
print_r($t->anything);
?>
<?php
class MyIterator extends ArrayIterator {

	function __construct() {
	 	$args = func_get_args();
		echo "   In " . __METHOD__ . "(" . implode($args, ',') . ")\n";
	}

	function rewind() { 
		$args = func_get_args();
		echo "   In " . __METHOD__ . "(" . implode($args, ',') . ")\n";
		return parent::rewind();
	}

	function valid() { 
		$args = func_get_args();
		echo "   In " . __METHOD__ . "(" . implode($args, ',') . ")\n";
		return parent::valid();
	}
	
	function current() { 
		$args = func_get_args();
		echo "   In " . __METHOD__ . "(" . implode($args, ',') . ")\n";
		return parent::current();
	}

	function next() { 
		$args = func_get_args();
		echo "   In " . __METHOD__ . "(" . implode($args, ',') . ")\n";
		return parent::next();
	}
	
	function key() { 
		$args = func_get_args();
		echo "   In " . __METHOD__ . "(" . implode($args, ',') . ")\n";
		return parent::key();
	}
}

$ao = new ArrayObject(array('a'=>1,'b'=>2,'c'=>3), 0, "MyIterator");

echo "--> Access using MyIterator:\n";
var_dump($ao->getIteratorClass());
var_dump($ao->getIterator());
foreach($ao as $key=>$value) {
	echo "  $key=>$value\n";
}

echo "\n\n--> Access using ArrayIterator:\n";
var_dump($ao->setIteratorClass("ArrayIterator"));
var_dump($ao->getIteratorClass());
var_dump($ao->getIterator());
foreach($ao as $key=>$value) {
	echo "$key=>$value\n";
}

?>
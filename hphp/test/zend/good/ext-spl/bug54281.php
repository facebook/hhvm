<?php

class RecursiveArrayIteratorIterator extends RecursiveIteratorIterator {
	function __construct($it, $max_depth) { }
}
$it = new RecursiveArrayIteratorIterator(new RecursiveArrayIterator(array()), 2);

foreach($it as $k=>$v) { }

?>
<?php
class Foo {
    function __construct() {
    	$this->error = array($this,$this);
    }
}
$a =& new Foo();

class Bar {
	function __construct() {
		$this->_rme2 = $this;
	}
}

$b =& new Bar();
$b->_rme2 = 0;
var_dump($b);
?>
<?php

class foo {
	public function __construct() {
		$a =& $this;
		
		$a->a[] = function() {
			return 1;	
		};
		
		var_dump($this);
		
		var_dump($this->a[0]());
	}
}

$x = new foo;

print "--------------\n";

foreach ($x as $b => $c) {
	var_dump($b, $c);
	var_dump($c[0]());
}

?>
<?php

class foo {
	private $test = 3;
	
	public function x() {
		$a = &$this;
		$this->a = function() use (&$a) { return $a; };
		var_dump($this->a->__invoke());
		var_dump(is_a($this->a, 'closure'));
		var_dump(is_callable($this->a));
		
		return $this->a;
	}
}

$foo = new foo;
$y = $foo->x();
var_dump($y()->test);

?>
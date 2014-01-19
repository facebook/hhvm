<?php
abstract class space{
	function __construct(){}
	abstract protected function unfold();
}

abstract class shape extends space{
	private function x1() {}
	protected final function unfold(){}
}

abstract class quad extends shape{
	private function x2() {}
	function buggy(){
		$c = get_class($this);
		$a = get_class_methods(get_class($this));
		$b = get_class_methods($this);
		print($c."\n".'a:');
		print_r($a);
		print('b:');
		print_r($b);
	}
}

class square extends quad{}

$a = new square();
$a->buggy();
print_r(get_class_methods("square"));
print_r(get_class_methods($a));
?>
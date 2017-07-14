<?php
class A {
	public $b;
}

$a = new A;
var_dump($a); // force properties HT
unset($a->b);
var_dump(serialize($a));
?>

<?php
class A {
	public $a;
	public $x;
	function __destruct() {
		unset($this->x);
	}
}
$a = new A;
$a->a = $a;
$a->x = [];
$a->x[] =& $a->x;
$a->x[] = $a;
var_dump(gc_collect_cycles());
unset($a);
var_dump(gc_collect_cycles());
var_dump(gc_collect_cycles());
